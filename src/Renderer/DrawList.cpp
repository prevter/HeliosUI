#include <Helios/Renderer/DrawList.hpp>

namespace Helios {
    DrawList::DrawList() noexcept {
        m_transformStack.emplace_back(Mat3::identity());
        m_clipStack.emplace_back(Rect::maxRect());
    }

    void DrawList::pushClipRect(Rect const& r, bool intersect) {
        m_currentClip = intersect ? m_currentClip.intersection(r) : r;
        m_clipStack.push_back(m_currentClip);
    }

    void DrawList::popClipRect() {
        // TODO: HELIOS_ASSERT(!m_clipStack.empty(), "Clip stack underflow");
        m_clipStack.pop_back();
        m_currentClip = m_clipStack.back();
    }

    void DrawList::fillRect(Rect const& r, Color color, float z) {
        uint32_t base = this->pushVert({r.x, r.y}, {0, 0}, color, DrawMode::Solid);
        this->pushVert({r.x + r.w, r.y}, {1, 0}, color, DrawMode::Solid);
        this->pushVert({r.x + r.w, r.y + r.h}, {1, 1}, color, DrawMode::Solid);
        this->pushVert({r.x, r.y + r.h}, {0, 1}, color, DrawMode::Solid);

        this->pushQuad(base);
        this->touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::Solid);
    }

    void DrawList::fillRectGradient(
        Rect const& rect,
        Color topLeft, Color topRight,
        Color bottomLeft, Color bottomRight,
        float z
    ) {
        uint32_t base = this->pushVert({rect.x, rect.y}, {0, 0}, topLeft, DrawMode::Solid);
        this->pushVert({rect.x + rect.w, rect.y}, {1, 0}, topRight, DrawMode::Solid);
        this->pushVert({rect.x + rect.w, rect.y + rect.h}, {1, 1}, bottomRight, DrawMode::Solid);
        this->pushVert({rect.x, rect.y + rect.h}, {0, 1}, bottomLeft, DrawMode::Solid);

        this->pushQuad(base);
        this->touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::Solid);
    }

    void DrawList::fillRoundedRect(Rect const& r, float radius, Color color, float z) {
        this->fillRoundedRect(r, CornerRadii::all(radius), color, z);
    }

    void DrawList::fillRoundedRect(Rect const& r, CornerRadii radii, Color color, float z) {
        CornerRadii cr = radii.clamped(r.w, r.h);
        if (cr.tl == 0 && cr.tr == 0 && cr.br == 0 && cr.bl == 0) {
            fillRect(r, color, z);
            return;
        }

        float hw = r.w * 0.5f, hh = r.h * 0.5f;

        uint32_t base = pushVert(
            {r.x, r.y},
            {-hw, -hh},
            color, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x + r.w, r.y},
            {hw, -hh},
            color, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x + r.w, r.y + r.h},
            {hw, hh},
            color, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x, r.y + r.h},
            {-hw, hh},
            color, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::RoundedRect);
    }

    void DrawList::fillRoundedRectGradient(
        Rect const& r, float radius,
        Color topLeft, Color topRight,
        Color bottomLeft, Color bottomRight,
        float z
    ) {
        fillRoundedRectGradient(r, CornerRadii::all(radius), topLeft, topRight, bottomLeft, bottomRight, z);
    }

    void DrawList::fillRoundedRectGradient(
        Rect const& r,
        CornerRadii radii,
        Color topLeft, Color topRight,
        Color bottomLeft, Color bottomRight,
        float z
    ) {
        CornerRadii cr = radii.clamped(r.w, r.h);
        if (cr.tl == 0 && cr.tr == 0 && cr.br == 0 && cr.bl == 0) {
            fillRectGradient(r, topLeft, topRight, bottomLeft, bottomRight, z);
            return;
        }

        float hw = r.w * 0.5f, hh = r.h * 0.5f;

        uint32_t base = pushVert(
            {r.x, r.y},
            {-hw, -hh},
            topLeft, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x + r.w, r.y},
            {hw, -hh},
            topRight, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x + r.w, r.y + r.h},
            {hw, hh},
            bottomRight, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x, r.y + r.h},
            {-hw, hh},
            bottomLeft, DrawMode::RoundedRect,
            hw, hh, 0,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::RoundedRect);
    }

    void DrawList::fillCircle(Vec2 center, float radius, Color color, float z) {
        uint32_t base = pushVert(
            {center.x - radius, center.y - radius},
            {-radius, -radius},
            color, DrawMode::Circle, radius
        );
        pushVert(
            {center.x + radius, center.y - radius},
            {radius, -radius},
            color, DrawMode::Circle, radius
        );
        pushVert(
            {center.x + radius, center.y + radius},
            {radius, radius},
            color, DrawMode::Circle, radius
        );
        pushVert(
            {center.x - radius, center.y + radius},
            {-radius, radius},
            color, DrawMode::Circle, radius
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::Circle);
    }

    void DrawList::fillEllipse(Vec2 center, float radiusX, float radiusY, Color color, float z) {
        uint32_t base = pushVert(
            {center.x - radiusX, center.y - radiusY},
            {-radiusX, -radiusY},
            color, DrawMode::Circle, radiusX, radiusY
        );
        pushVert(
            {center.x + radiusX, center.y - radiusY},
            {radiusX, -radiusY},
            color, DrawMode::Circle, radiusX, radiusY
        );
        pushVert(
            {center.x + radiusX, center.y + radiusY},
            {radiusX, radiusY},
            color, DrawMode::Circle, radiusX, radiusY
        );
        pushVert(
            {center.x - radiusX, center.y + radiusY},
            {-radiusX, radiusY},
            color, DrawMode::Circle, radiusX, radiusY
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::Circle);
    }

    void DrawList::fillCircleGradient(Vec2 center, float radius, Color innerColor, Color outerColor, float z) {
        float oc0 = outerColor.r / 255.f;
        float oc1 = outerColor.g / 255.f;
        float oc2 = outerColor.b / 255.f;
        float oc3 = outerColor.a / 255.f;

        uint32_t base = pushVert(
            {center.x - radius, center.y - radius},
            {-radius, -radius},
            innerColor, DrawMode::RadialGradient,
            radius, 0, 0,
            oc0, oc1, oc2, oc3
        );
        pushVert(
            {center.x + radius, center.y - radius},
            {radius, -radius},
            innerColor, DrawMode::RadialGradient,
            radius, 0, 0,
            oc0, oc1, oc2, oc3
        );
        pushVert(
            {center.x + radius, center.y + radius},
            {radius, radius},
            innerColor, DrawMode::RadialGradient,
            radius, 0, 0,
            oc0, oc1, oc2, oc3
        );
        pushVert(
            {center.x - radius, center.y + radius},
            {-radius, radius},
            innerColor, DrawMode::RadialGradient,
            radius, 0, 0,
            oc0, oc1, oc2, oc3
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::RadialGradient);
    }

    void DrawList::fillConvexPoly(Vec2 const* pts, size_t count, Color color, float z) {
        if (count < 3) return;

        uint32_t base = pushVert(pts[0], {0, 0}, color, DrawMode::Solid);
        for (size_t i = 1; i < count; ++i) {
            pushVert(pts[i], {0, 0}, color, DrawMode::Solid);
        }

        uint32_t idxCount = static_cast<uint32_t>((count - 2) * 3);
        for (size_t i = 1; i < count - 1; ++i) {
            pushTriangle(base, base + i, base + i + 1);
        }

        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, idxCount, DrawMode::Solid);
    }

    void DrawList::fillTriangle(Vec2 a, Vec2 b, Vec2 c, Color color, float z) {
        uint32_t base = pushVert(a, {0, 0}, color, DrawMode::Solid);
        pushVert(b, {1, 0}, color, DrawMode::Solid);
        pushVert(c, {0, 1}, color, DrawMode::Solid);
        pushTriangle(base, base + 1, base + 2);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 3, DrawMode::Solid);
    }

    void DrawList::strokeRect(Rect const& r, Color color, float thickness, float z) {
        strokeLine(r.topLeft(), r.topRight(), color, thickness, z);
        strokeLine(r.topRight(), r.bottomRight(), color, thickness, z);
        strokeLine(r.bottomRight(), r.bottomLeft(), color, thickness, z);
        strokeLine(r.bottomLeft(), r.topLeft(), color, thickness, z);
    }

    void DrawList::strokeRoundedRect(Rect const& r, float radius, Color color, float thickness, float z) {
        this->strokeRoundedRect(r, CornerRadii::all(radius), color, thickness, z);
    }

    void DrawList::strokeRoundedRect(Rect const& r, CornerRadii radii, Color color, float thickness, float z) {
        CornerRadii cr = radii.clamped(r.w, r.h);
        float hw = r.w * 0.5f, hh = r.h * 0.5f;

        uint32_t base = pushVert(
            {r.x, r.y},
            {-hw, -hh},
            color, DrawMode::RoundedBorder,
            hw, hh, thickness,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x + r.w, r.y},
            {hw, -hh},
            color, DrawMode::RoundedBorder,
            hw, hh, thickness,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x + r.w, r.y + r.h},
            {hw, hh},
            color, DrawMode::RoundedBorder,
            hw, hh, thickness,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {r.x, r.y + r.h},
            {-hw, hh},
            color, DrawMode::RoundedBorder,
            hw, hh, thickness,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::RoundedBorder);
    }

    void DrawList::strokeCircle(Vec2 center, float radius, Color color, float thickness, float z) {
        float innerR = std::max(0.0f, radius - thickness);
        uint32_t base = pushVert(
            {center.x - radius, center.y - radius},
            {-radius, -radius},
            color, DrawMode::CircleBorder, radius, innerR, thickness
        );
        pushVert(
            {center.x + radius, center.y - radius},
            {radius, -radius},
            color, DrawMode::CircleBorder, radius, innerR, thickness
        );
        pushVert(
            {center.x + radius, center.y + radius},
            {radius, radius},
            color, DrawMode::CircleBorder, radius, innerR, thickness
        );
        pushVert(
            {center.x - radius, center.y + radius},
            {-radius, radius},
            color, DrawMode::CircleBorder, radius, innerR, thickness
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::CircleBorder);
    }

    void DrawList::strokeLine(Vec2 a, Vec2 b, Color color, float thickness, float z) {
        Vec2 dir = (b - a).normalized();
        Vec2 perp = dir.perpendicular() * (thickness * 0.5f);
        Vec2 ext = dir * (thickness * 0.5f);
        Vec2 a0 = a - ext, b0 = b + ext;

        uint32_t base = pushVert(a0 + perp, {0, 0}, color, DrawMode::Solid);
        pushVert(b0 + perp, {1, 0}, color, DrawMode::Solid);
        pushVert(b0 - perp, {1, 1}, color, DrawMode::Solid);
        pushVert(a0 - perp, {0, 1}, color, DrawMode::Solid);
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::Solid);
    }

    void DrawList::strokePolyline(Vec2 const* pts, size_t count, Color color, float thickness, bool closed, float z) {
        if (count < 2) return;

        for (size_t i = 0; i < count - 1; ++i) {
            strokeLine(pts[i], pts[i + 1], color, thickness, z);
        }
        if (closed) {
            strokeLine(pts[count - 1], pts[0], color, thickness, z);
        }
    }

    void DrawList::drawImage(TextureHandle texture, Rect const& dest, Color tint, float z) {
        this->drawImageUV(texture, dest, {0, 0, 1, 1}, tint, z);
    }

    void DrawList::drawImageUV(TextureHandle texture, Rect const& dest, Rect const& uv, Color tint, float z) {
        this->drawImageUVMode(texture, dest, uv, tint, DrawMode::Textured, z);
    }

    void DrawList::drawImageUVMode(
        TextureHandle texture,
        Rect const& dest,
        Rect const& uv,
        Color color,
        DrawMode mode,
        float z
    ) {
        uint32_t base = this->pushVert(dest.topLeft(), uv.topLeft(), color, mode);
        this->pushVert(dest.topRight(), uv.topRight(), color, mode);
        this->pushVert(dest.bottomRight(), uv.bottomRight(), color, mode);
        this->pushVert(dest.bottomLeft(), uv.bottomLeft(), color, mode);
        this->pushQuad(base);
        this->touchCmd(texture, BlendMode::Normal, z, m_layer, 6, mode);
    }

    void DrawList::drawGlyphs(TextureHandle atlas, GlyphQuad const* quads, size_t count, float z) {
        if (count == 0) return;

        uint32_t vertBase = static_cast<uint32_t>(m_vertices.size());
        uint32_t idxBase = static_cast<uint32_t>(m_indices.size());
        uint32_t totalVert = static_cast<uint32_t>(count) * 4;
        uint32_t totalIdx = static_cast<uint32_t>(count) * 6;

        m_vertices.resize(vertBase + totalVert);
        m_indices.resize(idxBase + totalIdx);

        Vertex2D* vp = m_vertices.data() + vertBase;
        uint32_t* ip = m_indices.data() + idxBase;

        for (size_t i = 0; i < count; ++i) {
            GlyphQuad const& g = quads[i];
            uint32_t base = vertBase + static_cast<uint32_t>(i) * 4;

            float x0 = g.dest.x, y0 = g.dest.y;
            float x1 = g.dest.x + g.dest.w, y1 = g.dest.y + g.dest.h;
            float u0 = g.uv.x, v0 = g.uv.y;
            float u1 = g.uv.x + g.uv.w, v1 = g.uv.y + g.uv.h;

            Vec2 tl = m_currentTransform.transformPoint({x0, y0});
            Vec2 tr = m_currentTransform.transformPoint({x1, y0});
            Vec2 br = m_currentTransform.transformPoint({x1, y1});
            Vec2 bl = m_currentTransform.transformPoint({x0, y1});

            auto setV = [&](Vertex2D& v, Vec2 p, float u, float fv) {
                v.pos = p;
                v.uv = {u, fv};
                v.color = g.color;
                v.mode = static_cast<float>(static_cast<int>(DrawMode::TextMTSDF));
                v.ex = v.ey = v.ez = 0.0f;
            };

            setV(vp[i * 4 + 0], tl, u0, v0);
            setV(vp[i * 4 + 1], tr, u1, v0);
            setV(vp[i * 4 + 2], br, u1, v1);
            setV(vp[i * 4 + 3], bl, u0, v1);

            ip[i * 6 + 0] = base;
            ip[i * 6 + 1] = base + 1;
            ip[i * 6 + 2] = base + 2;
            ip[i * 6 + 3] = base;
            ip[i * 6 + 4] = base + 2;
            ip[i * 6 + 5] = base + 3;
        }

        touchCmd(atlas, BlendMode::Normal, z, m_layer, totalIdx, DrawMode::TextMTSDF);
    }

    void DrawList::appendTransformed(DrawList const& src, Mat3 const& transform) {
        if (src.m_vertices.empty()) return;

        uint32_t vertBase = static_cast<uint32_t>(m_vertices.size());
        uint32_t idxBase = static_cast<uint32_t>(m_indices.size());

        m_vertices.reserve(vertBase + src.m_vertices.size());
        for (Vertex2D const& sv : src.m_vertices) {
            Vec2 tp = transform.transformPoint(sv.pos);
            Vertex2D& dv = m_vertices.emplace_back(sv);
            dv.pos = tp;
        }

        m_indices.reserve(idxBase + src.m_indices.size());
        for (uint32_t si : src.m_indices)
            m_indices.push_back(si + vertBase);

        m_commands.reserve(m_commands.size() + src.m_commands.size());
        for (DrawCmd const& sc : src.m_commands) {
            Rect clip = sc.clipRect;
            bool isMax = clip.w >= 1e17f;
            if (!isMax) {
                clip = transform.transformRect(clip);
            }

            if (!isMax) clip = m_currentClip.intersection(clip);
            else clip = m_currentClip;

            DrawCmd dc = sc;
            dc.indexOffset += idxBase;
            dc.clipRect = clip;

            if (!m_commands.empty()) {
                DrawCmd& prev = m_commands.back();
                if (prev.canMerge(dc)) {
                    prev.indexCount += sc.indexCount;
                    if (dc.usesTexture && !prev.usesTexture) {
                        prev.texture = dc.texture;
                        prev.usesTexture = true;
                    }
                    continue;
                }
            }

            m_commands.push_back(dc);
        }
    }

    void DrawList::clear() noexcept {
        m_vertices.clear();
        m_indices.clear();
        m_commands.clear();

        m_transformStack.clear();
        m_transformStack.emplace_back(Mat3::identity());

        m_clipStack.clear();
        m_clipStack.emplace_back(Rect::maxRect());

        m_currentTransform = Mat3::identity();
        m_currentClip = Rect::maxRect();
    }

    void DrawList::reserveCapacity(size_t verts, size_t indices, size_t cmds) {
        m_vertices.reserve(verts);
        m_indices.reserve(indices);
        m_commands.reserve(cmds);
    }

    uint32_t DrawList::pushVert(
        Vec2 pos, Vec2 uv, Color color, DrawMode mode,
        float ex, float ey, float ez,
        float c0, float c1, float c2, float c3
    ) noexcept {
        uint32_t idx = static_cast<uint32_t>(m_vertices.size());
        m_vertices.emplace_back(
            m_currentTransform.transformPoint(pos),
            uv,
            color,
            static_cast<float>(static_cast<int>(mode)),
            ex, ey, ez,
            c0, c1, c2, c3
        );
        return idx;
    }

    void DrawList::pushIdx(uint32_t i) noexcept {
        m_indices.push_back(i);
    }

    void DrawList::pushQuad(uint32_t base) noexcept {
        this->pushIdx(base);
        this->pushIdx(base + 1);
        this->pushIdx(base + 2);

        this->pushIdx(base);
        this->pushIdx(base + 2);
        this->pushIdx(base + 3);
    }

    void DrawList::pushTriangle(uint32_t a, uint32_t b, uint32_t c) noexcept {
        this->pushIdx(a);
        this->pushIdx(b);
        this->pushIdx(c);
    }

    void DrawList::touchCmd(
        TextureHandle texture, BlendMode blendMode, float z, uint8_t layer, uint32_t index, DrawMode mode
    ) {
        bool usesTexture = mode == DrawMode::Textured
                           || mode == DrawMode::TextSDF
                           || mode == DrawMode::TextMSDF
                           || mode == DrawMode::TextMTSDF;

        DrawCmd cmd{
            .indexOffset = static_cast<uint32_t>(m_indices.size()) - index,
            .indexCount = index,
            .texture = texture,
            .clipRect = m_currentClip,
            .z = z,
            .blendMode = blendMode,
            .layer = layer,
            .usesTexture = usesTexture
        };

        if (!m_commands.empty()) {
            auto& last = m_commands.back();
            if (last.z == z && last.canMerge(cmd)) {
                last.indexCount += index;
                // promote to textured if needed
                if (usesTexture && !last.usesTexture) {
                    last.texture = texture;
                    last.usesTexture = true;
                }
                return;
            }
        }

        m_commands.push_back(cmd);
    }

    void DrawList::fillArcSegment(
        Vec2 center,
        float radius,
        float startAngleRad,
        float endAngleRad,
        float inset,
        float lineWidth,
        Color color,
        float z
    ) {
        float const clampedInset = std::max(0.0f, inset);
        float const outerRadius = std::max(0.0f, radius - clampedInset);
        if (outerRadius <= 0.0f) return;

        float innerRadius = std::max(0.0f, outerRadius - std::max(0.0f, lineWidth));
        if (innerRadius >= outerRadius) innerRadius = 0.0f;

        uint32_t base = pushVert(
            {center.x - outerRadius, center.y - outerRadius},
            {-outerRadius, -outerRadius},
            color,
            DrawMode::ArcSegment,
            outerRadius,
            innerRadius,
            0.0f,
            startAngleRad,
            endAngleRad,
            0.0f,
            0.0f
        );
        pushVert(
            {center.x + outerRadius, center.y - outerRadius},
            {outerRadius, -outerRadius},
            color,
            DrawMode::ArcSegment,
            outerRadius,
            innerRadius,
            0.0f,
            startAngleRad,
            endAngleRad,
            0.0f,
            0.0f
        );
        pushVert(
            {center.x + outerRadius, center.y + outerRadius},
            {outerRadius, outerRadius},
            color,
            DrawMode::ArcSegment,
            outerRadius,
            innerRadius,
            0.0f,
            startAngleRad,
            endAngleRad,
            0.0f,
            0.0f
        );
        pushVert(
            {center.x - outerRadius, center.y + outerRadius},
            {-outerRadius, outerRadius},
            color,
            DrawMode::ArcSegment,
            outerRadius,
            innerRadius,
            0.0f,
            startAngleRad,
            endAngleRad,
            0.0f,
            0.0f
        );

        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::ArcSegment);
    }

    void DrawList::fillRectGlow(Rect const& rect, float blur, Color color, float z) {
        this->fillRoundedRectGlow(rect, 0.0f, blur, color, z);
    }

    void DrawList::fillRoundedRectGlow(Rect const& rect, float radius, float blur, Color color, float z) {
        this->fillRoundedRectGlow(rect, CornerRadii::all(radius), blur, color, z);
    }

    void DrawList::fillRoundedRectGlow(Rect const& rect, CornerRadii radii, float blur, Color color, float z) {
        CornerRadii cr = radii.clamped(rect.w, rect.h);
        float hw = rect.w * 0.5f, hh = rect.h * 0.5f;
        float cx = rect.x + hw, cy = rect.y + hh;
        float glowPad = std::max(blur * 4.0f, 1.0f);
        float ew = hw + glowPad, eh = hh + glowPad;

        uint32_t base = pushVert(
            {cx - ew, cy - eh},
            {-ew, -eh},
            color, DrawMode::RectGlow,
            hw, hh, blur,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {cx + ew, cy - eh},
            {ew, -eh},
            color, DrawMode::RectGlow,
            hw, hh, blur,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {cx + ew, cy + eh},
            {ew, eh},
            color, DrawMode::RectGlow,
            hw, hh, blur,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushVert(
            {cx - ew, cy + eh},
            {-ew, eh},
            color, DrawMode::RectGlow,
            hw, hh, blur,
            cr.tl, cr.tr, cr.br, cr.bl
        );
        pushQuad(base);
        touchCmd(TextureHandle::invalid(), BlendMode::Normal, z, m_layer, 6, DrawMode::RectGlow);
    }
}
