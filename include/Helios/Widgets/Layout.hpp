#pragma once
#ifndef HELIOS_WIDGETS_LAYOUT_HPP
#define HELIOS_WIDGETS_LAYOUT_HPP

#include <cstdint>

namespace Helios {
    struct EdgeInsets {
        float left = 0.f;
        float top = 0.f;
        float right = 0.f;
        float bottom = 0.f;

        constexpr EdgeInsets() noexcept = default;
        constexpr EdgeInsets(float l, float t, float r, float b) noexcept : left(l), top(t), right(r), bottom(b) {}

        static constexpr EdgeInsets all(float e) noexcept { return {e, e, e, e}; }
        static constexpr EdgeInsets symmetric(float h, float v) noexcept { return {h, v, h, v}; }

        constexpr float horizontal() const noexcept { return left + right; }
        constexpr float vertical() const noexcept { return top + bottom; }
    };

    struct LayoutParams {
        enum class SizeMode : uint8_t { Fit, Fill };
        enum class Align : uint8_t { Unset, Start, Center, End, Stretch };

        float weight = 1.f;
        SizeMode widthMode = SizeMode::Fit;
        SizeMode heightMode = SizeMode::Fit;
        Align alignSelf = Align::Unset;
    };

    enum class Arrangement : uint8_t {
        Start,
        End,
        Center,
        SpaceBetween,
        SpaceAround,
        SpaceEvenly
    };

    enum class Alignment : uint8_t {
        Start,
        Center,
        End,
        Stretch
    };
}

#endif // HELIOS_WIDGETS_LAYOUT_HPP