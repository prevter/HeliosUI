#pragma once
#ifndef HELIOS_FONTMANAGER_HPP
#define HELIOS_FONTMANAGER_HPP

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <Tint/Tint.hpp>
#include "../Handles.hpp"

namespace Helios {
    class FontManager {
    public:
        FontManager();
        ~FontManager();

        static FontManager& get() noexcept;

        FontManager(FontManager const&) = delete;
        FontManager& operator=(FontManager const&) = delete;
        FontManager(FontManager&&) = delete;
        FontManager& operator=(FontManager&&) = delete;

        [[nodiscard]] FontHandle loadFontFromFile(std::filesystem::path const& path, float sizePx = 48.f, uint32_t dpi = 72);
        [[nodiscard]] FontHandle loadFontFromMemory(void const* data, size_t size, float sizePx = 48.f, uint32_t dpi = 72);
        [[nodiscard]] FontHandle loadTntFile(std::filesystem::path const& path);

        [[nodiscard]] tint::Font* getFont(FontHandle h) const noexcept;
        [[nodiscard]] tint::Shaper* getShaper() noexcept { return &m_shaper; }
        [[nodiscard]] tint::AtlasGroup* getAtlasGroup(FontHandle h) noexcept;
        [[nodiscard]] std::vector<TextureHandle> const& getTextures(FontHandle h);
        [[nodiscard]] bool hasFont(FontHandle h) const noexcept;
        [[nodiscard]] FontHandle getDefaultFont() const noexcept { return m_defaultFont; }
        void updateAtlasTextures(FontHandle h);

        void shutdown();

        struct FontData {
            std::unique_ptr<tint::Font> font;
            std::unique_ptr<tint::AtlasGroup> atlasGroup;
            std::vector<TextureHandle> textures;
            std::vector<uint8_t> dirtyPages;
            size_t lastPageCount = 0;
        };

        std::unordered_map<FontHandle, FontData> const& getAllFonts() const noexcept { return m_fonts; }

    private:
        static void markAtlasPageDirty(void* userData, size_t pageIndex) noexcept;

        tint::FontLibrary m_library;
        tint::Shaper m_shaper;
        std::unordered_map<FontHandle, FontData> m_fonts;
        FontHandle m_nextFontHandle{1};
        FontHandle m_defaultFont{0};
    };
}

#endif // HELIOS_FONTMANAGER_HPP