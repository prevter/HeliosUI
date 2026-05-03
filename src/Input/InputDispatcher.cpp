#include <Helios/Input/InputDispatcher.hpp>

#include <Helios/Director.hpp>

#include <utility>

namespace Helios {
    InputDispatcher& InputDispatcher::get() noexcept {
        return Director::get().input();
    }

    static bool worldToLocalFor(Widget const* w, Mat3& worldToLocal) {
        return w->worldTransform().inverseAffine(worldToLocal);
    }

    static bool isInParentChain(Widget const* leaf, Widget const* maybeParent) {
        Widget const* w = leaf;
        while (w) {
            if (w == maybeParent) {
                return true;
            }
            w = w->parent();
        }
        return false;
    }

    bool InputDispatcher::handleTouchBegin(int touchId, Vec2 pos) {
        auto hit = this->hitTest(pos);
        if (!hit.target) return false;

        Widget* leaf = hit.target;

        Widget* captured = this->dispatchTouchBegin(leaf, touchId, pos);
        if (captured) {
            Mat3 capturedWorldToLocal;
            if (worldToLocalFor(captured, capturedWorldToLocal)) {
                m_activeTouches[touchId] = {captured, capturedWorldToLocal};
            }

            if (captured->wantsFocus()) {
                this->setFocus(captured);
            }
        }
        return captured != nullptr;
    }

    void InputDispatcher::handleTouchMoved(int touchId, Vec2 pos) {
        auto it = m_activeTouches.find(touchId);
        if (it == m_activeTouches.end()) return;
        it->second.target->onTouchMoved(touchId, it->second.worldToLocal.transformPoint(pos));
    }

    void InputDispatcher::handleTouchEnded(int touchId, Vec2 pos) {
        auto it = m_activeTouches.find(touchId);
        if (it == m_activeTouches.end()) return;
        it->second.target->onTouchEnded(touchId, it->second.worldToLocal.transformPoint(pos));
        m_activeTouches.erase(it);
    }

    void InputDispatcher::handleTouchCancelled(int touchId) {
        auto it = m_activeTouches.find(touchId);
        if (it == m_activeTouches.end()) return;
        it->second.target->onTouchCancelled(touchId);
        m_activeTouches.erase(it);
    }

    bool InputDispatcher::handleMouseButton(MouseButton button, bool pressed, Vec2 pos) {
        int touchId = this->mouseButtonTouchId(button);

        if (pressed) {
            auto [leaf, leafWorldToLocal] = this->hitTest(pos);
            if (!leaf) return false;

            Vec2 localPos = leafWorldToLocal.transformPoint(pos);
            leaf->onMouseButton(button, pressed, localPos);

            Widget* captured = this->dispatchTouchBegin(leaf, touchId, pos);
            if (captured) {
                Mat3 capturedWorldToLocal;
                if (worldToLocalFor(captured, capturedWorldToLocal)) {
                    m_activeTouches[touchId] = {captured, capturedWorldToLocal};
                }
                if (captured->wantsFocus()) {
                    this->setFocus(captured);
                }
            }
            return captured != nullptr;
        }

        // handle release
        auto it = m_activeTouches.find(touchId);
        if (it != m_activeTouches.end()) {
            Vec2 localPos = it->second.worldToLocal.transformPoint(pos);
            it->second.target->onMouseButton(button, pressed, localPos);
            it->second.target->onTouchEnded(touchId, localPos);
            m_activeTouches.erase(it);
        } else {
            auto [leaf, leafWorldToLocal] = this->hitTest(pos);
            if (leaf) leaf->onMouseButton(button, pressed, leafWorldToLocal.transformPoint(pos));
        }
        return false;
    }

    void InputDispatcher::handleMouseMoved(Vec2 pos) {
        m_lastMousePos = pos;

        auto [leaf, leafWorldToLocal] = this->hitTest(pos);
        Widget* newHovered = leaf;

        if (newHovered != m_hoveredWidget) {
            if (m_hoveredWidget) m_hoveredWidget->onMouseLeave();
            m_hoveredWidget = newHovered;
            if (m_hoveredWidget) m_hoveredWidget->onMouseEnter();
        }

        this->updateHoveredCursor();

        Widget* hoveredLeaf = m_hoveredWidget;
        Widget* w = hoveredLeaf;
        while (w) {
            Mat3 worldToLocal;
            if (worldToLocalFor(w, worldToLocal)) {
                w->onMouseMoved(worldToLocal.transformPoint(pos));
            }
            w = w->parent();
        }

        for (int touchId = -1; touchId > -5; --touchId) {
            auto it = m_activeTouches.find(touchId);
            if (it != m_activeTouches.end()) {
                Vec2 localPos = it->second.worldToLocal.transformPoint(pos);
                if (!isInParentChain(hoveredLeaf, it->second.target)) {
                    it->second.target->onMouseMoved(localPos);
                }
                it->second.target->onTouchMoved(touchId, localPos);
            }
        }
    }

