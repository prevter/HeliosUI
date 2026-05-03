#include <Helios/Renderer/Renderer.hpp>

#include <bit>
#include <chrono>
#include <Helios/Director.hpp>
#include <Helios/Backends/GLBackend.hpp>
#include <Helios/Debug/Instrumentation.hpp>

namespace Helios {
    Renderer::Renderer() = default;
    Renderer::~Renderer() = default;

    Renderer& Renderer::get() noexcept {
        return Director::get().renderer();
    }

    void Renderer::render(DrawList const& dl, uint32_t screenW, uint32_t screenH) {
        this->renderWithDesc(dl, FrameDesc::native(screenW, screenH));
    }

    void Renderer::render(DrawList const& dl, Viewport const& vp, uint32_t screenW, uint32_t screenH) {
        this->renderWithDesc(dl, FrameDesc::fromViewport(vp, screenW, screenH));
    }

    void Renderer::init(std::unique_ptr<RenderBackend> backend) {
        m_backend = std::move(backend);
    }

    void Renderer::shutdown() {
        m_backend.reset();
        m_sortedCmds.clear();
        m_sortedIndices.clear();
        m_batches.clear();
        m_batchLayers.clear();
    }

    static uint64_t cmdSortKey(DrawCmd const& c) noexcept {
        return static_cast<uint64_t>(c.layer) << 32 | std::bit_cast<uint32_t>(c.z);
    }

    void Renderer::renderWithDesc(DrawList const& dl, FrameDesc const& desc) {
        auto const frameStart = std::chrono::steady_clock::now();

        auto const& cmds = dl.getCommands();
        auto const& verts = dl.getVertices();
        auto const& indices = dl.getIndices();

        HELIOS_INSTRUMENT_COMMANDS(cmds.size());
        HELIOS_INSTRUMENT_VERTICES(verts.size());
        HELIOS_INSTRUMENT_INDICES(indices.size());

        if (cmds.empty() || verts.empty()) {
            m_backend->beginFrame(desc);
            m_backend->endFrame();
            double const frameMs = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - frameStart
            ).count();
            HELIOS_INSTRUMENT_FRAME_CPU_MS(frameMs);
            return;
        }

        m_sortedCmds.resize(cmds.size());
        for (size_t i = 0; i < cmds.size(); ++i) {
            m_sortedCmds[i] = &cmds[i];
        }

        {
            size_t groupStart = 0;
            while (groupStart < m_sortedCmds.size()) {
                Rect const& groupClip = m_sortedCmds[groupStart]->clipRect;
                size_t groupEnd = groupStart + 1;
                while (groupEnd < m_sortedCmds.size() && m_sortedCmds[groupEnd]->clipRect == groupClip) {
                    ++groupEnd;
                }
                if (groupEnd - groupStart > 1) {
                    std::stable_sort(
                        m_sortedCmds.begin() + groupStart,
                        m_sortedCmds.begin() + groupEnd,
                        [](DrawCmd const* a, DrawCmd const* b) {
                            return cmdSortKey(*a) < cmdSortKey(*b);
                        }
                    );
                }
                groupStart = groupEnd;
            }
        }

        m_sortedIndices.clear();
        m_sortedIndices.reserve(indices.size());
        m_batches.clear();
        m_batchLayers.clear();

        for (DrawCmd const* cmd : m_sortedCmds) {
            uint32_t newOffset = static_cast<uint32_t>(m_sortedIndices.size());
            m_sortedIndices.insert(
                m_sortedIndices.end(),
                indices.data() + cmd->indexOffset,
                indices.data() + cmd->indexOffset + cmd->indexCount
            );

            if (!m_batches.empty()) {
                auto& prev = m_batches.back();
                if (cmd->canBatch({
                    .texture = prev.texture,
                    .clipRect = prev.clipRect,
                    .blendMode = prev.blend,
                    .usesTexture = prev.usesTexture,
                })) {
                    prev.indexCount += cmd->indexCount;
                    if (cmd->usesTexture && !prev.usesTexture) {
                        prev.texture = cmd->texture;
                        prev.usesTexture = true;
                    }
                    HELIOS_INSTRUMENT_MERGED_COMMANDS(1);
                    continue;
                }
            }

            m_batches.emplace_back(
                static_cast<uint32_t>(newOffset * sizeof(uint32_t)),
                cmd->indexCount,
                cmd->texture,
                cmd->clipRect,
                cmd->blendMode,
                cmd->usesTexture
            );

            m_batchLayers.push_back(cmd->layer);
        }

        HELIOS_INSTRUMENT_BATCHES(m_batches.size());

        m_backend->beginFrame(desc);
        m_backend->uploadGeometry(
            verts.data(), static_cast<uint32_t>(verts.size()),
            m_sortedIndices.data(), static_cast<uint32_t>(m_sortedIndices.size())
        );

        uint8_t prevLayer = 0;
        for (size_t i = 0; i < m_batches.size(); ++i) {
            uint8_t curLayer = m_batchLayers[i];
            if (curLayer != prevLayer) {
                prevLayer = curLayer;
            }
            m_backend->submitBatch(m_batches[i]);
        }

        m_backend->endFrame();
        double const frameMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - frameStart
        ).count();
        HELIOS_INSTRUMENT_FRAME_CPU_MS(frameMs);
    }
}
