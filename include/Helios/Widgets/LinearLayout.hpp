#pragma once
#ifndef HELIOS_WIDGET_ROW_HPP
#define HELIOS_WIDGET_ROW_HPP

#include "Layout.hpp"
#include "Widget.hpp"

namespace Helios {
    class Row : public Widget {
    public:
        Row() noexcept { m_isLayoutContainer = true; }

        void onLayout() override;
        Vec2 measure() override;

        void setPadding(EdgeInsets p) { padding = p; markDirty(DirtyFlags::Layout); }
        void setGap(float g) { gap = g; markDirty(DirtyFlags::Layout); }
        void setHorizontalArrangement(Arrangement a) { horizontalArrangement = a; markDirty(DirtyFlags::Layout); }
        void setVerticalAlignment(Alignment a) { verticalAlignment = a; markDirty(DirtyFlags::Layout); }

        EdgeInsets const& getPadding() const noexcept { return padding; }
        float getGap() const noexcept { return gap; }
        Arrangement getHorizontalArrangement() const noexcept { return horizontalArrangement; }
        Alignment getVerticalAlignment() const noexcept { return verticalAlignment; }

    protected:
        EdgeInsets padding;
        float gap = 0.f;
        Arrangement horizontalArrangement = Arrangement::Start;
        Alignment verticalAlignment = Alignment::Start;
    };

    class Column : public Widget {
    public:
        Column() noexcept { m_isLayoutContainer = true; }

        void onLayout() override;
        Vec2 measure() override;

        void setPadding(EdgeInsets p) { padding = p; markDirty(DirtyFlags::Layout); }
        void setGap(float g) { gap = g; markDirty(DirtyFlags::Layout); }
        void setVerticalArrangement(Arrangement a) { verticalArrangement = a; markDirty(DirtyFlags::Layout); }
        void setHorizontalAlignment(Alignment a) { horizontalAlignment = a; markDirty(DirtyFlags::Layout); }

        EdgeInsets const& getPadding() const noexcept { return padding; }
        float getGap() const noexcept { return gap; }
        Arrangement getVerticalArrangement() const noexcept { return verticalArrangement; }
        Alignment getHorizontalAlignment() const noexcept { return horizontalAlignment; }

    protected:
        EdgeInsets padding;
        float gap = 0.f;
        Arrangement verticalArrangement = Arrangement::Start;
        Alignment horizontalAlignment = Alignment::Start;
    };
}

#endif // HELIOS_WIDGET_ROW_HPP