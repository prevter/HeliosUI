#pragma once
#ifndef HELIOS_RENDER_BACKEND_HPP
#define HELIOS_RENDER_BACKEND_HPP

#include <string_view>

#include "DrawList.hpp"
#include "../Handles.hpp"
#include "../Math.hpp"

namespace Helios {
    enum class TextureFormat : uint8_t {
        RGBA8,
        RGB8,
        R8
    };

    enum class TextureFilter : uint8_t {
        Nearest,
        Linear
    };

    struct TextureDesc {
        void const* data = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        TextureFormat format = TextureFormat::RGBA8;
        TextureFilter filter = TextureFilter::Linear;
        bool generateMips = false;
    };

    struct RenderBatch {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
        TextureHandle texture;
        Rect clipRect = Rect::maxRect();
        BlendMode blend = BlendMode::Normal;
        bool usesTexture = false;
    };

    struct BackendCaps {
        int maxTextureSize = 2048;
        int glVersionMajor = 0;
        int glVersionMinor = 0;
        int glesVersion = 0;
        bool supportsNonPowerOfTwoTextures = true;
        bool supportsInstancing = false; // GL 3.1+ / ES 3.0+
        bool supportsUBO = false;        // GL 3.1+ / ES 3.0+
        bool supportsVAO = true;         // GL 3.0+ / ES 2.0+
        bool supportsMultiDraw = false;  // GL 4.3+ / ES 3.0+
        bool isGLES = false;
    };

    struct FrameDesc {
        uint32_t screenW = 0;
        uint32_t screenH = 0;
        int vpX = 0;
        int vpY = 0;
        int vpW = 0;
        int vpH = 0;
        float proj[16] = {};
        float virtualW = 0.f;
        float virtualH = 0.f;

        static FrameDesc native(uint32_t w, uint32_t h) noexcept {
            FrameDesc d{
                .screenW = w,
                .screenH = h,
                .vpX = 0,
                .vpY = 0,
                .vpW = static_cast<int>(w),
                .vpH = static_cast<int>(h),
                .virtualW = static_cast<float>(w),
                .virtualH = static_cast<float>(h)
            };

            buildOrthoProj(d.proj, 0.f, d.virtualW, 0.f, d.virtualH);
            return d;
        }

        static FrameDesc fromViewport(class Viewport const& vp, uint32_t w, uint32_t h) noexcept;

        static void buildOrthoProj(float out[16], float left, float right, float top, float bottom) noexcept {
            std::fill_n(out, 16, 0.f);
            out[0] = 2.f / (right - left);
            out[5] = 2.f / (top - bottom);
            out[10] = -1.f;
            out[12] = -(right + left) / (right - left);
            out[13] = -(top + bottom) / (top - bottom);
            out[15] = 1.f;
        }
    };

    class RenderBackend {
    public:
        virtual ~RenderBackend() = default;

        virtual bool init() = 0;
        virtual void shutdown() = 0;

        virtual BackendCaps const& getCaps() const = 0;

        virtual TextureHandle createTexture(TextureDesc const& desc) = 0;
        virtual void destroyTexture(TextureHandle texture) = 0;

        virtual void updateTexture(
            TextureHandle texture,
            void const* data,
            uint32_t x, uint32_t y,
            uint32_t w, uint32_t h,
            TextureFormat fmt = TextureFormat::RGBA8
        ) = 0;

        virtual TextureHandle whiteTexture() const = 0;

        virtual void beginFrame(FrameDesc const& desc) = 0;
        virtual void endFrame() = 0;

        virtual void uploadGeometry(Vertex2D const* vertices, uint32_t vertCount, uint32_t const* indices, uint32_t indexCount) = 0;
        virtual void submitBatch(RenderBatch const& batch) = 0;
    };
}

#endif // HELIOS_RENDER_BACKEND_HPP
