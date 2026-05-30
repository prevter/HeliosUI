#include <cstring>
#include <Helios/Backends/GLBackend.hpp>
#include <Helios/Debug/Instrumentation.hpp>
#include <Helios/Debug/Log.hpp>

#include <utility>

#ifndef GL_LUMINANCE
    #define GL_LUMINANCE 0x1903
#endif
#ifndef GL_RED
    #define GL_RED  0x1903
#endif
#ifndef GL_R8
    #define GL_R8   0x8229
#endif
#ifndef GL_RGB8
    #define GL_RGB8 0x8051
#endif
#ifndef GL_RGBA8
    #define GL_RGBA8 0x8058
#endif
#ifndef GL_TEXTURE_SWIZZLE_RGBA
    #define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#endif
#ifndef GL_VERTEX_ARRAY_BINDING
    #define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif

namespace Helios {
    static constexpr GLuint ATTRIB_POS = 0;
    static constexpr GLuint ATTRIB_UV = 1;
    static constexpr GLuint ATTRIB_COLOR = 2;
    static constexpr GLuint ATTRIB_EXTRA = 3;
    static constexpr GLuint ATTRIB_CORNERS = 4;

    namespace Shaders {
        // Vertex - GLES 2 / GLSL 1.00
        constexpr auto kVertHeaderGLES2 =
                "#version 100\n"
                "precision mediump float;\n"
                "attribute vec2 a_pos;\n"
                "attribute vec2 a_uv;\n"
                "attribute vec4 a_color;\n"
                "attribute vec4 a_extra;\n"
                "attribute vec4 a_corners;\n"
                "uniform mat4 u_proj;\n"
                "varying vec2 v_uv;\n"
                "varying vec4 v_color;\n"
                "varying vec4 v_extra;\n"
                "varying vec4 v_corners;\n";

        // Vertex - GLES 3 / GLSL ES 3.00
        constexpr auto kVertHeaderGLES3 =
                "#version 300 es\n"
                "precision mediump float;\n"
                "layout(location = 0) in vec2 a_pos;\n"
                "layout(location = 1) in vec2 a_uv;\n"
                "layout(location = 2) in vec4 a_color;\n"
                "layout(location = 3) in vec4 a_extra;\n"
                "layout(location = 4) in vec4 a_corners;\n"
                "uniform mat4 u_proj;\n"
                "out vec2 v_uv;\n"
                "out vec4 v_color;\n"
                "out vec4 v_extra;\n"
                "out vec4 v_corners;\n";

        // Vertex - GL 3.3+ / GLSL 3.30 core
        constexpr auto kVertHeaderGL330 =
                "#version 330 core\n"
                "layout(location = 0) in vec2 a_pos;\n"
                "layout(location = 1) in vec2 a_uv;\n"
                "layout(location = 2) in vec4 a_color;\n"
                "layout(location = 3) in vec4 a_extra;\n"
                "layout(location = 4) in vec4 a_corners;\n"
                "uniform mat4 u_proj;\n"
                "out vec2 v_uv;\n"
                "out vec4 v_color;\n"
                "out vec4 v_extra;\n"
                "out vec4 v_corners;\n";

        // Vertex - GL 3.0-3.2 / GLSL 1.30
        constexpr auto kVertHeaderGL130 =
                "#version 130\n"
                "in vec2 a_pos;\n"
                "in vec2 a_uv;\n"
                "in vec4 a_color;\n"
                "in vec4 a_extra;\n"
                "in vec4 a_corners;\n"
                "uniform mat4 u_proj;\n"
                "out vec2 v_uv;\n"
                "out vec4 v_color;\n"
                "out vec4 v_extra;\n"
                "out vec4 v_corners;\n";

