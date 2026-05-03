#include <Helios/Actions/Easings.hpp>

#include <cmath>
#include <numbers>

namespace Helios {
    EasingFn getEasingFunction(Easing type) noexcept {
        switch (type) {
            case Easing::Linear: return Easings::linear;
            case Easing::SineIn: return Easings::sineIn;
            case Easing::SineOut: return Easings::sineOut;
            case Easing::SineInOut: return Easings::sineInOut;
            case Easing::QuadIn: return Easings::quadIn;
            case Easing::QuadOut: return Easings::quadOut;
            case Easing::QuadInOut: return Easings::quadInOut;
            case Easing::CubicIn: return Easings::cubicIn;
            case Easing::CubicOut: return Easings::cubicOut;
            case Easing::CubicInOut: return Easings::cubicInOut;
            case Easing::QuartIn: return Easings::quartIn;
            case Easing::QuartOut: return Easings::quartOut;
            case Easing::QuartInOut: return Easings::quartInOut;
            case Easing::QuintIn: return Easings::quintIn;
            case Easing::QuintOut: return Easings::quintOut;
            case Easing::QuintInOut: return Easings::quintInOut;
            case Easing::ExpoIn: return Easings::expoIn;
            case Easing::ExpoOut: return Easings::expoOut;
            case Easing::ExpoInOut: return Easings::expoInOut;
            case Easing::CircIn: return Easings::circIn;
            case Easing::CircOut: return Easings::circOut;
            case Easing::CircInOut: return Easings::circInOut;
            case Easing::BackIn: return Easings::backIn;
            case Easing::BackOut: return Easings::backOut;
            case Easing::BackInOut: return Easings::backInOut;
            case Easing::ElasticIn: return Easings::elasticIn;
            case Easing::ElasticOut: return Easings::elasticOut;
            case Easing::ElasticInOut: return Easings::elasticInOut;
            case Easing::BounceIn: return Easings::bounceIn;
            case Easing::BounceOut: return Easings::bounceOut;
            case Easing::BounceInOut: return Easings::bounceInOut;
            default: return Easings::linear;
        }
    }

    constexpr float PI = std::numbers::pi_v<float>;
    constexpr float HALF_PI = PI * 0.5f;
    constexpr float TWO_PI = PI * 2.f;

    float Easings::linear(float t) noexcept { return t; }

    float Easings::sineIn(float t) noexcept { return 1.f - std::cosf(t * HALF_PI); }
    float Easings::sineOut(float t) noexcept { return std::sinf(t * HALF_PI); }
    float Easings::sineInOut(float t) noexcept { return 0.5f * (1.f - std::cosf(t * PI)); }

    float Easings::quadIn(float t) noexcept { return t * t; }
    float Easings::quadOut(float t) noexcept { return t * (2.f - t); }
    float Easings::quadInOut(float t) noexcept {
        if (t < 0.5f) {
            return 2.f * t * t;
        }

        float f = t - 1.f;
        return 1.f - 2.f * f * f;
    }

    float Easings::cubicIn(float t) noexcept { return t * t * t; }
    float Easings::cubicOut(float t) noexcept { float f = t - 1.f; return f * f * f + 1.f; }
    float Easings::cubicInOut(float t) noexcept {
        if (t < 0.5f) {
            return 4.f * t * t * t;
        }

        float f = t - 1.f;
        return 4.f * f * f * f + 1.f;
    }

    float Easings::quartIn(float t) noexcept { return t * t * t * t; }
    float Easings::quartOut(float t) noexcept { float f = t - 1.f; return 1.f - f * f * f * f; }
    float Easings::quartInOut(float t) noexcept {
        if (t < 0.5f) {
            return 8.f * t * t * t * t;
        }

        float f = t - 1.f;
        return 1.f - 8.f * f * f * f * f;
    }

    float Easings::quintIn(float t) noexcept { return t * t * t * t * t; }
    float Easings::quintOut(float t) noexcept { float f = t - 1.f; return f * f * f * f * f + 1.f; }
    float Easings::quintInOut(float t) noexcept {
        if (t < 0.5f) {
            return 16.f * t * t * t * t * t;
        }

        float f = t - 1.f;
        return 16.f * f * f * f * f * f + 1.f;
    }

