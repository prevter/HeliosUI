#pragma once
#ifndef HELIOS_GLBACKEND_HPP
#define HELIOS_GLBACKEND_HPP

#include "../Renderer/RenderBackend.hpp"

#include <cstdint>
#include <string>
#include <vector>

#if HELIOS_PREFER_GLEW
    #include <GL/glew.h>
#elif defined(HELIOS_OPENGL_LOADER_INCLUDE)
    #include HELIOS_OPENGL_LOADER_INCLUDE
#else
    #include <glad/gl.h>
#endif

namespace Helios {
    class GLBackend final : public RenderBackend {
    public:
        GLBackend() = default;
        ~GLBackend() override;

        bool init() override;
        void shutdown() override;

        BackendCaps const& getCaps() const override { return m_caps; }

        TextureHandle createTexture(TextureDesc const& desc) override;
        void updateTexture(
            TextureHandle texture,
            void const* data,
            uint32_t x, uint32_t y, uint32_t w, uint32_t h,
            TextureFormat fmt
        ) override;
        void destroyTexture(TextureHandle texture) override;

        TextureHandle whiteTexture() const override;

        void beginFrame(FrameDesc const& desc) override;
        void endFrame() override;

        void uploadGeometry(
            Vertex2D const* vertices, uint32_t vertCount, uint32_t const* indices, uint32_t indexCount
        ) override;
        void submitBatch(RenderBatch const& batch) override;

    private:
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ibo = 0;
        GLuint m_shader = 0;

        GLint m_uProj = -1;
        GLint m_uTexture = -1;
        GLint m_uLineWidth = -1;

        TextureHandle m_whiteTex;
        BackendCaps m_caps;

        uint32_t m_screenW = 0;
        uint32_t m_screenH = 0;
        int m_vpX = 0, m_vpY = 0, m_vpW = 0, m_vpH = 0;
        float m_virtualToScreen = 1.f;

        struct SavedState {
            GLint     blendSrc, blendDst;
            GLboolean blend, depthTest, cullFace, scissor;
            GLint     scissorBox[4];
            GLint     viewport[4];
            GLint     program, vao, activeTexture, texture;
            GLint     arrayBuffer;
        } m_saved;

        void setScissor(Rect const& rect) const;
        static void setBlendMode(BlendMode mode);
        void setupVAO() const;

        static GLuint compileStage(GLenum type, char const** parts, GLsizei count);
        bool buildShader();
    };
}

#endif // HELIOS_GLBACKEND_HPP
