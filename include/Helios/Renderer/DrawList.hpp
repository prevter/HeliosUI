#pragma once
#ifndef HELIOS_RENDERER_DRAWLIST_HPP
#define HELIOS_RENDERER_DRAWLIST_HPP

#include <cstdint>

#include "../Handles.hpp"
#include "../Math.hpp"

namespace Helios {
    namespace Layer {
        constexpr uint8_t Background = 0;
        constexpr uint8_t Content = 64;
        constexpr uint8_t Text = 128;
        constexpr uint8_t Overlay = 192;
        constexpr uint8_t Debug = 255;
    }

    struct GlyphQuad {
        Rect dest;
        Rect uv;
        Color color;
    };

    struct CornerRadii {
        float tl = 0.f;
        float tr = 0.f;
        float br = 0.f;
        float bl = 0.f;

        static constexpr CornerRadii all(float r) noexcept { return {r, r, r, r}; }
        static constexpr CornerRadii none() noexcept { return {0.f, 0.f, 0.f, 0.f}; }
        static constexpr CornerRadii top(float r) noexcept { return {r, r, 0.f, 0.f}; }
        static constexpr CornerRadii bottom(float r) noexcept { return {0.f, 0.f, r, r}; }
        static constexpr CornerRadii left(float r) noexcept { return {r, 0.f, 0.f, r}; }
        static constexpr CornerRadii right(float r) noexcept { return {0.f, r, r, 0.f}; }
        static constexpr CornerRadii topLeft(float r) noexcept { return {r, 0.f, 0.f, 0.f}; }
        static constexpr CornerRadii topRight(float r) noexcept { return {0.f, r, 0.f, 0.f}; }
        static constexpr CornerRadii bottomRight(float r) noexcept { return {0.f, 0.f, r, 0.f}; }
        static constexpr CornerRadii bottomLeft(float r) noexcept { return {0.f, 0.f, 0.f, r}; }

        constexpr CornerRadii clamped(float w, float h) const noexcept {
            float m = std::min(w, h) * 0.5f;
            return {
                std::min(tl, m),
                std::min(tr, m),
                std::min(br, m),
                std::min(bl, m)
            };
        }
    };

    struct alignas(4) Vertex2D {
        Vec2 pos;
        Vec2 uv;
        Color color;
        float mode;
        float ex, ey, ez;
        float c0, c1, c2, c3;
    };

    static_assert(sizeof(Vertex2D) == 52, "Vertex2D size mismatch");

    enum class DrawMode : uint8_t {
        Solid          = 0,
        Textured       = 1,
        RoundedRect    = 2,
        Circle         = 3,
        TextSDF        = 4,
        RoundedBorder  = 5,
        CircleBorder   = 6,
        Line           = 7,
        ArcSegment     = 8,
        TextMSDF       = 9,
        TextMTSDF      = 10,
        RadialGradient = 11,
        RectGlow       = 12
    };

    enum class BlendMode : uint8_t {
        Normal,
        Additive,
        Multiply,
        None
    };

    struct DrawCmd {
        uint32_t indexOffset = 0;
        uint32_t indexCount = 0;
        TextureHandle texture;
        Rect clipRect = Rect::maxRect();
        float z = 0.f;
        BlendMode blendMode = BlendMode::Normal;
        uint8_t layer = Layer::Content;
        bool usesTexture = false;

        bool canMerge(DrawCmd const& o) const noexcept {
            if (layer != o.layer) return false;
            if (blendMode != o.blendMode) return false;
            if (clipRect != o.clipRect) return false;
            if (usesTexture && o.usesTexture && texture != o.texture) return false;
            return true;
        }

        bool canBatch(DrawCmd const& o) const noexcept {
            if (blendMode != o.blendMode) return false;
            if (clipRect != o.clipRect) return false;
            if (usesTexture && o.usesTexture && texture != o.texture) return false;
            return true;
        }
    };

    class DrawList {
    public:
        DrawList() noexcept;
        ~DrawList() = default;

        DrawList(DrawList const&) = delete;
        DrawList& operator=(DrawList const&) = delete;
        DrawList(DrawList&&) noexcept = default;
        DrawList& operator=(DrawList&&) noexcept = default;

        void pushTransform(Mat3 const& t);
        void popTransform();

        void pushClipRect(Rect const& r, bool intersect = true);
        void popClipRect();

        void fillRect(Rect const& r, Color color, float z = 0.f);
        void fillRectGradient(
            Rect const& rect,
            Color topLeft, Color topRight, Color bottomLeft, Color bottomRight,
            float z = 0.f
        );

