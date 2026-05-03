#pragma once
#ifndef HELIOS_RENDERER_HPP
#define HELIOS_RENDERER_HPP

#include "DrawList.hpp"
#include "RenderBackend.hpp"
#include "Viewport.hpp"

#include <memory>

namespace Helios {
    class Renderer {
    public:
        Renderer();
        ~Renderer();

        static Renderer& get() noexcept;

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        RenderBackend& backend() noexcept { return *m_backend; }
        RenderBackend const& backend() const noexcept { return *m_backend; }

        void render(DrawList const& dl, uint32_t screenW, uint32_t screenH);
        void render(DrawList const& dl, Viewport const& vp, uint32_t screenW, uint32_t screenH);

        TextureHandle createTexture(TextureDesc const& desc) const { return m_backend->createTexture(desc); }
        void destroyTexture(TextureHandle texture) const { m_backend->destroyTexture(texture); }
        void updateTexture(
            TextureHandle texture, void const* data,
            uint32_t x, uint32_t y,
            uint32_t w, uint32_t h,
            TextureFormat fmt = TextureFormat::RGBA8
        ) const {
            m_backend->updateTexture(texture, data, x, y, w, h, fmt);
        }

        void init(std::unique_ptr<RenderBackend> backend);
        void shutdown();

    private:
        std::unique_ptr<RenderBackend> m_backend;

        std::vector<DrawCmd const*> m_sortedCmds;
        std::vector<uint32_t> m_sortedIndices;
        std::vector<RenderBatch> m_batches;
        std::vector<uint8_t> m_batchLayers;

        void renderWithDesc(DrawList const& dl, FrameDesc const& desc);
    };
}

#endif // HELIOS_RENDERER_HPP