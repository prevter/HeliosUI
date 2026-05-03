#include <Helios/Actions/Scheduler.hpp>
#include <Helios/Director.hpp>
#include <Helios/Widgets/Widget.hpp>

#include <Helios/Debug/Instrumentation.hpp>
#include <Helios/Debug/Log.hpp>

namespace Helios {
    Widget::~Widget() {
        Scheduler::get().cancelAll(this);
        InputDispatcher::get().widgetDeleted(this);
    }

    void Widget::resolveRootAutoSize() noexcept {
        if (m_parent) return;
        if (m_requestedSize.x > 0.f && m_requestedSize.y > 0.f) return;

        Vec2 measured = this->measure();
        Vec2 resolved = {
            m_requestedSize.x > 0.f ? m_requestedSize.x : measured.x,
            m_requestedSize.y > 0.f ? m_requestedSize.y : measured.y,
        };

        if (resolved != m_size) {
            m_size = resolved;
            m_dirtyFlags = m_dirtyFlags | DirtyFlags::Geometry | DirtyFlags::Transform;
        }
    }

    Widget* Widget::addChild(WidgetPtr child) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
        this->markDirty(DirtyFlags::Layout);
        return m_children.back().get();
    }

    WidgetPtr Widget::removeChild(Widget* child) {
        for (auto it = m_children.begin(); it != m_children.end(); ++it) {
            if (it->get() == child) {
                WidgetPtr removed = std::move(*it);
                removed->m_parent = nullptr;
                m_children.erase(it);
                this->markDirty(DirtyFlags::Layout);
                return removed;
            }
        }
        return nullptr;
    }

    void Widget::clearChildren() {
        for (auto& child : m_children) {
            child->m_parent = nullptr;
        }
        m_children.clear();
        this->markDirty(DirtyFlags::Layout);
    }

    Mat3 Widget::worldTransform() const noexcept {
        Mat3 parentTransform = m_parent ? m_parent->worldTransform() : Mat3::identity();
        Vec2 anchorOffset = this->anchor() * this->size();
        Vec2 scale = this->getScale();
        Vec2 pos = this->position();
        Mat3 localTransform =
            Mat3::translate(pos.x, pos.y) *
            Mat3::scale(scale.x, scale.y) *
            Mat3::translate(-anchorOffset.x, -anchorOffset.y);

        return parentTransform * localTransform;
    }

    ActionHandle Widget::runAction(ActionPtr action) {
        return Scheduler::get().run(this, std::move(action));
    }

    void Widget::stopAllActions() const noexcept {
        Scheduler::get().cancelAll(this);
    }

    void Widget::markDirty(DirtyFlags flags) const noexcept {
        m_dirtyFlags = m_dirtyFlags | flags;
        Director::get().requestRender();
        if (m_parent && m_parent->m_isLayoutContainer && any(flags & (DirtyFlags::Geometry | DirtyFlags::Layout))) {
            bool fitW = m_layoutParams.widthMode == LayoutParams::SizeMode::Fit;
            bool fitH = m_layoutParams.heightMode == LayoutParams::SizeMode::Fit;
            if (fitW || fitH) m_parent->markDirty(DirtyFlags::Layout);
        }
    }

    void Widget::markGeometryDirty() const noexcept {
        markDirty(DirtyFlags::Geometry);
    }

    void Widget::markTransformDirty() const noexcept {
        markDirty(DirtyFlags::Transform);
        for (auto& c : m_children) {
            c->markTransformDirty();
        }
    }

    void Widget::markDirtyRecursive() const noexcept {
        markDirty(DirtyFlags::All);
        for (auto& c : m_children) {
            c->markDirtyRecursive();
        }
    }

    void Widget::layout() {
        this->resolveRootAutoSize();
        this->onLayout();
        m_dirtyFlags = m_dirtyFlags & ~DirtyFlags::Layout;
        for (auto& c : m_children) {
            c->markTransformDirty();
            c->layout();
        }
    }

    void Widget::submit(DrawList& frameDL, Vec2 parentOrigin) const {
        this->submitInternal(frameDL, Mat3::translate(parentOrigin.x, parentOrigin.y));
    }

    void Widget::submitInternal(DrawList& frameDL, Mat3 const& parentTransform) const {
        HELIOS_INSTRUMENT_WIDGETS_SUBMITTED(1);

        if (!m_visible) return;

        if (any(m_dirtyFlags & DirtyFlags::Layout)) {
            HELIOS_INSTRUMENT_WIDGET_RELAYOUTS(1);
            const_cast<Widget*>(this)->resolveRootAutoSize();
            const_cast<Widget*>(this)->onLayout();
            m_dirtyFlags = m_dirtyFlags & ~DirtyFlags::Layout;
        }

        Vec2 anchorOffset = {m_anchor.x * m_size.x, m_anchor.y * m_size.y};
        Mat3 localTransform =
            Mat3::translate(m_pos.x, m_pos.y) *
            Mat3::scale(m_scale.x, m_scale.y) *
            Mat3::translate(-anchorOffset.x, -anchorOffset.y);
        Mat3 worldTransform = parentTransform * localTransform;
        Vec2 origin = worldTransform.transformPoint({0.f, 0.f});

        if (static_cast<bool>(m_dirtyFlags & DirtyFlags::Geometry)) {
            HELIOS_INSTRUMENT_WIDGET_REDRAWS(1);
            m_localDrawList.clear();
            m_localDrawList.setLayer(Layer::Content);
            m_localDrawList.setZ(0.f);
            const_cast<Widget*>(this)->onDraw(m_localDrawList);
            m_cachedOrigin = origin;
            m_dirtyFlags = DirtyFlags::None;
        } else if (static_cast<bool>(m_dirtyFlags & DirtyFlags::Transform)) {
            m_cachedOrigin = origin;
            m_dirtyFlags = DirtyFlags::None;
        } else {
            origin = m_cachedOrigin;
        }

        if (!m_localDrawList.getVertices().empty() && !m_noDraw) {
            frameDL.appendTransformed(m_localDrawList, worldTransform);
        }

        if (m_children.empty() || m_hideChildren) return;

        if (m_hasClipping) {
            frameDL.pushClipRect(worldTransform.transformRect( {0.f, 0.f, m_size.x, m_size.y}));
        }

        for (auto& c : m_children) {
            c->submitInternal(frameDL, worldTransform);
        }

        if (m_hasClipping) {
            frameDL.popClipRect();
        }
    }

    void Widget::setPosition(Vec2 pos) {
        m_pos = pos;
        this->markTransformDirty();
    }

    void Widget::setSize(Vec2 size) {
        m_requestedSize = size;
        m_size = size;
        this->markDirty(DirtyFlags::Geometry | DirtyFlags::Layout);
    }

    void Widget::setRect(Vec2 pos, Vec2 size) {
        bool sizeChanged = m_size != size;
        m_pos = pos;
        m_size = size;
        DirtyFlags flags = DirtyFlags::Geometry | DirtyFlags::Transform;
        if (sizeChanged && m_isLayoutContainer) {
            flags = flags | DirtyFlags::Layout;
        }
        this->markDirty(flags);
    }

    void Widget::setAnchor(Vec2 anchor) {
        m_anchor = anchor;
        this->markTransformDirty();
    }

    void Widget::setScale(Vec2 scale) noexcept {
        if (m_scale == scale) return;
        m_scale = scale;
        this->markTransformDirty();
        this->markDirty(DirtyFlags::Layout);
    }

    void Widget::setScale(float scale) noexcept {
        this->setScale({scale, scale});
    }

    void Widget::setVisible(bool visible) noexcept {
        if (m_visible == visible) return;
        m_visible = visible;
        this->markDirty(DirtyFlags::All);
    }
}
