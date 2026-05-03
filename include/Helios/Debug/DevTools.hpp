#pragma once
#if !defined(HELIOS_DEVTOOLS_HPP) && defined(HELIOS_ENABLE_DEVTOOLS)
#define HELIOS_DEVTOOLS_HPP
#include <optional>
#include <Helios/Math.hpp>

struct GLFWwindow;

namespace Helios {
    class DevTools {
    public:
        static void init(GLFWwindow* window);
        static void shutdown();

        static void prerender();
        static void render();

        static std::optional<Rect> getSelectedBounds();
    };
}

#endif // HELIOS_DEVTOOLS_HPP