        // Vertex shader body
        constexpr auto kVertBody = R"GLSL(void main() {
    v_uv      = a_uv;
    v_color   = a_color;
    v_extra   = a_extra;
    v_corners = a_corners;
    gl_Position = u_proj * vec4(a_pos, 0.0, 1.0);
})GLSL";

        // Fragment - GLES 2 / GLSL 1.00
        constexpr auto kFragHeaderGLES2 =
                "#version 100\n"
                "#extension GL_OES_standard_derivatives : enable\n"
                "precision mediump float;\n"
                "varying vec2 v_uv;\n"
                "varying vec4 v_color;\n"
                "varying vec4 v_extra;\n"
                "varying vec4 v_corners;\n"
                "uniform sampler2D u_texture;\n"
                "uniform float u_lineWidth;\n"
                "#define FRAG_OUT gl_FragColor\n"
                "#define SAMPLE(uv) texture2D(u_texture, uv)\n";

        // Fragment - GLES 3 / GLSL ES 3.00
        constexpr auto kFragHeaderGLES3 =
                "#version 300 es\n"
                "precision mediump float;\n"
                "in vec2 v_uv;\n"
                "in vec4 v_color;\n"
                "in vec4 v_extra;\n"
                "in vec4 v_corners;\n"
                "uniform sampler2D u_texture;\n"
                "uniform float u_lineWidth;\n"
                "out vec4 frag_color;\n"
                "#define FRAG_OUT frag_color\n"
                "#define SAMPLE(uv) texture(u_texture, uv)\n";

        // Fragment - GL 3.3+ / GLSL 3.30 core
        constexpr auto kFragHeaderGL330 =
                "#version 330 core\n"
                "in vec2 v_uv;\n"
                "in vec4 v_color;\n"
                "in vec4 v_extra;\n"
                "in vec4 v_corners;\n"
                "uniform sampler2D u_texture;\n"
                "uniform float u_lineWidth;\n"
                "out vec4 frag_color;\n"
                "#define FRAG_OUT frag_color\n"
                "#define SAMPLE(uv) texture(u_texture, uv)\n";

        constexpr auto kFragHeaderGL130 =
                "#version 130\n"
                "in vec2 v_uv;\n"
                "in vec4 v_color;\n"
                "in vec4 v_extra;\n"
                "in vec4 v_corners;\n"
                "uniform sampler2D u_texture;\n"
                "uniform float u_lineWidth;\n"
                "out vec4 frag_color;\n"
                "#define FRAG_OUT frag_color\n"
                "#define SAMPLE(uv) texture(u_texture, uv)\n";

        constexpr auto kFragBody = R"GLSL(
float sdRoundBox(vec2 p, vec2 halfSize, vec4 corners) {
    vec2 r;
    r.xy = (p.x > 0.0) ? corners.yz : corners.xw;
    float radius = (p.y > 0.0) ? r.y : r.x;
    vec2 q = abs(p) - halfSize + radius;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - radius;
}

float sdCircle(vec2 p, float r) { return length(p) - r; }

float aaMask(float d) {
    float fw = length(vec2(dFdx(d), dFdy(d)));
    return 1.0 - smoothstep(-fw * 0.5, fw * 0.5, d);
}

float median(float a, float b, float c) {
    return max(min(a, b), min(max(a, b), c));
}

