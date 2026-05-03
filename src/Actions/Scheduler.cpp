#include <Helios/Actions/Scheduler.hpp>

#include <Helios/Director.hpp>

namespace Helios {
    Scheduler& Scheduler::get() noexcept {
        return Director::get().scheduler();
    }

    ActionHandle Scheduler::run(Widget* target, ActionPtr action) {
        ActionHandle h = this->nextHandle();
        Entry e{target, h, std::move(action), false};

        if (m_updating) {
            m_pending.push_back(std::move(e));
        } else {
            m_entries.push_back(std::move(e));
        }

        Director::get().requestRender();

        return h;
    }

    void Scheduler::cancel(ActionHandle handle) {
        for (auto& e : m_entries) {
            if (e.handle == handle) {
                e.cancelled = true;
                return;
            }
        }

        for (auto& e : m_pending) {
            if (e.handle == handle) {
                e.cancelled = true;
                return;
            }
        }
    }

    void Scheduler::cancelAll(Widget const* target) {
        if (!m_alive) return;
        for (auto& e : m_entries) {
            if (e.target == target) {
                e.cancelled = true;
            }
        }
        for (auto& e : m_pending) {
            if (e.target == target) {
                e.cancelled = true;
            }
        }
    }

    void Scheduler::cancelAll() noexcept {
        for (auto& e : m_entries) {
            e.cancelled = true;
        }
        m_pending.clear();
    }

    void Scheduler::update(float dt) {
        if (!m_alive) return;

        m_lastDelta = dt;
        m_updating = true;

        for (auto& e : m_entries) {
            if (e.cancelled) continue;
            if (e.action->update(dt))
                e.cancelled = true;
        }

        m_updating = false;

        std::erase_if(
            m_entries, [](Entry const& e) {
                return e.cancelled;
            }
        );

        for (auto& e : m_pending) {
            m_entries.push_back(std::move(e));
        }

        m_pending.clear();
    }

    bool Scheduler::isRunning(ActionHandle handle) const noexcept {
        for (auto& e : m_entries) {
            if (e.handle == handle && !e.cancelled) {
                return true;
            }
        }
        return false;
    }

    bool Scheduler::hasActiveActions() const noexcept {
        return !m_entries.empty() || !m_pending.empty();
    }
}
