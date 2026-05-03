#include <Helios/Widgets/Box.hpp>

#include <algorithm>
#include <cmath>

namespace Helios {
    static float axisScale(float scale) {
        return std::max(std::abs(scale), 0.0001f);
    }

    void Box::onLayout() {
        float innerW = m_size.x - m_padding.horizontal();
        float innerH = m_size.y - m_padding.vertical();
        float padL = m_padding.left;
        float padT = m_padding.top;

        auto resolveAlign = [&](LayoutParams::Align self) -> Alignment {
            switch (self) {
                case LayoutParams::Align::Start: return Alignment::Start;
                case LayoutParams::Align::Center: return Alignment::Center;
                case LayoutParams::Align::End: return Alignment::End;
                case LayoutParams::Align::Stretch: return Alignment::Stretch;
                default: return m_alignment;
            }
        };

        for (auto& c : children()) {
            if (!c->isVisible()) continue;

            auto& params = c->layoutParams();
            Alignment align = resolveAlign(params.alignSelf);

            bool fillW = params.widthMode == LayoutParams::SizeMode::Fill;
            bool fillH = params.heightMode == LayoutParams::SizeMode::Fill;

            Vec2 m = c->measure();
            Vec2 s = c->getScale();
            float sx = axisScale(s.x);
            float sy = axisScale(s.y);

            float visualW = fillW || align == Alignment::Stretch ? innerW : m.x * sx;
            float visualH = fillH || align == Alignment::Stretch ? innerH : m.y * sy;

            float cx, cy;
            switch (align) {
                case Alignment::Start:
                case Alignment::Stretch:
                default:
                    cx = padL;
                    cy = padT;
                    break;
                case Alignment::Center:
                    cx = padL + (innerW - visualW) * 0.5f;
                    cy = padT + (innerH - visualH) * 0.5f;
                    break;
                case Alignment::End:
                    cx = padL + (innerW - visualW);
                    cy = padT + (innerH - visualH);
                    break;
            }

            c->setRect({cx, cy}, {visualW / sx, visualH / sy});
        }
    }

    Vec2 Box::measure() {
        float maxW = 0.f, maxH = 0.f;
        for (auto const& c : children()) {
            if (!c->isVisible()) continue;

            auto& params = c->layoutParams();
            bool fillW = params.widthMode == LayoutParams::SizeMode::Fill;
            bool fillH = params.heightMode == LayoutParams::SizeMode::Fill;

            Vec2 m = c->measure();
            Vec2 s = c->getScale();
            float sx = axisScale(s.x);
            float sy = axisScale(s.y);

            if (!fillW) maxW = std::max(maxW, m.x * sx);
            if (!fillH) maxH = std::max(maxH, m.y * sy);
        }
        Vec2 intrinsic = { maxW + m_padding.horizontal(), maxH + m_padding.vertical() };
        return {
            m_requestedSize.x > 0.f ? m_requestedSize.x : intrinsic.x,
            m_requestedSize.y > 0.f ? m_requestedSize.y : intrinsic.y,
        };
    }
}
