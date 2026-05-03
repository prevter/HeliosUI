#include <Helios/Widgets/LinearLayout.hpp>

#include <algorithm>
#include <cmath>

namespace Helios {
    static float& ax(Vec2& v, bool horizontal) { return horizontal ? v.x : v.y; }
    static float cax(Vec2 const& v, bool horizontal) { return horizontal ? v.x : v.y; }
    static float axisScale(float scale) { return std::max(std::abs(scale), 0.0001f); }

    static Alignment resolveAlign(LayoutParams::Align self, Alignment parent) {
        switch (self) {
            case LayoutParams::Align::Start: return Alignment::Start;
            case LayoutParams::Align::Center: return Alignment::Center;
            case LayoutParams::Align::End: return Alignment::End;
            case LayoutParams::Align::Stretch: return Alignment::Stretch;
            default: return parent;
        }
    }

    struct LinearConfig {
        EdgeInsets padding;
        float gap;
        Arrangement arrangement;
        Alignment crossAlign;
        bool horizontal;
    };

    static Vec2 linearMeasure(std::vector<WidgetPtr> const& children, LinearConfig const& cfg) {
        size_t visibleChildren = 0;
        float mainSize = 0.f;
        float crossSize = 0.f;

        for (auto const& c : children) {
            if (!c->isVisible()) continue;

            ++visibleChildren;
            auto& params = c->layoutParams();
            Vec2 m = c->measure();
            Vec2 s = c->getScale();
            float mainScale = axisScale(cax(s, cfg.horizontal));
            float crossScale = axisScale(cax(s, !cfg.horizontal));
            bool mainFill = cfg.horizontal
                              ? params.widthMode == LayoutParams::SizeMode::Fill
                              : params.heightMode == LayoutParams::SizeMode::Fill;
            bool crossFill = cfg.horizontal
                              ? params.heightMode == LayoutParams::SizeMode::Fill
                              : params.widthMode == LayoutParams::SizeMode::Fill;

            if (!mainFill) {
                mainSize += cax(m, cfg.horizontal) * mainScale;
            }

            if (!crossFill) {
                crossSize = std::max(cax(m, !cfg.horizontal) * crossScale, crossSize);
            }
        }

        if (visibleChildren > 1) {
            mainSize += cfg.gap * (visibleChildren - 1);
        }

        float padMain = cfg.horizontal ? cfg.padding.horizontal() : cfg.padding.vertical();
        float padCross = cfg.horizontal ? cfg.padding.vertical() : cfg.padding.horizontal();

        Vec2 result;
        ax(result, cfg.horizontal) = mainSize + padMain;
        ax(result, !cfg.horizontal) = crossSize + padCross;
        return result;
    }

