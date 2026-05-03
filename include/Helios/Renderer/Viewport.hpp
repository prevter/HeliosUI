#pragma once
#ifndef HELIOS_VIEWPORT_HPP
#define HELIOS_VIEWPORT_HPP

#include <cmath>
#include <cstring>
#include "../Math.hpp"

namespace Helios {
    class Viewport {
    public:
        explicit Viewport(Vec2 targetSize = Vec2(640.f, 360.f)) noexcept : m_target(targetSize) {}

        void update(float screenW, float screenH) noexcept {
            m_screenW = screenW;
            m_screenH = screenH;

            float screenAspect  = screenW / screenH;
            float virtualAspect = m_target.x / m_target.y;

            if (screenAspect < virtualAspect) {
                m_canvas.x = m_target.x;
                m_canvas.y = m_target.x / screenAspect;
            } else {
                m_canvas.x = m_target.y * screenAspect;
                m_canvas.y = m_target.y;
            }

            m_scale = screenW / m_canvas.x;
            this->buildOrthoProj();
        }

        Vec2 screenToVirtual(Vec2 screenPos) const noexcept { return screenPos / m_scale; }
        Vec2 screenToVirtual(float sx, float sy) const noexcept { return { sx / m_scale, sy / m_scale }; }

        Vec2 virtualToScreen(Vec2 virtualPos) const noexcept { return virtualPos * m_scale; }
        Vec2 virtualToScreen(float vx, float vy) const noexcept { return { vx * m_scale, vy * m_scale }; }

        Vec2 targetSize() const noexcept { return m_target; }
        Vec2 canvasSize() const noexcept { return m_canvas; }
        Vec2 screenSize() const noexcept { return Vec2(m_screenW, m_screenH); }
        float scale() const noexcept { return m_scale; }

        float const* projMatrix() const noexcept { return m_proj; }

    private:
        Vec2 m_target = {640.f, 360.f};
        Vec2 m_canvas = {640.f, 360.f};
        float m_screenW = 640;
        float m_screenH = 360;
        float m_scale = 1.f;
        float m_proj[16] = {};

        void buildOrthoProj() noexcept {
            float left = 0.f, right = m_canvas.x;
            float top = 0.f, bottom = m_canvas.y;
            std::fill_n(m_proj, 16, 0.f);

            m_proj[0] = 2.f / (right - left);
            m_proj[5] = 2.f / (top - bottom);
            m_proj[10] = -1.f;
            m_proj[12] = -(right + left) / (right - left);
            m_proj[13] = -(top + bottom) / (top - bottom);
            m_proj[15] = 1.f;
        }
    };
}

#include "RenderBackend.hpp" // avoid circular dep

namespace Helios {
    inline FrameDesc FrameDesc::fromViewport(
        Viewport const& vp, uint32_t w, uint32_t h
    ) noexcept {
        FrameDesc d{
            .screenW = w,
            .screenH = h,
            .vpX = 0,
            .vpY = 0,
            .vpW = static_cast<int>(w),
            .vpH = static_cast<int>(h),
            .virtualW = vp.canvasSize().x,
            .virtualH = vp.canvasSize().y
        };
        std::memcpy(d.proj, vp.projMatrix(), sizeof(float) * 16);
        return d;
    }
}

#endif // HELIOS_VIEWPORT_HPP