void main() {
    int mode = int(v_extra.x + 0.5);
    if (mode == 0) { // Solid
        FRAG_OUT = v_color;
    } else if (mode == 1) { // Textured
        FRAG_OUT = v_color * SAMPLE(v_uv);
    } else if (mode == 2) { // RoundedRect
        float d = sdRoundBox(v_uv, v_extra.yz, v_corners);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * aaMask(d));
    } else if (mode == 3) { // Circle
        float d = sdCircle(v_uv, v_extra.y);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * aaMask(d));
    } else if (mode == 4) { // TextSDF
        float sdf = SAMPLE(v_uv).r;
        float fw = length(vec2(dFdx(sdf), dFdy(sdf)));
        float alpha = smoothstep(0.5 - fw, 0.5 + fw, sdf);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * alpha);
    } else if (mode == 5) { // RoundedBorder
        float d = sdRoundBox(v_uv, v_extra.yz, v_corners);
        float thickness = v_extra.w;
        float fw = length(vec2(dFdx(d), dFdy(d)));
        float outer = 1.0 - smoothstep(-fw * 0.5, fw * 0.5, d);
        float inner = smoothstep(-fw * 0.5, fw * 0.5, d + thickness);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * outer * inner);
    } else if (mode == 6) { // CircleBorder
        float outer = aaMask(sdCircle(v_uv, v_extra.y));
        float inner = 1.0 - aaMask(sdCircle(v_uv, v_extra.z));
        FRAG_OUT = vec4(v_color.rgb, v_color.a * outer * inner);
    } else if (mode == 8) { // ArcSegment
        float ringMask = aaMask(sdCircle(v_uv, v_extra.y)) * (1.0 - aaMask(sdCircle(v_uv, v_extra.z)));
        vec2 dS = vec2(cos(v_corners.x), sin(v_corners.x));
        vec2 dE = vec2(cos(v_corners.y), sin(v_corners.y));
        float cS = dS.x * v_uv.y - dS.y * v_uv.x;
        float cE = dE.x * v_uv.y - dE.y * v_uv.x;
        float mS = smoothstep(-max(fwidth(cS), 1e-4), max(fwidth(cS), 1e-4), cS);
        float mE = 1.0 - smoothstep(-max(fwidth(cE), 1e-4), max(fwidth(cE), 1e-4), cE);
        float crossSE = dS.x * dE.y - dS.y * dE.x;
        float angMask = (crossSE >= 0.0) ? mS * mE : max(mS, mE);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * ringMask * angMask);
    } else if (mode == 9) { // TextMSDF
        vec3 s = SAMPLE(v_uv).rgb;
        float sd = median(s.r, s.g, s.b);
        float fw = length(vec2(dFdx(sd), dFdy(sd)));
        float alpha = smoothstep(0.5 - fw, 0.5 + fw, sd);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * alpha);
    } else if (mode == 10) { // TextMTSDF
        vec4 s = SAMPLE(v_uv);
        float sd = median(s.r, s.g, s.b);
        float fw = length(vec2(dFdx(sd), dFdy(sd)));
        float msdfAlpha = smoothstep(0.5 - fw, 0.5 + fw, sd);
        float sdfAlpha = smoothstep(0.5 - fw, 0.5 + fw, s.a);
        float alpha = max(msdfAlpha, sdfAlpha);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * alpha);
    } else if (mode == 11) { // RadialGradient
        float dist = length(v_uv);
        float t = clamp(dist / v_extra.y, 0.0, 1.0);
        vec4 col = mix(v_color, v_corners, t);
        float d = sdCircle(v_uv, v_extra.y);
        FRAG_OUT = vec4(col.rgb, col.a * aaMask(d));
    } else if (mode == 12) { // RectGlow
        float d = sdRoundBox(v_uv, v_extra.yz, v_corners);
        float sigma = max(v_extra.w, 0.001);
        float alpha = exp(-abs(d) / sigma);
        FRAG_OUT = vec4(v_color.rgb, v_color.a * alpha);
    } else {
        FRAG_OUT = v_color;
    }
})GLSL";
    }

    GLBackend::~GLBackend() {
        GLBackend::shutdown();
    }

    bool GLBackend::init() {
        auto verStr = reinterpret_cast<char const*>(glGetString(GL_VERSION));
        if (!verStr) {
            Log::error("[HeliosUI] glGetString(GL_VERSION) returned null");
            return false;
        }

        m_caps.isGLES = std::strstr(verStr, "OpenGL ES") != nullptr;

        char const* numStart = verStr;
        while (*numStart && (*numStart < '0' || *numStart > '9')) ++numStart;
        int major = 0, minor = 0;
        if (std::sscanf(numStart, "%d.%d", &major, &minor) != 2) {
            Log::error("[HeliosUI] Failed to parse GL version: {}", verStr);
            return false;
        }

        m_caps.glVersionMajor = major;
        m_caps.glVersionMinor = minor;

        if (m_caps.isGLES) {
            m_caps.glesVersion = major;
            if (major < 2) {
                Log::error("[HeliosUI] OpenGL ES {}.{} is below minimum (2.0)", major, minor);
                return false;
            }
        } else {
            if (major < 3) {
                Log::error("[HeliosUI] OpenGL {}.{} is below minimum (3.0)", major, minor);
                return false;
            }
        }

        bool gles3orDesktop31 = m_caps.isGLES ? major >= 3 : major > 3 || minor >= 1;
        m_caps.supportsVAO = true;
        m_caps.supportsUBO = gles3orDesktop31;
        m_caps.supportsInstancing = gles3orDesktop31;

        GLint maxTex = 0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTex);
        m_caps.maxTextureSize = maxTex;

        Log::info("[HeliosUI] Using {} {}.{}", m_caps.isGLES ? "GLES" : "GL", major, minor);

        if (!buildShader()) return false;

        m_uProj = glGetUniformLocation(m_shader, "u_proj");
        m_uTexture = glGetUniformLocation(m_shader, "u_texture");
        m_uLineWidth = glGetUniformLocation(m_shader, "u_lineWidth");

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ibo);
        setupVAO();

        constexpr uint8_t white[4] = {255, 255, 255, 255};
        m_whiteTex = this->createTexture(
            {
                .data = white,
                .width = 1,
                .height = 1,
                .format = TextureFormat::RGBA8,
                .filter = TextureFilter::Nearest,
                .generateMips = false
            }
        );

        return m_whiteTex.isValid();
    }

    void GLBackend::shutdown() {
        if (m_whiteTex) {
            this->destroyTexture(m_whiteTex);
            m_whiteTex = {};
        }
        if (m_vao) {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_vbo) {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_ibo) {
            glDeleteBuffers(1, &m_ibo);
            m_ibo = 0;
        }
        if (m_shader) {
            glDeleteProgram(m_shader);
            m_shader = 0;
        }
    }

    static GLenum uploadFormat(TextureFormat fmt, bool gles2) {
        switch (fmt) {
            case TextureFormat::RGBA8: return GL_RGBA;
            case TextureFormat::RGB8: return GL_RGB;
            case TextureFormat::R8: return gles2 ? GL_LUMINANCE : GL_RED;
        }
        return GL_RGBA;
    }

    static GLenum internalFormat(TextureFormat fmt, bool gles2) {
        if (gles2) return uploadFormat(fmt, true);
        switch (fmt) {
            case TextureFormat::RGBA8: return GL_RGBA8;
            case TextureFormat::RGB8: return GL_RGB8;
            case TextureFormat::R8: return GL_R8;
        }
        return GL_RGBA8;
    }

    TextureHandle GLBackend::createTexture(TextureDesc const& desc) {
        bool gles2 = m_caps.isGLES && m_caps.glesVersion < 3;

        GLuint id = 0;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        GLenum ufmt = uploadFormat(desc.format, gles2);
        GLenum ifmt = internalFormat(desc.format, gles2);
        GLenum filter = desc.filter == TextureFilter::Linear ? GL_LINEAR : GL_NEAREST;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (desc.format == TextureFormat::R8) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            if (!gles2) {
                GLint swizzle[] = {GL_RED, GL_RED, GL_RED, GL_RED};
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);
            }
        }

        glTexImage2D(
            GL_TEXTURE_2D, 0, static_cast<GLint>(ifmt),
            static_cast<GLsizei>(desc.width),
            static_cast<GLsizei>(desc.height),
            0, ufmt, GL_UNSIGNED_BYTE, desc.data
        );

        if (desc.format == TextureFormat::R8)
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        if (desc.generateMips && desc.data)
            glGenerateMipmap(GL_TEXTURE_2D);

        return TextureHandle(id);
    }

    void GLBackend::updateTexture(
        TextureHandle texture,
        void const* data,
        uint32_t x, uint32_t y,
        uint32_t w, uint32_t h,
        TextureFormat fmt
    ) {
        bool gles2 = m_caps.isGLES && m_caps.glesVersion < 3;
        glBindTexture(GL_TEXTURE_2D, texture.value);
        if (fmt == TextureFormat::R8) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        glTexSubImage2D(
            GL_TEXTURE_2D, 0,
            static_cast<GLint>(x), static_cast<GLint>(y),
            static_cast<GLsizei>(w), static_cast<GLsizei>(h),
            uploadFormat(fmt, gles2), GL_UNSIGNED_BYTE, data
        );
        if (fmt == TextureFormat::R8) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void GLBackend::destroyTexture(TextureHandle texture) {
        if (!texture) return;
        glDeleteTextures(1, &texture.value);
    }

    TextureHandle GLBackend::whiteTexture() const {
        return m_whiteTex;
    }

    void GLBackend::beginFrame(FrameDesc const& desc) {
        m_screenW = desc.screenW;
        m_screenH = desc.screenH;
        m_vpX = desc.vpX;
        m_vpY = desc.vpY;
        m_vpW = desc.vpW;
        m_vpH = desc.vpH;
        m_virtualToScreen = desc.virtualW > 0.0f
                                ? static_cast<float>(m_vpW) / desc.virtualW
                                : 1.0f;

        glGetIntegerv(GL_BLEND_SRC_RGB, &m_saved.blendSrc);
        glGetIntegerv(GL_BLEND_DST_RGB, &m_saved.blendDst);
        glGetIntegerv(GL_VIEWPORT, m_saved.viewport);
        glGetIntegerv(GL_SCISSOR_BOX, m_saved.scissorBox);
        glGetIntegerv(GL_CURRENT_PROGRAM, &m_saved.program);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_saved.arrayBuffer);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &m_saved.activeTexture);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &m_saved.texture);
        m_saved.blend = glIsEnabled(GL_BLEND);
        m_saved.scissor = glIsEnabled(GL_SCISSOR_TEST);
        m_saved.depthTest = glIsEnabled(GL_DEPTH_TEST);
        m_saved.cullFace = glIsEnabled(GL_CULL_FACE);

        if (!m_caps.isGLES || m_caps.glesVersion >= 3) {
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_saved.vao);
        } else {
            m_saved.vao = 0;
        }

        glViewport(m_vpX, m_vpY, m_vpW, m_vpH);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(m_shader);
        glBindVertexArray(m_vao);
        glActiveTexture(GL_TEXTURE0);
        glUniformMatrix4fv(m_uProj, 1, GL_FALSE, desc.proj);
        glUniform1i(m_uTexture, 0);
        glUniform1f(m_uLineWidth, 1.0f);
    }

    void GLBackend::endFrame() {
        glUseProgram(static_cast<GLuint>(m_saved.program));
        glBindVertexArray(static_cast<GLuint>(m_saved.vao));
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(m_saved.arrayBuffer));
        glActiveTexture(static_cast<GLenum>(m_saved.activeTexture));
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(m_saved.texture));
        glViewport(
            m_saved.viewport[0], m_saved.viewport[1],
            m_saved.viewport[2], m_saved.viewport[3]
        );
        glScissor(
            m_saved.scissorBox[0], m_saved.scissorBox[1],
            m_saved.scissorBox[2], m_saved.scissorBox[3]
        );
        glBlendFunc(
            static_cast<GLenum>(m_saved.blendSrc),
            static_cast<GLenum>(m_saved.blendDst)
        );
        m_saved.blend ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
        m_saved.scissor ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
        m_saved.depthTest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        m_saved.cullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
    }

    void GLBackend::uploadGeometry(
        Vertex2D const* vertices, uint32_t vertCount,
        uint32_t const* indices, uint32_t indexCount
    ) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(vertCount * sizeof(Vertex2D)),
            vertices, GL_STREAM_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(indexCount * sizeof(uint32_t)),
            indices, GL_STREAM_DRAW
        );
    }

    void GLBackend::submitBatch(RenderBatch const& batch) {
        if (batch.indexCount == 0) return;

        this->setBlendMode(batch.blend);
        this->setScissor(batch.clipRect);

        TextureHandle tex = batch.texture ? batch.texture : m_whiteTex;
        glBindTexture(GL_TEXTURE_2D, tex.value);

        glDrawElements(
            GL_TRIANGLES,
            static_cast<GLsizei>(batch.indexCount),
            GL_UNSIGNED_INT,
            reinterpret_cast<void const*>(static_cast<uintptr_t>(batch.indexOffset))
        );
        HELIOS_INSTRUMENT_DRAW_CALLS(1);
    }

    void GLBackend::setScissor(Rect const& rect) const {
        if (rect.w >= 1e17f) {
            glDisable(GL_SCISSOR_TEST);
            return;
        }

        GLint gx = static_cast<GLint>(rect.x * m_virtualToScreen);
        GLint gw = static_cast<GLint>(rect.w * m_virtualToScreen);
        GLint gh = static_cast<GLint>(rect.h * m_virtualToScreen);
        GLint gy = static_cast<GLint>(static_cast<float>(m_screenH) - rect.y * m_virtualToScreen - gh);

        if (gx < 0) {
            gw += gx;
            gx = 0;
        }
        if (gy < 0) {
            gh += gy;
            gy = 0;
        }
        gw = std::min(gw, static_cast<GLint>(m_screenW) - gx);
        gh = std::min(gh, static_cast<GLint>(m_screenH) - gy);
        if (gw <= 0 || gh <= 0) gw = gh = 0;

        glEnable(GL_SCISSOR_TEST);
        glScissor(gx, gy, gw, gh);
    }

    void GLBackend::setBlendMode(BlendMode mode) {
        switch (mode) {
            case BlendMode::Normal: glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case BlendMode::Additive: glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            case BlendMode::Multiply: glBlendEquation(GL_FUNC_ADD);
                glBlendFunc(GL_DST_COLOR, GL_ZERO);
                break;
            case BlendMode::None: glDisable(GL_BLEND);
                return;
        }
        glEnable(GL_BLEND);
    }

    void GLBackend::setupVAO() const {
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

        GLsizei stride = sizeof(Vertex2D);

        glEnableVertexAttribArray(ATTRIB_POS);
        glVertexAttribPointer(
            ATTRIB_POS, 2, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void*>(offsetof(Vertex2D, pos))
        );

        glEnableVertexAttribArray(ATTRIB_UV);
        glVertexAttribPointer(
            ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void*>(offsetof(Vertex2D, uv))
        );

        glEnableVertexAttribArray(ATTRIB_COLOR);
        glVertexAttribPointer(
            ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride,
            reinterpret_cast<void*>(offsetof(Vertex2D, color))
        );

        glEnableVertexAttribArray(ATTRIB_EXTRA);
        glVertexAttribPointer(
            ATTRIB_EXTRA, 4, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void*>(offsetof(Vertex2D, mode))
        );

        glEnableVertexAttribArray(ATTRIB_CORNERS);
        glVertexAttribPointer(
            ATTRIB_CORNERS, 4, GL_FLOAT, GL_FALSE, stride,
            reinterpret_cast<void*>(offsetof(Vertex2D, c0))
        );

        glBindVertexArray(0);
    }

    GLuint GLBackend::compileStage(GLenum type, char const** parts, GLsizei count) {
        GLuint id = glCreateShader(type);
        glShaderSource(id, count, parts, nullptr);
        glCompileShader(id);

        GLint ok = 0;
        glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[1024];
            glGetShaderInfoLog(id, sizeof(log), nullptr, log);
            Log::error("[HeliosUI] Shader compile error:\n{}", log);
            int line = 1;
            fmt::memory_buffer buf;
            fmt::format_to(std::back_inserter(buf), "\n---\n{:>3} | ", line);
            for (GLsizei i = 0; i < count; ++i) {
                for (char const* c = parts[i]; *c; ++c) {
                    fmt::format_to(std::back_inserter(buf), "{}", *c);
                    if (*c == '\n') fmt::format_to(std::back_inserter(buf), "{:>3} | ", ++line);
                }
            }
            Log::error("{}\n---", std::string_view(buf.data(), buf.size()));
            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    bool GLBackend::buildShader() {
        bool gles2 = m_caps.isGLES && m_caps.glesVersion < 3;
        bool useLayoutLoc = m_caps.isGLES
                                ? m_caps.glesVersion >= 3
                                : m_caps.glVersionMajor > 3
                                  || (m_caps.glVersionMajor == 3 && m_caps.glVersionMinor >= 3);

        auto vertHeader = gles2
                              ? Shaders::kVertHeaderGLES2
                              : m_caps.isGLES
                                    ? Shaders::kVertHeaderGLES3
                                    : useLayoutLoc
                                          ? Shaders::kVertHeaderGL330
                                          : Shaders::kVertHeaderGL130;

        auto fragHeader = gles2
                              ? Shaders::kFragHeaderGLES2
                              : m_caps.isGLES
                                    ? Shaders::kFragHeaderGLES3
                                    : useLayoutLoc
                                          ? Shaders::kFragHeaderGL330
                                          : Shaders::kFragHeaderGL130;

        char const* vertParts[] = {vertHeader, Shaders::kVertBody};
        char const* fragParts[] = {fragHeader, Shaders::kFragBody};

        GLuint vert = compileStage(GL_VERTEX_SHADER, vertParts, 2);
        GLuint frag = compileStage(GL_FRAGMENT_SHADER, fragParts, 2);
        if (!vert || !frag) {
            if (vert) glDeleteShader(vert);
            if (frag) glDeleteShader(frag);
            return false;
        }

        m_shader = glCreateProgram();
        glAttachShader(m_shader, vert);
        glAttachShader(m_shader, frag);

        if (!useLayoutLoc) {
            glBindAttribLocation(m_shader, ATTRIB_POS, "a_pos");
            glBindAttribLocation(m_shader, ATTRIB_UV, "a_uv");
            glBindAttribLocation(m_shader, ATTRIB_COLOR, "a_color");
            glBindAttribLocation(m_shader, ATTRIB_EXTRA, "a_extra");
            glBindAttribLocation(m_shader, ATTRIB_CORNERS, "a_corners");
        }

        glLinkProgram(m_shader);
        GLint ok = 0;
        glGetProgramiv(m_shader, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetProgramInfoLog(m_shader, sizeof(log), nullptr, log);
            Log::error("[HeliosUI] Shader link error:\n{}", log);
            glDeleteProgram(m_shader);
            m_shader = 0;
            glDeleteShader(vert);
            glDeleteShader(frag);
            return false;
        }

        glDetachShader(m_shader, vert);
        glDeleteShader(vert);
        glDetachShader(m_shader, frag);
        glDeleteShader(frag);

        return true;
    }
}
