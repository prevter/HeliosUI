#pragma once
#ifndef HELIOS_WIDGETS_WIDGET_HPP
#define HELIOS_WIDGETS_WIDGET_HPP

#include "Layout.hpp"
#include "../Actions/Action.hpp"
#include "../Input/InputTypes.hpp"
#include "../Renderer/DrawList.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace Helios {
    enum class DirtyFlags : uint8_t {
        None      = 0,
        Geometry  = 1 << 0,
        Transform = 1 << 1,
        Layout    = 1 << 2,
        All       = Geometry | Transform | Layout
    };

    constexpr DirtyFlags operator|(DirtyFlags a, DirtyFlags b) noexcept {
        return static_cast<DirtyFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    constexpr DirtyFlags operator&(DirtyFlags a, DirtyFlags b) noexcept {
        return static_cast<DirtyFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    constexpr DirtyFlags operator~(DirtyFlags flags) noexcept {
        return static_cast<DirtyFlags>(~static_cast<uint8_t>(flags));
    }

    constexpr bool any(DirtyFlags flags) noexcept {
        return static_cast<uint8_t>(flags) != 0;
    }

    class Widget;
    using WidgetPtr = std::unique_ptr<Widget>;

    class Widget {
    public:
        Widget() = default;
        virtual ~Widget();

        Widget(Widget const&) = delete;
        Widget& operator=(Widget const&) = delete;
        Widget(Widget&&) noexcept = default;
        Widget& operator=(Widget&&) noexcept = default;

        Widget* addChild(WidgetPtr child);
        WidgetPtr removeChild(Widget* child);

        template <typename T, typename... Args>
        T* addChild(Args&&... args) {
            auto child = std::make_unique<T>(std::forward<Args>(args)...);
            T* childPtr = child.get();
            this->addChild(std::move(child));
            return childPtr;
        }

        ActionHandle runAction(ActionPtr action);
        void stopAllActions() const noexcept;

        void markDirty(DirtyFlags flags = DirtyFlags::All) const noexcept;
        void markGeometryDirty() const noexcept;
        void markTransformDirty() const noexcept;
        void markDirtyRecursive() const noexcept;

        void layout();
        void submit(DrawList& frameDL, Vec2 parentOrigin = {}) const;

        void setPosition(Vec2 pos);
        void setSize(Vec2 size);
        void setRect(Vec2 pos, Vec2 size);
        void setAnchor(Vec2 anchor);
        void setScale(Vec2 scale) noexcept;
        void setScale(float scale) noexcept;

        LayoutParams& layoutParams() noexcept { return m_layoutParams; }
        LayoutParams const& layoutParams() const noexcept { return m_layoutParams; }

        bool isVisible() const noexcept { return m_visible; }
        void setVisible(bool visible) noexcept;

        bool hidesChildren() const noexcept { return m_hideChildren; }
        void setHideChildren(bool hideChildren) { m_hideChildren = hideChildren; }

        bool noDraw() const noexcept { return m_noDraw; }
        void setNoDraw(bool noDraw) { m_noDraw = noDraw; }

        bool hasClipping() const noexcept { return m_hasClipping; }
        void setClipping(bool enabled) { m_hasClipping = enabled; }

        bool isHitTestable() const noexcept { return m_hitTestable; }
        void setHitTestable(bool hitTestable) { m_hitTestable = hitTestable; }

        Widget* parent() const noexcept { return m_parent; }
        std::vector<WidgetPtr> const& children() const noexcept { return m_children; }
        void clearChildren();

        Vec2 position() const noexcept { return m_pos; }
        Vec2 size() const noexcept { return m_size; }
        Vec2 scaledSize() const noexcept {
            float sx = m_scale.x >= 0.f ? m_scale.x : -m_scale.x;
            float sy = m_scale.y >= 0.f ? m_scale.y : -m_scale.y;
            return {m_size.x * sx, m_size.y * sy};
        }
        Vec2 anchor() const noexcept { return m_anchor; }
        Vec2 getScale() const noexcept { return m_scale; }
        Vec2 requestedSize() const noexcept { return m_requestedSize; }

        Vec2 parentOrigin() const noexcept { return m_cachedOrigin; }
        Mat3 worldTransform() const noexcept;

        virtual void onLayout() {}
        virtual void onDraw(DrawList& dl) { (void) dl; }
        virtual Vec2 measure() { return m_requestedSize; }

        virtual bool onTouchBegin(int touchId, Vec2 pos) { (void) touchId; (void) pos; return false; }
        virtual void onTouchMoved(int touchId, Vec2 pos) { (void) touchId; (void) pos; }
        virtual void onTouchEnded(int touchId, Vec2 pos) { (void) touchId; (void) pos; }
        virtual void onTouchCancelled(int touchId) { (void) touchId; }

        virtual bool onMouseButton(MouseButton btn, bool pressed, Vec2 pos) { (void) btn; (void) pressed; (void) pos; return false; }
        virtual void onMouseMoved(Vec2 pos) { (void) pos; }
        virtual bool onMouseWheel(Vec2 delta) { (void) delta; return false; }
        virtual void onMouseEnter() {}
        virtual void onMouseLeave() {}
        virtual MouseCursor cursorOnHover() const { return MouseCursor::Default; }

        virtual bool onKeyDown(KeyCode key, Modifiers mods) { (void) key; (void) mods; return false; }
        virtual bool onKeyUp(KeyCode key, Modifiers mods) { (void) key; (void) mods; return false; }

        virtual bool onTextInput(std::string_view text) { (void) text; return false; }
        virtual void onImeCompositionUpdate(std::string_view composition) { (void) composition; }

        virtual bool wantsFocus() const { return false; }
        virtual void onFocusGained() {}
        virtual void onFocusLost() {}

    protected:
        void submitInternal(DrawList& frameDL, Mat3 const& parentTransform) const;

        void resolveRootAutoSize() noexcept;

        Vec2 m_pos = {};
        Vec2 m_size = {};
        Vec2 m_scale = {1.f, 1.f};
        Vec2 m_requestedSize = {};
        Vec2 m_anchor = {};
        LayoutParams m_layoutParams;

        Widget* m_parent = nullptr;
        std::vector<WidgetPtr> m_children;

        mutable DrawList m_localDrawList;
        mutable Vec2 m_cachedOrigin = {};
        mutable DirtyFlags m_dirtyFlags = DirtyFlags::None;

        bool m_visible = true;
        bool m_hideChildren = false;
        bool m_noDraw = false; // disables drawing, but still participates in layout and hit testing
        bool m_hasClipping = false;
        bool m_isLayoutContainer = false;
        bool m_hitTestable = true;
    };
}

#endif // HELIOS_WIDGETS_WIDGET_HPP
