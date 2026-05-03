#include <Helios/Director.hpp>
#include <Helios/Backends/GLBackend.hpp>
#include <Helios/Debug/DevTools.hpp>
#include <Helios/Debug/Instrumentation.hpp>

#include <GLFW/glfw3.h>

#include <utility>

namespace Helios {
    Director& Director::get() noexcept {
        static Director instance;
        return instance;
    }

    bool Director::init(float screenW, float screenH, BackendType backend) {
        std::unique_ptr<RenderBackend> renderBackend;
        if (backend == BackendType::Auto) {
            renderBackend = std::make_unique<GLBackend>();
        } else {
            switch (backend) {
                default:
                case BackendType::OpenGL: renderBackend = std::make_unique<GLBackend>();
                    break;
            }
        }

        if (!renderBackend->init()) {
            return false;
        }

        m_renderer.init(std::move(renderBackend));
        m_viewport.update(screenW, screenH);
        m_inputDispatcher.setWidgetSource(&m_widgets);
        return true;
    }

    void Director::shutdown() {
        m_fontManager.shutdown();
        m_renderer.shutdown();
        m_widgets.clear();
    }

    void Director::updateViewport(float screenW, float screenH) noexcept {
        m_viewport.update(screenW, screenH);
        this->requestRender();
    }

    void Director::requestRender() noexcept {
        m_renderRequested.store(true, std::memory_order_release);
        glfwPostEmptyEvent();
    }

    bool Director::consumeRenderRequest() noexcept {
        return m_renderRequested.exchange(false, std::memory_order_acq_rel);
    }

    void Director::update(float dt) {
        m_scheduler.update(dt);
    }

    void Director::render() {
        DrawStats::get().reset();
        m_drawList.clear();

        for (auto& widget : m_widgets) {
            widget->submit(m_drawList);
        }

        #ifdef HELIOS_ENABLE_DEVTOOLS
        if (auto bounds = DevTools::getSelectedBounds()) {
            m_drawList.setLayer(Layer::Debug);
            m_drawList.strokeRect(*bounds, Color::green().withAlpha(100), 2.f);
        }
        #endif

        m_renderer.render(
            m_drawList, m_viewport,
            static_cast<uint32_t>(m_viewport.screenSize().x),
            static_cast<uint32_t>(m_viewport.screenSize().y)
        );

        DrawStats::snapshot() = DrawStats::get();
    }

    void Director::render(float dt) {
        this->update(dt);
        this->render();
    }

    Widget* Director::addWidget(WidgetPtr widget) {
        m_widgets.push_back(std::move(widget));
        this->requestRender();
        return m_widgets.back().get();
    }

    WidgetPtr Director::removeWidget(Widget* widget) {
        for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
            if (it->get() == widget) {
                WidgetPtr removed = std::move(*it);
                m_widgets.erase(it);
                this->requestRender();
                return removed;
            }
        }
        return nullptr;
    }

    void Director::markAllWidgetsDirty(DirtyFlags flags) const noexcept {
        for (auto& w : m_widgets) {
            w->markDirty(flags);
        }
    }

    void Director::layoutWidgets() const {
        for (auto& w : m_widgets) {
            w->layout();
        }
    }

    bool Director::handleTouchBegin(int touchId, Vec2 pos) {
        return m_inputDispatcher.handleTouchBegin(touchId, m_viewport.screenToVirtual(pos));
    }

    void Director::handleTouchMoved(int touchId, Vec2 pos) {
        m_inputDispatcher.handleTouchMoved(touchId, m_viewport.screenToVirtual(pos));
    }

    void Director::handleTouchEnded(int touchId, Vec2 pos) {
        m_inputDispatcher.handleTouchEnded(touchId, m_viewport.screenToVirtual(pos));
    }

    void Director::handleTouchCancelled(int touchId) {
        m_inputDispatcher.handleTouchCancelled(touchId);
    }

    bool Director::handleMouseButton(MouseButton button, bool pressed, Vec2 pos) {
        return m_inputDispatcher.handleMouseButton(button, pressed, m_viewport.screenToVirtual(pos));
    }

    void Director::handleMouseMoved(Vec2 pos) {
        m_inputDispatcher.handleMouseMoved(m_viewport.screenToVirtual(pos));
    }

    void Director::handleMouseWheel(Vec2 delta) const {
        m_inputDispatcher.handleMouseWheel(delta);
    }

    bool Director::handleKeyDown(KeyCode key, Modifiers mods) const {
        return m_inputDispatcher.handleKeyDown(key, mods);
    }

    bool Director::handleKeyUp(KeyCode key, Modifiers mods) const {
        return m_inputDispatcher.handleKeyUp(key, mods);
    }

    bool Director::handleTextInput(std::string_view text) const {
        return m_inputDispatcher.handleTextInput(text);
    }

    void Director::handleImeComposition(std::string_view composition) const {
        m_inputDispatcher.handleImeComposition(composition);
    }

    void Director::setCursorCallback(std::function<void(MouseCursor)> callback) {
        m_inputDispatcher.setCursorCallback(std::move(callback));
    }

    Director::Director() {
        m_drawList.reserveCapacity(8192, 16384, 256);
    }

    Director::~Director() {
        this->shutdown();
    }
}
