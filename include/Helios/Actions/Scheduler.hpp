#pragma once
#ifndef HELIOS_SCHEDULER_HPP
#define HELIOS_SCHEDULER_HPP

#include "Action.hpp"
#include "../Handles.hpp"

namespace Helios {
    class Widget;

    class Scheduler {
    public:
        static Scheduler& get() noexcept;

        Scheduler() = default;

        Scheduler(Scheduler const&) = delete;
        Scheduler& operator=(Scheduler const&) = delete;
        Scheduler(Scheduler&&) = delete;
        Scheduler& operator=(Scheduler&&) = delete;

        ActionHandle run(Widget* target, ActionPtr action);
        void cancel(ActionHandle handle);
        void cancelAll(Widget const* target);
        void cancelAll() noexcept;

        void update(float dt);

        bool isRunning(ActionHandle handle) const noexcept;
        [[nodiscard]] bool hasActiveActions() const noexcept;

        [[nodiscard]] float deltaTime() const noexcept { return m_lastDelta; }

    private:
        struct Entry {
            Widget* target = nullptr;
            ActionHandle handle;
            ActionPtr action;
            bool cancelled = false;
        };

        ActionHandle nextHandle() noexcept { return ActionHandle(++m_nextId); }

        std::vector<Entry> m_entries;
        std::vector<Entry> m_pending;

        float m_lastDelta = 0.f;
        uint32_t m_nextId = 0;
        bool m_updating = false;
        bool m_alive = true;
    };
}

#endif // HELIOS_SCHEDULER_HPP
