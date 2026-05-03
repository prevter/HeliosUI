#pragma once
#ifndef HELIOS_EASINGS_HPP
#define HELIOS_EASINGS_HPP

#include <cstdint>

namespace Helios {
    using EasingFn = float(*)(float t) noexcept;

    enum class Easing : uint8_t {
        Linear,
        SineIn, SineOut, SineInOut,
        QuadIn, QuadOut, QuadInOut,
        CubicIn, CubicOut, CubicInOut,
        QuartIn, QuartOut, QuartInOut,
        QuintIn, QuintOut, QuintInOut,
        ExpoIn, ExpoOut, ExpoInOut,
        CircIn, CircOut, CircInOut,
        BackIn, BackOut, BackInOut,
        ElasticIn, ElasticOut, ElasticInOut,
        BounceIn, BounceOut, BounceInOut
    };

    EasingFn getEasingFunction(Easing type) noexcept;

    namespace Easings {
        float linear(float t) noexcept;
        float sineIn(float t) noexcept;
        float sineOut(float t) noexcept;
        float sineInOut(float t) noexcept;
        float quadIn(float t) noexcept;
        float quadOut(float t) noexcept;
        float quadInOut(float t) noexcept;
        float cubicIn(float t) noexcept;
        float cubicOut(float t) noexcept;
        float cubicInOut(float t) noexcept;
        float quartIn(float t) noexcept;
        float quartOut(float t) noexcept;
        float quartInOut(float t) noexcept;
        float quintIn(float t) noexcept;
        float quintOut(float t) noexcept;
        float quintInOut(float t) noexcept;
        float expoIn(float t) noexcept;
        float expoOut(float t) noexcept;
        float expoInOut(float t) noexcept;
        float circIn(float t) noexcept;
        float circOut(float t) noexcept;
        float circInOut(float t) noexcept;
        float backIn(float t) noexcept;
        float backOut(float t) noexcept;
        float backInOut(float t) noexcept;
        float elasticIn(float t) noexcept;
        float elasticOut(float t) noexcept;
        float elasticInOut(float t) noexcept;
        float bounceIn(float t) noexcept;
        float bounceOut(float t) noexcept;
        float bounceInOut(float t) noexcept;
    }
}

#endif // HELIOS_EASINGS_HPP