        void fillRoundedRect(Rect const& r, float radius, Color color, float z = 0.f);
        void fillRoundedRect(Rect const& r, CornerRadii radii, Color color, float z = 0.f);
        void fillRoundedRectGradient(
            Rect const& r, float radius,
            Color topLeft, Color topRight, Color bottomLeft, Color bottomRight,
            float z = 0.f
        );
        void fillRoundedRectGradient(
            Rect const& r, CornerRadii radii,
            Color topLeft, Color topRight, Color bottomLeft, Color bottomRight,
            float z = 0.f
        );

        void fillCircle(Vec2 center, float radius, Color color, float z = 0.f);
        void fillEllipse(Vec2 center, float radiusX, float radiusY, Color color, float z = 0.f);
        void fillCircleGradient(Vec2 center, float radius, Color innerColor, Color outerColor, float z = 0.f);

        void fillArcSegment(
            Vec2 center,
            float radius,
            float startAngleRad,
            float endAngleRad,
            float inset,
            float lineWidth,
            Color color,
            float z = 0.f
        );

        void fillRectGlow(Rect const& rect, float blur, Color color, float z = 0.f);
        void fillRoundedRectGlow(Rect const& rect, float radius, float blur, Color color, float z = 0.f);
        void fillRoundedRectGlow(Rect const& rect, CornerRadii radii, float blur, Color color, float z = 0.f);

        void fillConvexPoly(Vec2 const* pts, size_t count, Color color, float z = 0.f);
        void fillTriangle(Vec2 a, Vec2 b, Vec2 c, Color color, float z = 0.f);

        void strokeRect(Rect const& r, Color color, float thickness = 1.f, float z = 0.f);
        void strokeRoundedRect(Rect const& r, float radius, Color color, float thickness = 1.f, float z = 0.f);
        void strokeRoundedRect(Rect const& r, CornerRadii radii, Color color, float thickness = 1.f, float z = 0.f);
        void strokeCircle(Vec2 center, float radius, Color color, float thickness = 1.f, float z = 0.f);
        void strokeLine(Vec2 a, Vec2 b, Color color, float thickness = 1.f, float z = 0.f);
        void strokePolyline(Vec2 const* pts, size_t count, Color color, float thickness = 1.f, bool closed = false, float z = 0.f);

        void setLayer(uint8_t layer) noexcept { m_layer = layer; }
        void setZ(float z) noexcept { m_z = z; }
        uint8_t getLayer() const noexcept { return m_layer; }
        float getZ() const noexcept { return m_z; }

        void drawImage(TextureHandle texture, Rect const& dest, Color tint = Color::white(), float z = 0.f);
        void drawImageUV(TextureHandle texture, Rect const& dest, Rect const& uv, Color tint = Color::white(), float z = 0.f);

        void drawImageUVMode(TextureHandle texture, Rect const& dest, Rect const& uv, Color color, DrawMode mode, float z);

        void drawGlyphs(TextureHandle atlas, GlyphQuad const* quads, size_t count, float z = 0.f);

        void appendTransformed(DrawList const& src, Mat3 const& transform);

        struct Reservation {
            Vertex2D* vertices;
            uint32_t* indices;
            uint32_t vertexBase;
        };
        Reservation reserve(
            uint32_t vertCount, uint32_t indexCount,
            TextureHandle texture = TextureHandle::invalid(),
            BlendMode blendMode = BlendMode::Normal,
            float z = 0.f
        );

        std::vector<Vertex2D> const& getVertices() const noexcept { return m_vertices; }
        std::vector<uint32_t> const& getIndices() const noexcept { return m_indices; }
        std::vector<DrawCmd> const& getCommands() const noexcept { return m_commands; }

        void clear() noexcept;
        void reserveCapacity(size_t verts, size_t indices, size_t cmds);

    private:
        std::vector<Vertex2D> m_vertices;
        std::vector<uint32_t> m_indices;
        std::vector<DrawCmd> m_commands;

        std::vector<Mat3> m_transformStack;
        std::vector<Rect> m_clipStack;

        Mat3 m_currentTransform = Mat3::identity();
        Rect m_currentClip = Rect::maxRect();
        float m_z = 0.f;
        uint8_t m_layer = Layer::Content;

        uint32_t pushVert(
            Vec2 pos, Vec2 uv, Color color, DrawMode mode,
            float ex = 0.f, float ey = 0.f, float ez = 0.f,
            float c0 = 0.f, float c1 = 0.f, float c2 = 0.f, float c3 = 0.f
        ) noexcept;

        void pushIdx(uint32_t i) noexcept;
        void pushQuad(uint32_t base) noexcept;
        void pushTriangle(uint32_t a, uint32_t b, uint32_t c) noexcept;

        void touchCmd(
            TextureHandle texture,
            BlendMode blendMode,
            float z, uint8_t layer,
            uint32_t index,
            DrawMode mode
        );
    };
}

#endif // HELIOS_RENDERER_DRAWLIST_HPP
