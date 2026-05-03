#pragma once
#ifndef HELIOS_WIDGET_BOX_HPP
#define HELIOS_WIDGET_BOX_HPP

#include "Layout.hpp"
#include "Widget.hpp"

namespace Helios {
    class Box : public Widget {
    public:
        Box() noexcept { m_isLayoutContainer = true; }

        void onLayout() override;
        Vec2 measure() override;

        void setPadding(EdgeInsets padding) { m_padding = padding; markDirty(DirtyFlags::Layout); }
        void setAlignment(Alignment alignment) { m_alignment = alignment; markDirty(DirtyFlags::Layout); }

        EdgeInsets const& padding() const noexcept { return m_padding; }
        Alignment alignment() const noexcept { return m_alignment; }

    protected:
        EdgeInsets m_padding;
        Alignment m_alignment = Alignment::Start;
    };
}

#endif // HELIOS_WIDGET_BOX_HPP