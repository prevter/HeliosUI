#include <ranges>
#include <Helios/Director.hpp>
#include <Helios/Renderer/Renderer.hpp>
#include <Helios/Text/FontManager.hpp>
#include <Tint/TntFont.hpp>

#include <fmt/format.h>

namespace Helios {
    void FontManager::markAtlasPageDirty(void* userData, size_t pageIndex) noexcept {
        auto* data = static_cast<FontData*>(userData);
        if (!data) {
            return;
        }

        if (pageIndex >= data->dirtyPages.size()) {
            data->dirtyPages.resize(pageIndex + 1, 0);
        }

        data->dirtyPages[pageIndex] = 1;
    }

    FontManager::FontManager() {
        if (!m_library.isValid()) {
            throw std::runtime_error("Failed to initialize font library");
        }
    }

    FontManager::~FontManager() {
        for (auto& data : m_fonts | std::views::values) {
            for (auto tex : data.textures) {
                Director::get().renderer().destroyTexture(tex);
            }
        }
    }

    FontManager& FontManager::get() noexcept {
        return Director::get().fontManager();
    }

    FontHandle FontManager::loadFontFromFile(std::filesystem::path const& path, float sizePx, uint32_t dpi) {
        if (!m_library.isValid()) {
            return FontHandle::invalid();
        }

        tint::FontConfig config{
            .sizePx = sizePx,
            .dpi = dpi,
            .enableLigatures = true,
            .enableKerning = true
        };

        auto font = tint::Font::loadFile(m_library, path, config);
        if (!font) {
            return FontHandle::invalid();
        }

        auto atlasGroup = std::make_unique<tint::AtlasGroup>(
            1024, 1024,
            tint::PixelFormat::Gray8,
            tint::AtlasMode::SDF
        );

        FontHandle handle = m_nextFontHandle++;

        auto it = m_fonts.emplace(handle, FontData{
            .font = std::move(font),
            .atlasGroup = std::move(atlasGroup),
            .textures = {},
            .dirtyPages = {},
            .lastPageCount = 0
        }).first;

        if (it->second.atlasGroup) {
            it->second.atlasGroup->setPageDirtyCallback(&FontManager::markAtlasPageDirty, &it->second);
        }

        if (!m_defaultFont.isValid()) {
            m_defaultFont = handle;
        }

        return handle;
    }

    FontHandle FontManager::loadFontFromMemory(void const* data, size_t size, float sizePx, uint32_t dpi) {
        if (!m_library.isValid()) {
            return FontHandle::invalid();
        }

        tint::FontConfig config{
            .sizePx = sizePx,
            .dpi = dpi,
            .enableLigatures = true,
            .enableKerning = true
        };

        auto font = tint::Font::loadMemory(m_library, std::span(static_cast<uint8_t const*>(data), size), config);
        if (!font) {
            return FontHandle::invalid();
        }

        auto atlasGroup = std::make_unique<tint::AtlasGroup>(
            1024, 1024,
            tint::PixelFormat::Gray8,
            tint::AtlasMode::SDF
        );

        FontHandle handle = m_nextFontHandle++;

        auto it = m_fonts.emplace(handle, FontData{
            .font = std::move(font),
            .atlasGroup = std::move(atlasGroup),
            .textures = {},
            .dirtyPages = {},
            .lastPageCount = 0
        }).first;

        if (it->second.atlasGroup) {
            it->second.atlasGroup->setPageDirtyCallback(&FontManager::markAtlasPageDirty, &it->second);
        }

        if (!m_defaultFont.isValid()) {
            m_defaultFont = handle;
        }

        return handle;
    }

    FontHandle FontManager::loadTntFile(std::filesystem::path const& path) {
        auto [font, atlasGroup] = tint::loadTntFont(m_library, path);
        if (!font || !atlasGroup) {
            fmt::println(stderr, "loadTntFile failed: {}", path.string());
            return FontHandle::invalid();
        }

        FontHandle handle = m_nextFontHandle++;
        auto it = m_fonts.emplace(handle, FontData{
            std::move(font),
            std::move(atlasGroup),
            {}, {}, 0
        }).first;

        if (it->second.atlasGroup) {
            it->second.atlasGroup->setPageDirtyCallback(&FontManager::markAtlasPageDirty, &it->second);
        }

        if (!m_defaultFont.isValid()) {
            m_defaultFont = handle;
        }

        return handle;
    }