    static void linearLayout(std::vector<WidgetPtr> const& children, Vec2 size, LinearConfig const& cfg) {
        std::vector<Widget*> visibleChildren;
        visibleChildren.reserve(children.size());
        for (auto const& child : children) {
            if (child->isVisible()) {
                visibleChildren.push_back(child.get());
            }
        }

        if (visibleChildren.empty()) return;

        float padMainStart = cfg.horizontal ? cfg.padding.left : cfg.padding.top;
        float padMainEnd = cfg.horizontal ? cfg.padding.right : cfg.padding.bottom;
        float padCrossStart = cfg.horizontal ? cfg.padding.top : cfg.padding.left;
        float padCrossEnd = cfg.horizontal ? cfg.padding.bottom : cfg.padding.right;

        float innerMain = cax(size, cfg.horizontal) - padMainStart - padMainEnd;
        float innerCross = cax(size, !cfg.horizontal) - padCrossStart - padCrossEnd;

        float totalFitMain = 0.f;
        float totalWeight = 0.f;
        auto n = visibleChildren.size();

        struct ChildInfo {
            float mainVisual = 0.f;
            float crossVisual = 0.f;
            float mainScale = 1.f;
            float crossScale = 1.f;
        };

        std::vector<ChildInfo> infos(n);

        bool isSpaceArrangement = (cfg.arrangement == Arrangement::SpaceBetween ||
                                   cfg.arrangement == Arrangement::SpaceAround ||
                                   cfg.arrangement == Arrangement::SpaceEvenly);

        for (size_t i = 0; i < n; ++i) {
            auto* c = visibleChildren[i];
            auto& params = c->layoutParams();
            Vec2 m = c->measure();
            Vec2 s = c->getScale();
            infos[i].mainScale = axisScale(cax(s, cfg.horizontal));
            infos[i].crossScale = axisScale(cax(s, !cfg.horizontal));

            bool isFill = cfg.horizontal
                              ? params.widthMode == LayoutParams::SizeMode::Fill
                              : params.heightMode == LayoutParams::SizeMode::Fill;

            infos[i].crossVisual = cax(m, !cfg.horizontal) * infos[i].crossScale;

            if (isFill) {
                totalWeight += params.weight;
            } else {
                infos[i].mainVisual = cax(m, cfg.horizontal) * infos[i].mainScale;
                totalFitMain += infos[i].mainVisual;
            }
        }

        float totalGap = cfg.gap * (n - 1);
        float gapForFill = isSpaceArrangement ? 0.f : totalGap;
        float fillSpace = std::max(0.f, innerMain - totalFitMain - gapForFill);

        for (size_t i = 0; i < n; ++i) {
            auto* c = visibleChildren[i];
            auto& params = c->layoutParams();
            bool isFill = cfg.horizontal
                              ? params.widthMode == LayoutParams::SizeMode::Fill
                              : params.heightMode == LayoutParams::SizeMode::Fill;

            if (isFill) {
                infos[i].mainVisual = totalWeight > 0.f ? fillSpace * (params.weight / totalWeight) : 0;
            }
        }

        float totalChildren = totalFitMain + (totalWeight > 0.f ? fillSpace : 0.f);
        float remaining;

        if (isSpaceArrangement) {
            remaining = innerMain - totalChildren;
        } else {
            remaining = innerMain - totalChildren - totalGap;
        }

        float offset = 0;
        float extraGap = 0;

        switch (cfg.arrangement) {
            case Arrangement::Start: break;
            case Arrangement::End: offset = remaining;
                break;
            case Arrangement::Center: offset = remaining * 0.5f;
                break;
            case Arrangement::SpaceBetween: extraGap = n > 1 ? remaining / (n - 1) : 0.f;
                break;
            case Arrangement::SpaceAround: extraGap = n > 0 ? remaining / n : 0.f;
                offset = extraGap * 0.5f;
                break;
            case Arrangement::SpaceEvenly: extraGap = remaining / (n + 1);
                offset = extraGap;
                break;
        }

        float stepGap = isSpaceArrangement ? 0.f : cfg.gap;
        float cursor = padMainStart + offset;

        for (size_t i = 0; i < n; ++i) {
            auto* c = visibleChildren[i];
            auto& info = infos[i];
            auto& params = c->layoutParams();

            Alignment align = resolveAlign(params.alignSelf, cfg.crossAlign);
            bool fillCross = cfg.horizontal
                                 ? params.heightMode == LayoutParams::SizeMode::Fill
                                 : params.widthMode == LayoutParams::SizeMode::Fill;

            float childCrossVisual = fillCross || align == Alignment::Stretch ? innerCross : info.crossVisual;

            float crossOffset = padCrossStart;
            switch (align) {
                case Alignment::Start: break;
                case Alignment::Stretch: break;
                case Alignment::Center: crossOffset += (innerCross - childCrossVisual) * 0.5f;
                    break;
                case Alignment::End: crossOffset += (innerCross - childCrossVisual);
                    break;
            }

            Vec2 childPos, childSize;
            ax(childPos, cfg.horizontal) = cursor;
            ax(childPos, !cfg.horizontal) = crossOffset;
            ax(childSize, cfg.horizontal) = info.mainVisual / info.mainScale;
            ax(childSize, !cfg.horizontal) = childCrossVisual / info.crossScale;

            c->setRect(childPos, childSize);

            cursor += info.mainVisual + stepGap + extraGap;
        }
    }

    static Vec2 blendMeasure(Vec2 explicitSize, Vec2 intrinsic) {
        return {
            explicitSize.x > 0.f ? explicitSize.x : intrinsic.x,
            explicitSize.y > 0.f ? explicitSize.y : intrinsic.y,
        };
    }

    void Row::onLayout() {
        linearLayout(children(), m_size, {padding, gap, horizontalArrangement, verticalAlignment, true});
    }

    Vec2 Row::measure() {
        Vec2 intrinsic = linearMeasure(children(), {padding, gap, horizontalArrangement, verticalAlignment, true});
        return blendMeasure(m_requestedSize, intrinsic);
    }

    void Column::onLayout() {
        linearLayout(children(), m_size, {padding, gap, verticalArrangement, horizontalAlignment, false});
    }

    Vec2 Column::measure() {
        Vec2 intrinsic = linearMeasure(children(), {padding, gap, verticalArrangement, horizontalAlignment, false});
        return blendMeasure(m_requestedSize, intrinsic);
    }
}
