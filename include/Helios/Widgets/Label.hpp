#pragma once
#ifndef HELIOS_WIDGET_LABEL_HPP
#define HELIOS_WIDGET_LABEL_HPP

#include "Widget.hpp"
#include "../Text/FontManager.hpp"

namespace Helios {
    class Label : public Widget {
    public:
        Label() = default;
        explicit Label(std::string text, FontHandle font = FontManager::get().getDefaultFont()) {
            this->setText(std::move(text));
            this->setFont(font);
        }

        void setText(std::string text);
        void setFont(FontHandle font);
        void setColor(Color color);
        void setZ(float z);
        void setAutoScale(bool enable);

        [[nodiscard]] std::string const& getText() const noexcept { return m_text; }
        [[nodiscard]] FontHandle getFont() const noexcept { return m_font; }
        [[nodiscard]] Color getColor() const noexcept { return m_color; }
        [[nodiscard]] float getZ() const noexcept { return m_z; }

        [[nodiscard]] Vec2 naturalSize() const noexcept { return m_naturalSize; }
        [[nodiscard]] Vec2 textSize() const noexcept { return m_textSize; }
        [[nodiscard]] bool autoScale() const noexcept { return m_autoScale; }

        void onDraw(DrawList& dl) override;
        void onLayout() override;
        Vec2 measure() override;

    private:
        void invalidate() {
            m_naturalSizeDirty = true;
            m_quadsDirty = true;
            markDirty();
        }

        void invalidateQuads() {
            m_quadsDirty = true;
            markDirty(DirtyFlags::Geometry);
        }

        void buildQuads();

    protected:
        struct PageBatch {
            std::vector<GlyphQuad> quads;
        };

        std::vector<PageBatch> m_pages;
        std::string m_text;
        FontHandle m_font = FontHandle::invalid();
        Color m_color = Color::white();
        float m_z = 0.f;

        Vec2 m_naturalSize;
        Vec2 m_textSize;

        bool m_naturalSizeDirty = true;
        bool m_quadsDirty = true;
        bool m_autoScale = false;
    };
}

#endif // HELIOS_WIDGET_LABEL_HPP