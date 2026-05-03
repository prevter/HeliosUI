#pragma once
#ifndef HELIOS_ACTION_HPP
#define HELIOS_ACTION_HPP

#include "Easings.hpp"
#include "../Math.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace Helios {
    class Action;
    using ActionPtr = std::unique_ptr<Action>;

    template <typename F>
    using ActionFunc = std::function<F>;

    class Action {
    public:
        virtual ~Action() = default;

        Action(Action const&) = delete;
        Action& operator=(Action const&) = delete;

        virtual bool update(float dt) = 0;
        virtual void reset() = 0;

        static ActionPtr delay(float duration, ActionFunc<void()> onComplete = nullptr);
        static ActionPtr call(ActionFunc<void()> func);

        static ActionPtr tweenFloat(float from, float to, float duration, Easing easing, ActionFunc<void(float)> setter);
        static ActionPtr tweenVec2(Vec2 from, Vec2 to, float duration, Easing easing, ActionFunc<void(Vec2)> setter);
        static ActionPtr tweenColor(Color from, Color to, float duration, Easing easing, ActionFunc<void(Color)> setter);

        static ActionPtr sequence(std::vector<ActionPtr> actions);
        static ActionPtr spawn(std::vector<ActionPtr> actions);

        static ActionPtr repeat(ActionPtr action, int count);

        static constexpr int Forever = -1;

        template <typename... Args>
        static ActionPtr sequence(Args&&... actions) {
            static_assert(sizeof...(actions) > 0, "At least one action must be provided to sequence");
            std::vector<ActionPtr> actionVec;
            actionVec.reserve(sizeof...(actions));
            (actionVec.push_back(std::forward<Args>(actions)), ...);
            return sequence(std::move(actionVec));
        }

        template <typename... Args>
        static ActionPtr spawn(Args&&... actions) {
            static_assert(sizeof...(actions) > 0, "At least one action must be provided to spawn");
            std::vector<ActionPtr> actionVec;
            actionVec.reserve(sizeof...(actions));
            (actionVec.push_back(std::forward<Args>(actions)), ...);
            return spawn(std::move(actionVec));
        }

    protected:
        Action() = default;
    };

    class DelayAction : public Action {
    public:
        DelayAction(float duration, ActionFunc<void()> onComplete = nullptr)
            : m_onComplete(std::move(onComplete)), m_duration(duration) {}

        bool update(float dt) override {
            m_elapsed += dt;
            if (m_elapsed >= m_duration) {
                if (m_onComplete) m_onComplete();
                return true;
            }
            return false;
        }

        void reset() override { m_elapsed = 0.f; }

    private:
        ActionFunc<void()> m_onComplete;
        float m_duration;
        float m_elapsed = 0.f;
    };

    class CallAction : public Action {
    public:
        explicit CallAction(ActionFunc<void()> func) : m_func(std::move(func)) {}

        bool update(float) override {
            if (m_func) m_func();
            return true;
        }

        void reset() override {}

    private:
        ActionFunc<void()> m_func;
    };

    template <typename T>
    class TweenAction : public Action {
    public:
        TweenAction(T from, T to, float duration, Easing easing, ActionFunc<void(T)> setter)
            : m_setter(std::move(setter)), m_easing(getEasingFunction(easing)),
              m_duration(duration), m_from(from), m_to(to) {}

        bool update(float dt) override {
            if (m_duration <= 0.f) {
                m_setter(m_to);
                return true;
            }

            m_elapsed = std::min(m_elapsed + dt, m_duration);
            float t = m_easing(std::min(m_elapsed / m_duration, 1.f));
            m_setter(lerp(m_from, m_to, t));
            return m_elapsed >= m_duration;
        }

        void reset() override { m_elapsed = 0.f; }

    private:
        static float lerp(float a, float b, float t) noexcept { return a + (b - a) * t; }
        static Vec2 lerp(Vec2 a, Vec2 b, float t) noexcept { return Vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t)); }
        static Color lerp(Color a, Color b, float t) noexcept {
            return Color(
                static_cast<uint8_t>(lerp(a.r, b.r, t)),
                static_cast<uint8_t>(lerp(a.g, b.g, t)),
                static_cast<uint8_t>(lerp(a.b, b.b, t)),
                static_cast<uint8_t>(lerp(a.a, b.a, t))
            );
        }

        ActionFunc<void(T)> m_setter;
        EasingFn m_easing;
        float m_duration;
        float m_elapsed = 0.f;
        T m_from;
        T m_to;
    };

    class SequenceAction : public Action {
    public:
        explicit SequenceAction(std::vector<ActionPtr> actions) : m_actions(std::move(actions)) {}

        bool update(float dt) override {
            if (m_index >= m_actions.size()) return true;

            while (m_index < m_actions.size()) {
                if (!m_actions[m_index]->update(dt)) {
                    return false;
                }
                m_index++;
                dt = 0.f;
            }

            return true;
        }

        void reset() override {
            m_index = 0;
            for (auto& action : m_actions) {
                action->reset();
            }
        }

    private:
        std::vector<ActionPtr> m_actions;
        size_t m_index = 0;
    };

    class SpawnAction : public Action {
    public:
        explicit SpawnAction(std::vector<ActionPtr> actions) : m_actions(std::move(actions)) {
            m_done.resize(m_actions.size(), false);
        }

        bool update(float dt) override {
            bool allDone = true;
            for (size_t i = 0; i < m_actions.size(); ++i) {
                if (m_done[i]) continue;
                if (m_actions[i]->update(dt)) {
                    m_done[i] = true;
                } else {
                    allDone = false;
                }
            }
            return allDone;
        }

        void reset() override {
            for (auto& action : m_actions) {
                action->reset();
            }
            std::fill(m_done.begin(), m_done.end(), false);
        }

    private:
        std::vector<ActionPtr> m_actions;
        std::vector<bool> m_done;
    };

    class RepeatAction : public Action {
    public:
        RepeatAction(ActionPtr action, int count) : m_action(std::move(action)), m_total(count) {}

        bool update(float dt) override {
            if (m_total != Forever && m_done >= m_total) return true;

            if (m_action->update(dt)) {
                m_done++;
                m_action->reset();
            }

            return false;
        }

        void reset() override {
            m_done = 0;
            m_action->reset();
        }

    private:
        ActionPtr m_action;
        int m_total;
        int m_done = 0;
    };

    inline ActionPtr Action::delay(float duration, ActionFunc<void()> onComplete) {
        return std::make_unique<DelayAction>(duration, std::move(onComplete));
    }

    inline ActionPtr Action::call(ActionFunc<void()> func) {
        return std::make_unique<CallAction>(std::move(func));
    }

    inline ActionPtr Action::tweenFloat(
        float from, float to,
        float duration, Easing easing,
        ActionFunc<void(float)> setter
    ) {
        return std::make_unique<TweenAction<float>>(from, to, duration, easing, std::move(setter));
    }

    inline ActionPtr Action::tweenVec2(
        Vec2 from, Vec2 to,
        float duration, Easing easing,
        ActionFunc<void(Vec2)> setter
    ) {
        return std::make_unique<TweenAction<Vec2>>(from, to, duration, easing, std::move(setter));
    }

    inline ActionPtr Action::tweenColor(
        Color from, Color to,
        float duration, Easing easing,
        ActionFunc<void(Color)> setter
    ) {
        return std::make_unique<TweenAction<Color>>(from, to, duration, easing, std::move(setter));
    }

    inline ActionPtr Action::sequence(std::vector<ActionPtr> actions) {
        return std::make_unique<SequenceAction>(std::move(actions));
    }

    inline ActionPtr Action::spawn(std::vector<ActionPtr> actions) {
        return std::make_unique<SpawnAction>(std::move(actions));
    }

    inline ActionPtr Action::repeat(ActionPtr action, int count) {
        return std::make_unique<RepeatAction>(std::move(action), count);
    }
}

#endif // HELIOS_ACTION_HPP