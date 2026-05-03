#pragma once
#ifndef HELIOS_HANDLES_HPP
#define HELIOS_HANDLES_HPP

#include <cstdint>
#include <functional>

namespace Helios {
    template <typename Tag, typename T = uint32_t>
    struct BaseHandle {
        T value = 0;

        constexpr BaseHandle() noexcept = default;
        constexpr explicit BaseHandle(T v) noexcept : value(v) {}

        constexpr bool isValid() const noexcept { return value != 0; }
        constexpr operator bool() const noexcept { return isValid(); }

        constexpr bool operator==(BaseHandle other) const noexcept { return value == other.value; }
        constexpr bool operator!=(BaseHandle other) const noexcept { return value != other.value; }
        constexpr bool operator<(BaseHandle other) const noexcept { return value < other.value; }

        constexpr BaseHandle operator++() noexcept { return BaseHandle(++value); }
        constexpr BaseHandle operator++(int) noexcept { return BaseHandle(value++); }

        static constexpr BaseHandle invalid() noexcept { return BaseHandle(0); }
    };

    struct TextureTag {};
    struct ShaderTag {};
    struct BufferTag {};
    struct FontTag {};
    struct ActionTag {};

    using TextureHandle = BaseHandle<TextureTag>;
    using ShaderHandle = BaseHandle<ShaderTag>;
    using BufferHandle = BaseHandle<BufferTag>;
    using FontHandle = BaseHandle<FontTag>;
    using ActionHandle = BaseHandle<ActionTag>;
}

template <typename Tag, typename T>
struct std::hash<Helios::BaseHandle<Tag, T>> {
    size_t operator()(Helios::BaseHandle<Tag, T> handle) const noexcept {
        return std::hash<T>()(handle.value);
    }
};

#endif // HELIOS_HANDLES_HPP