#pragma once
#ifndef HELIOS_INPUT_TYPES_HPP
#define HELIOS_INPUT_TYPES_HPP

#include <cstdint>

namespace Helios {
    enum Modifiers : uint8_t {
        None  = 0,
        Shift = 1 << 0,
        Ctrl  = 1 << 1,
        Alt   = 1 << 2,
        Super = 1 << 3, // Win on Windows, Cmd on macOS
    };

    constexpr Modifiers operator|(Modifiers a, Modifiers b) noexcept {
        return static_cast<Modifiers>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    constexpr Modifiers operator&(Modifiers a, Modifiers b) noexcept {
        return static_cast<Modifiers>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    constexpr Modifiers operator~(Modifiers mods) noexcept {
        return static_cast<Modifiers>(~static_cast<uint8_t>(mods));
    }

    constexpr bool any(Modifiers mods) noexcept {
        return static_cast<uint8_t>(mods) != 0;
    }

    enum class MouseButton : uint8_t {
        Left    = 0,
        Right   = 1,
        Middle  = 2,
        Button4 = 3,
        Button5 = 4
    };

    enum class MouseCursor : uint8_t {
        Default = 0,
        Text,
        Hand,
        Crosshair,
        ResizeEW,
        ResizeNS,
        ResizeNWSE,
        ResizeNESW,
        NotAllowed,
        Hidden
    };

    enum class KeyCode : uint8_t {
        Unknown = 0,

        // Navigation
        Left, Right, Up, Down,
        Home, End, PageUp, PageDown,

        // Editing
        Backspace, Tab, Enter, Escape, Space,
        Delete, Insert,

        // Modifiers
        LeftShift, RightShift,
        LeftCtrl, RightCtrl,
        LeftAlt, RightAlt,
        LeftSuper, RightSuper,

        // Function keys
        F1, F2, F3, F4, F5, F6,
        F7, F8, F9, F10, F11, F12,
        F13, F14, F15, F16, F17, F18,
        F19, F20, F21, F22, F23, F24,

        // Lock keys
        CapsLock, NumLock, ScrollLock,

        // Misc
        PrintScreen, Pause, Menu,

        // Numpad
        NumPad0, NumPad1, NumPad2, NumPad3, NumPad4,
        NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
        NumPadAdd, NumPadSub, NumPadMul, NumPadDiv,
        NumPadDecimal, NumPadEnter,
    };
}

#endif // HELIOS_INPUT_TYPES_HPP
