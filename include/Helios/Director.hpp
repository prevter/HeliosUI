#pragma once
#ifndef HELIOS_DIRECTOR_HPP
#define HELIOS_DIRECTOR_HPP

#include "Actions/Scheduler.hpp"
#include "Input/InputDispatcher.hpp"
#include "Renderer/Renderer.hpp"
#include "Text/FontManager.hpp"
#include "Widgets/Widget.hpp"

#include <atomic>
#include <functional>

namespace Helios {
    enum class BackendType {
        Auto,

        OpenGL,
        // Vulkan,
        // DirectX11,
        // DirectX12,
        // Metal
    };

    class Director {
    public:
        static Director& get() noexcept;

        Director(Director const&) = delete;
        Director& operator=(Director const&) = delete;
        Director(Director&&) = delete;
        Director& operator=(Director&&) = delete;

        bool init(float screenW, float screenH, BackendType backend = BackendType::Auto);
        void shutdown();

        void updateViewport(float screenW, float screenH) noexcept;

        void requestRender() noexcept;
        [[nodiscard]] bool consumeRenderRequest() noexcept;
        void setWakeupCallback(void(*callback)()) noexcept;

        void update(float dt);
        void render();
        void render(float dt);

        Renderer& renderer() noexcept { return m_renderer; }
        Viewport& viewport() noexcept { return m_viewport; }
        FontManager& fontManager() noexcept { return m_fontManager; }
        DrawList& drawList() noexcept { return m_drawList; }
        InputDispatcher& input() noexcept { return m_inputDispatcher; }
        Scheduler& scheduler() noexcept { return m_scheduler; }

        std::vector<WidgetPtr>& widgets() noexcept { return m_widgets; }

        Widget* addWidget(WidgetPtr widget);
        WidgetPtr removeWidget(Widget* widget);
        void clearWidgets() { m_widgets.clear(); }

        void markAllWidgetsDirty(DirtyFlags flags = DirtyFlags::All) const noexcept;
        void layoutWidgets() const;

        template <typename T, typename... Args>
        T* addWidget(Args&&... args) {
            auto widget = std::make_unique<T>(std::forward<Args>(args)...);
            T* widgetPtr = widget.get();
            this->addWidget(std::move(widget));
            return widgetPtr;
        }

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

        void setCursorCallback(std::function<void(MouseCursor)> callback);

    private:
        Director();
        ~Director();

        Renderer m_renderer;
        Viewport m_viewport;
        FontManager m_fontManager;
        DrawList m_drawList;
        InputDispatcher m_inputDispatcher;
        Scheduler m_scheduler;

        std::vector<WidgetPtr> m_widgets;
        void(*m_wakeupCallback)() = nullptr;
        std::atomic_bool m_renderRequested{true};
    };
}

#endif // HELIOS_DIRECTOR_HPP