    tint::Font* FontManager::getFont(FontHandle h) const noexcept {
        auto it = m_fonts.find(h);
        if (it == m_fonts.end()) {
            return nullptr;
        }
        return it->second.font.get();
    }

    tint::AtlasGroup* FontManager::getAtlasGroup(FontHandle h) noexcept {
        auto it = m_fonts.find(h);
        if (it == m_fonts.end()) {
            return nullptr;
        }
        return it->second.atlasGroup.get();
    }

    std::vector<TextureHandle> const& FontManager::getTextures(FontHandle h) {
        static std::vector<TextureHandle> empty;

        auto it = m_fonts.find(h);
        if (it == m_fonts.end()) {
            return empty;
        }

        auto& data = it->second;
        if (data.atlasGroup) {
            size_t pageCount = data.atlasGroup->pageCount();
            bool needsUpdate = data.textures.size() != pageCount;

            if (!needsUpdate) {
                if (data.dirtyPages.size() < pageCount) {
                    needsUpdate = true;
                } else {
                    for (size_t i = 0; i < pageCount; ++i) {
                        if (data.dirtyPages[i]) {
                            needsUpdate = true;
                            break;
                        }
                    }
                }
            }

            if (needsUpdate) {
                this->updateAtlasTextures(h);
            }
        }

        return data.textures;
    }

    bool FontManager::hasFont(FontHandle h) const noexcept {
        return m_fonts.contains(h);
    }

    static TextureFormat tintFmtToHelios(tint::PixelFormat fmt) {
        switch (fmt) {
            case tint::PixelFormat::Gray8: return TextureFormat::R8;
            case tint::PixelFormat::RGBA8: return TextureFormat::RGBA8;
            case tint::PixelFormat::RGB8: return TextureFormat::RGB8;
            default: return TextureFormat::R8;
        }
    }

    void FontManager::updateAtlasTextures(FontHandle h) {
        auto it = m_fonts.find(h);
        if (it == m_fonts.end()) {
            return;
        }

        auto& data = it->second;
        if (!data.atlasGroup) {
            return;
        }

        auto& renderer = Director::get().renderer();
        size_t pageCount = data.atlasGroup->pageCount();

        if (data.textures.size() < pageCount) {
            data.textures.resize(pageCount, TextureHandle::invalid());
        }

        if (data.dirtyPages.size() < pageCount) {
            data.dirtyPages.resize(pageCount, 1);
        }

        for (size_t i = 0; i < pageCount; ++i) {
            auto const& atlas = data.atlasGroup->page(i);
            auto atlasData = atlas.pixels();
            uint32_t atlasWidth = atlas.width();
            uint32_t atlasHeight = atlas.height();

            if (atlasData.empty() || atlasWidth == 0 || atlasHeight == 0) {
                data.dirtyPages[i] = 0;
                continue;
            }

            auto fmt = tintFmtToHelios(atlas.format());
            bool uploaded = false;

            if (data.textures[i] == 0) {
                data.textures[i] = renderer.createTexture({
                    .data = atlasData.data(),
                    .width = atlasWidth,
                    .height = atlasHeight,
                    .format = fmt,
                    .filter = TextureFilter::Linear,
                    .generateMips = false
                });
                uploaded = data.textures[i].isValid();
            } else if (i < data.dirtyPages.size() && data.dirtyPages[i]) {
                renderer.updateTexture(
                    data.textures[i],
                    atlasData.data(),
                    0, 0,
                    atlasWidth,
                    atlasHeight,
                    fmt
                );
                uploaded = true;
            }

            if (uploaded && i < data.dirtyPages.size()) {
                data.dirtyPages[i] = 0;
            }
        }

        data.lastPageCount = pageCount;
    }

    void FontManager::shutdown() {
        for (auto& data : m_fonts | std::views::values) {
            for (auto tex : data.textures) {
                Director::get().renderer().destroyTexture(tex);
            }
        }
        m_fonts.clear();
    }
}