    float Easings::expoIn(float t) noexcept { return t == 0.f ? 0.f : std::powf(2.f, 10.f * (t - 1.f)); }
    float Easings::expoOut(float t) noexcept { return t == 1.f ? 1.f : 1.f - std::powf(2.f, -10.f * t); }
    float Easings::expoInOut(float t) noexcept {
        if (t == 0.f) return 0.f;
        if (t == 1.f) return 1.f;
        if (t < 0.5f) {
            return 0.5f * std::powf(2.f, 20.f * t - 10.f);
        }
        return 1.f - 0.5f * std::powf(2.f, -20.f * t + 10.f);
    }

    float Easings::circIn(float t) noexcept { return 1.f - std::sqrtf(1.f - t * t); }
    float Easings::circOut(float t) noexcept { float f = t - 1.f; return std::sqrtf(1.f - f * f); }
    float Easings::circInOut(float t) noexcept {
        if (t < 0.5f) {
            return 0.5f * (1.f - std::sqrtf(1.f - 4.f * t * t));
        }

        float f = t * 2.f - 2.f;
        return 0.5f * (std::sqrtf(1.f - f * f) + 1.f);
    }

    constexpr float BACK_S = 1.70158f;
    float Easings::backIn(float t) noexcept { return t * t * ((BACK_S + 1.f) * t - BACK_S); }
    float Easings::backOut(float t) noexcept { float f = t - 1.f; return f * f * ((BACK_S + 1.f) * f + BACK_S) + 1.f; }
    float Easings::backInOut(float t) noexcept {
        if (t < 0.5f) {
            float f = t * 2.f;
            return 0.5f * (f * f * ((BACK_S * 1.525f + 1.f) * f - BACK_S * 1.525f));
        }

        float f = t * 2.f - 2.f;
        return 0.5f * (f * f * ((BACK_S * 1.525f + 1.f) * f + BACK_S * 1.525f) + 2.f);
    }

    float Easings::elasticIn(float t) noexcept {
        if (t == 0.f) return 0.f;
        if (t == 1.f) return 1.f;
        float p = 0.3f;
        float s = p / 4.f;
        float f = t - 1.f;
        return -std::powf(2.f, 10.f * f) * std::sinf((f - s) * TWO_PI / p);
    }
    float Easings::elasticOut(float t) noexcept {
        if (t == 0.f) return 0.f;
        if (t == 1.f) return 1.f;
        float p = 0.3f;
        float s = p / 4.f;
        return std::powf(2.f, -10.f * t) * std::sinf((t - s) * TWO_PI / p) + 1.f;
    }
    float Easings::elasticInOut(float t) noexcept {
        if (t == 0.f) return 0.f;
        if (t == 1.f) return 1.f;
        float p = 0.45f;
        float s = p / 4.f;
        if (t < 0.5f) {
            float f = t * 2.f - 1.f;
            return -0.5f * std::powf(2.f, 10.f * f) * std::sinf((f - s) * TWO_PI / p);
        }
        float f = t * 2.f - 1.f;
        return std::powf(2.f, -10.f * f) * std::sinf((f - s) * TWO_PI / p) * 0.5f + 1.f;
    }

    constexpr float B1 = 1.f / 2.75f;
    constexpr float B2 = 2.f / 2.75f;
    constexpr float B3 = 1.5f / 2.75f;
    constexpr float B4 = 2.5f / 2.75f;
    float Easings::bounceIn(float t) noexcept {
        if (t < B1) {
            return 7.5625f * t * t;
        }
        if (t < B2) {
            float f = t - B3;
            return 7.5625f * f * f + 0.75f;
        }
        if (t < B4) {
            float f = t - B4;
            return 7.5625f * f * f + 0.9375f;
        }
        float f = t - 2.625f / 2.75f;
        return 7.5625f * f * f + 0.984375f;
    }
    float Easings::bounceOut(float t) noexcept {
        if (t < B1) {
            return 7.5625f * t * t;
        }
        if (t < B2) {
            float f = t - B3;
            return 7.5625f * f * f + 0.75f;
        }
        if (t < B4) {
            float f = t - B4;
            return 7.5625f * f * f + 0.9375f;
        }
        float f = t - 2.625f / 2.75f;
        return 7.5625f * f * f + 0.984375f;
    }
    float Easings::bounceInOut(float t) noexcept {
        if (t < 0.5f) {
            return 0.5f * bounceIn(t * 2.f);
        }
        return 0.5f * bounceOut(t * 2.f - 1.f) + 0.5f;
    }
}