    void InputDispatcher::setCursorCallback(CursorCallback callback) {
        m_cursorCallback = std::move(callback);
        if (m_cursorCallback) {
            m_cursorCallback(m_currentCursor);
        }
    }

    void InputDispatcher::widgetDeleted(Widget const* widget) {
        if (m_focusedWidget == widget) {
            this->setFocus(nullptr);
        }
        if (m_hoveredWidget == widget) {
            m_hoveredWidget = nullptr;
            this->updateHoveredCursor();
        }
    }

    void InputDispatcher::handleMouseWheel(Vec2 delta) const {
        Widget* w = m_hoveredWidget;
        while (w) {
            if (w->onMouseWheel(delta)) break;
            w = w->parent();
        }
    }

    bool InputDispatcher::handleKeyDown(KeyCode key, Modifiers mods) const {
        if (!m_focusedWidget) return false;
        Widget* w = m_focusedWidget;
        while (w) {
            if (w->onKeyDown(key, mods)) return true;
            w = w->parent();
        }
        return false;
    }

    bool InputDispatcher::handleKeyUp(KeyCode key, Modifiers mods) const {
        if (!m_focusedWidget) return false;
        Widget* w = m_focusedWidget;
        while (w) {
            if (w->onKeyUp(key, mods)) return true;
            w = w->parent();
        }
        return false;
    }

    bool InputDispatcher::handleTextInput(std::string_view text) const {
        if (!m_focusedWidget) return false;
        return m_focusedWidget->onTextInput(text);
    }

    void InputDispatcher::handleImeComposition(std::string_view composition) const {
        if (m_focusedWidget) {
            m_focusedWidget->onImeCompositionUpdate(composition);
        }
    }

    void InputDispatcher::setFocus(Widget* widget) {
        if (m_focusedWidget == widget) return;

        if (m_focusedWidget) {
            m_focusedWidget->onFocusLost();
            m_focusedWidget->onImeCompositionUpdate({});
        }

        m_focusedWidget = widget;
        if (m_focusedWidget) {
            m_focusedWidget->onFocusGained();
        }
    }

    InputDispatcher::CaptureInfo InputDispatcher::hitTest(Vec2 worldPos) const {
        if (!m_widgets) return {};

        for (auto it = m_widgets->rbegin(); it != m_widgets->rend(); ++it) {
            auto r = this->hitTestWidget(it->get(), worldPos, Rect::maxRect());
            if (r.target) return r;
        }

        return {};
    }

    InputDispatcher::CaptureInfo InputDispatcher::hitTestWidget(Widget* w, Vec2 worldPos, Rect clipRect) const {
        if (!w->isVisible() || !w->isHitTestable()) return {};

        Mat3 worldTransform = w->worldTransform();
        Mat3 worldToLocal;
        if (!worldTransform.inverseAffine(worldToLocal)) {
            return {};
        }

        Vec2 size = w->size();
        Rect bounds = worldTransform.transformRect(Rect{0.f, 0.f, size.x, size.y});
        Vec2 localPos = worldToLocal.transformPoint(worldPos);
        Rect localBounds{0.f, 0.f, size.x, size.y};

        if (!bounds.contains(worldPos)) return {};
        if (!clipRect.contains(worldPos)) return {};
        if (!localBounds.contains(localPos)) return {};

        Rect childClip = w->hasClipping()
            ? clipRect.intersection(bounds)
            : clipRect;

        for (auto it = w->children().rbegin(); it != w->children().rend(); ++it) {
            auto r = this->hitTestWidget(it->get(), worldPos, childClip);
            if (r.target) return r;
        }

        return {w, worldToLocal};
    }

    Widget* InputDispatcher::dispatchTouchBegin(Widget* leaf, int touchId, Vec2 worldPos) {
        Widget* w = leaf;

        while (w) {
            Mat3 worldToLocal;
            if (!worldToLocalFor(w, worldToLocal)) {
                w = w->parent();
                continue;
            }

            Vec2 localPos = worldToLocal.transformPoint(worldPos);
            if (w->onTouchBegin(touchId, localPos)) {
                return w;
            }

            w = w->parent();
        }

        return nullptr;
    }

    MouseCursor InputDispatcher::cursorForHovered(Widget* hovered) noexcept {
        Widget* w = hovered;
        while (w) {
            MouseCursor cursor = w->cursorOnHover();
            if (cursor != MouseCursor::Default) {
                return cursor;
            }
            w = w->parent();
        }
        return MouseCursor::Default;
    }

    void InputDispatcher::updateHoveredCursor() {
        this->setCurrentCursor(this->cursorForHovered(m_hoveredWidget));
    }

    void InputDispatcher::setCurrentCursor(MouseCursor cursor) {
        if (m_currentCursor == cursor) return;
        m_currentCursor = cursor;
        if (m_cursorCallback) {
            m_cursorCallback(m_currentCursor);
        }
    }
}
