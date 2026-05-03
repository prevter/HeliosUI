#include <Helios/Director.hpp>
#include <Helios/Widgets/Label.hpp>

namespace Helios {
    static std::string normalizeNewlines(std::string const& text) {
        if (text.find('\r') == std::string::npos)
            return text;

        std::string normalized;
        normalized.reserve(text.size());

        for (size_t i = 0; i < text.size(); ++i) {
            char c = text[i];
            if (c == '\r') {
                if (i + 1 < text.size() && text[i + 1] == '\n')
                    continue;
                normalized.push_back('\n');
            } else {
                normalized.push_back(c);
            }
        }

        return normalized;
    }

    static Vec2 measureParagraph(tint::Font const& font, tint::ShapedParagraph const& paragraph) {
        float width = 0.f;
        for (auto const& line : paragraph.lines)
            width = std::max(width, line.width);

        float height = 0.f;
        if (!paragraph.lines.empty())
            height = paragraph.lines.back().baselineY - font.descender();

        return {width, height};
    }

    void Label::setText(std::string text) {
        if (m_text == text) return;
        m_text = std::move(text);
        invalidate();
    }

    void Label::setFont(FontHandle font) {
        if (m_font == font) return;
        m_font = font;
        invalidate();
    }

    void Label::setColor(Color color) {
        if (m_color == color) return;
        m_color = color;
        invalidateQuads();
    }

    void Label::setZ(float z) {
        if (m_z == z) return;
        m_z = z;
        invalidateQuads();
    }

    void Label::setAutoScale(bool enable) {
        if (m_autoScale == enable) return;
        m_autoScale = enable;
        invalidateQuads();
    }

    void Label::onDraw(DrawList& dl) {
        if (m_quadsDirty)
            buildQuads();

        if (m_pages.empty() || !m_font.isValid()) return;

        auto& fm = Director::get().fontManager();
        auto const& textures = fm.getTextures(m_font);

        for (size_t i = 0; i < m_pages.size(); ++i) {
            auto const& batch = m_pages[i];
            if (batch.quads.empty()) continue;
            if (i >= textures.size() || !textures[i].isValid()) continue;

            dl.setLayer(Layer::Text + i);

            dl.drawGlyphs(
                textures[i],
                batch.quads.data(),
                static_cast<int>(batch.quads.size()),
                m_z
            );
        }
    }

    void Label::onLayout() {
        if (!m_autoScale || m_size.x <= 0.f)
            return;

        if (m_naturalSizeDirty || m_naturalSize.x <= 0.f)
            return;

        float newScale = m_size.x / m_naturalSize.x;
        if (std::abs(newScale - this->getScale().x) > 0.0001f) {
            Widget::setScale(newScale);
            m_quadsDirty = true;
            markDirty();
        }
    }

    Vec2 Label::measure() {
        if (!m_naturalSizeDirty)
            return m_naturalSize;

        if (m_text.empty() || !m_font.isValid())
            return m_size;

        auto& fm = Director::get().fontManager();
        auto* font = fm.getFont(m_font);
        auto* shaper = fm.getShaper();
        if (!font || !shaper)
            return m_size;

        auto text = normalizeNewlines(m_text);
        auto paragraph = shaper->shapeMultiline(*font, text);
        if (paragraph.lines.empty())
            return m_size;

        m_naturalSize = measureParagraph(*font, paragraph);
        m_naturalSizeDirty = false;
        return m_naturalSize;
    }

    void Label::buildQuads() {
        m_pages.clear();
        m_textSize = {};
        m_quadsDirty = true;

        if (m_text.empty() || !m_font.isValid()) return;

        auto& fm = Director::get().fontManager();
        auto* font = fm.getFont(m_font);
        auto* shaper = fm.getShaper();
        auto* atlas = fm.getAtlasGroup(m_font);
        if (!font || !shaper || !atlas) return;

        auto text = normalizeNewlines(m_text);
        auto paragraph = shaper->shapeMultiline(*font, text);
        if (paragraph.lines.empty()) return;

        auto quads = atlas->buildQuads(*font, paragraph, 0.f, 0.f, tint::TextAlign::Left);

        for (auto& quad : quads) {
            if (quad.pageIndex >= m_pages.size())
                m_pages.resize(quad.pageIndex + 1);

            m_pages[quad.pageIndex].quads.push_back({
                .dest = {quad.x0, quad.y0, quad.x1 - quad.x0, quad.y1 - quad.y0},
                .uv = {quad.u0, quad.v0, quad.u1 - quad.u0, quad.v1 - quad.v0},
                .color = m_color
            });
        }

        m_textSize = measureParagraph(*font, paragraph);
        m_quadsDirty = false;

        if (m_autoScale && m_textSize.x > 0.f) {
            float newScale = m_size.x / m_textSize.x;
            this->setScale(newScale);
        } else if (!m_autoScale) {
            this->setSize(m_textSize);
        }
    }
}
