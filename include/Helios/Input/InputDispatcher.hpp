#pragma once
#ifndef HELIOS_INPUT_DISPATCHER_HPP
#define HELIOS_INPUT_DISPATCHER_HPP

#include "InputTypes.hpp"
#include "../Math.hpp"
#include "../Widgets/Widget.hpp"

#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Helios {
    class InputDispatcher {
    public:
        using CursorCallback = std::function<void(MouseCursor)>;

        static InputDispatcher& get() noexcept;

        InputDispatcher() = default;

        InputDispatcher(InputDispatcher const&) = delete;
        InputDispatcher& operator=(InputDispatcher const&) = delete;
        InputDispatcher(InputDispatcher&&) = delete;
        InputDispatcher& operator=(InputDispatcher&&) = delete;

        bool handleTouchBegin(int touchId, Vec2 pos);
        void handleTouchMoved(int touchId, Vec2 pos);
        void handleTouchEnded(int touchId, Vec2 pos);
        void handleTouchCancelled(int touchId);

        bool handleMouseButton(MouseButton button, bool pressed, Vec2 pos);
        void handleMouseMoved(Vec2 pos);
        void handleMouseWheel(Vec2 delta) const;

        bool handleKeyDown(KeyCode key, Modifiers mods) const;
        bool handleKeyUp(KeyCode key, Modifiers mods) const;

        bool handleTextInput(std::string_view text) const;
        void handleImeComposition(std::string_view composition) const;

        void setFocus(Widget* widget);
        Widget* focusedWidget() const noexcept { return m_focusedWidget; }

        void setWidgetSource(std::vector<WidgetPtr>* widgets) noexcept { m_widgets = widgets; }
        void setCursorCallback(CursorCallback callback);

        void widgetDeleted(Widget const* widget);

    private:
        struct CaptureInfo {
            Widget* target = nullptr;
            Mat3 worldToLocal = Mat3::identity();
        };

        CaptureInfo hitTest(Vec2 worldPos) const;
        CaptureInfo hitTestWidget(Widget* w, Vec2 worldPos, Rect clipRect) const;

        static Widget* dispatchTouchBegin(Widget* leaf, int touchId, Vec2 worldPos);
        static MouseCursor cursorForHovered(Widget* hovered) noexcept;

        void updateHoveredCursor();
        void setCurrentCursor(MouseCursor cursor);

        static constexpr int mouseButtonTouchId(MouseButton btn) noexcept {
            return -1 - static_cast<int>(btn);
        }

        std::vector<WidgetPtr>* m_widgets = nullptr;

        Widget* m_focusedWidget = nullptr;
        Widget* m_hoveredWidget = nullptr;

        std::unordered_map<int, CaptureInfo> m_activeTouches;
        Vec2 m_lastMousePos;

        CursorCallback m_cursorCallback;
        MouseCursor m_currentCursor = MouseCursor::Default;
    };
}

#endif // HELIOS_INPUT_DISPATCHER_HPP