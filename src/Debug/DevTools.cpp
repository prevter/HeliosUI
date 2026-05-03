#include <Helios/Debug/DevTools.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>

#include <fmt/format.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <Helios/Director.hpp>
#include <Helios/Text/FontManager.hpp>
#include <Helios/Widgets/Box.hpp>
#include <Helios/Widgets/Label.hpp>
#include <Helios/Widgets/LinearLayout.hpp>
#include <algorithm>
#include <Helios/Debug/Instrumentation.hpp>

#ifdef GEODE_IS_WINDOWS
#else
#include <cxxabi.h>
#include <typeindex>
#include <unordered_map>
#endif


namespace Helios {
    struct DevToolsState {
        static DevToolsState& get() {
            static DevToolsState state;
            return state;
        }

        DevToolsState() {
            glGenFramebuffers(1, &framebuffer);
            glGenTextures(1, &framebufferTexture);
        }

        ~DevToolsState() {
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteTextures(1, &framebufferTexture);
        }

        void initFramebuffer(int width, int height) {
            glBindTexture(GL_TEXTURE_2D, framebufferTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            framebufferW = width;
            framebufferH = height;
        }

        void resizeFramebuffer(int width, int height) {
            if (width == framebufferW && height == framebufferH) return;
            glBindTexture(GL_TEXTURE_2D, framebufferTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            framebufferW = width;
            framebufferH = height;
        }

        GLFWwindow* window = nullptr;
        int framebufferW = 800, framebufferH = 600;
        GLuint framebuffer = 0;
        GLuint framebufferTexture = 0;
        GLuint prevFramebuffer = 0;
        bool captureMainFramebuffer = false;

        Widget* selectedWidget = nullptr;
    };

    std::string_view getWidgetName(Widget const* w) {
    #ifdef _WIN32
        std::string_view tname = typeid(*w).name();
        if (tname.starts_with("class ")) {
            tname.remove_prefix(6);
        } else if (tname.starts_with("struct ")) {
            tname.remove_prefix(7);
        }

        return tname;
    #else
        static std::unordered_map<std::type_index, std::string> s_typeNames;
        std::type_index key = typeid(*w);

        auto it = s_typeNames.find(key);
        if (it != s_typeNames.end()) {
            return it->second;
        }

        std::string ret;

        int status = 0;
        auto demangle = abi::__cxa_demangle(typeid(*w).name(), nullptr, nullptr, &status);
        if (status == 0) {
            ret = demangle;
        }
        free(demangle);
        auto [iter, _] = s_typeNames.insert({key, std::move(ret)});

        return iter->second;
    #endif
    }

    void DevTools::init(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        auto& state = DevToolsState::get();
        state.window = window;

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        state.initFramebuffer(w, h);
    }

    void DevTools::shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void DevTools::prerender() {
        auto& state = DevToolsState::get();
        if (state.captureMainFramebuffer) {
            int w, h;
            glfwGetFramebufferSize(state.window, &w, &h);
            state.resizeFramebuffer(w, h);

            glGetIntegerv(GL_FRAMEBUFFER_BINDING, reinterpret_cast<GLint*>(&state.prevFramebuffer));
            glBindFramebuffer(GL_FRAMEBUFFER, state.framebuffer);
        }
    }

    static void RenderTreeBranch(Widget* w, size_t index, bool visible) {
        auto& state = DevToolsState::get();
        visible = w->isVisible() && visible;

        auto selected = state.selectedWidget == w;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (w->children().empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        auto alpha = ImGui::GetStyle().DisabledAlpha;
        ImGui::GetStyle().DisabledAlpha = visible ? 1.f : 0.5f;

        ImGui::BeginDisabled(!visible);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);

        auto name = getWidgetName(w);
        std::string_view text;
        if (auto label = dynamic_cast<Label*>(w)) {
            text = label->getText();
        }

        bool expanded;
        if (text.empty()) {
            expanded = ImGui::TreeNodeEx(w, flags, "%.*s", (int)name.size(), name.data());
        } else {
            expanded = ImGui::TreeNodeEx(w, flags, "%.*s \"%.*s\"", (int)name.size(), name.data(), (int)text.size(), text.data());
        }

        ImGui::GetStyle().DisabledAlpha = alpha;
        ImGui::PopItemFlag(); //ImGuiItemFlags_Disabled
        ImGui::EndDisabled();

        if (ImGui::IsItemClicked()) {
            state.selectedWidget = w;
        }

        if (expanded) {
            size_t i = 0;
            for (auto& child : w->children()) {
                RenderTreeBranch(child.get(), i++, visible);
            }
            ImGui::TreePop();
        }
    }

    static void ShowWidgetTree() {
        if (ImGui::Begin("Widget Tree")) {
            auto& widgets = Director::get().widgets();

            for (size_t i = 0; i < widgets.size(); ++i) {
                RenderTreeBranch(widgets[i].get(), i, true);
            }
        }
        ImGui::End();
    }

    static void ShowWidgetInspector() {
        auto& state = DevToolsState::get();
        if (!state.selectedWidget) return;

        if (ImGui::Begin("Inspector")) {
            auto w = state.selectedWidget;
            if (ImGui::Button("Deselect")) {
                state.selectedWidget = nullptr;
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy Class Name")) {
                ImGui::SetClipboardText(getWidgetName(w).data());
            }

            ImGui::Text("Address: %p", static_cast<void*>(w));
            ImGui::SameLine();

            if (ImGui::Button("Copy")) {
                ImGui::SetClipboardText(fmt::format("{:#x}", reinterpret_cast<uintptr_t>(w)).c_str());
            }

            Vec2 pos = w->position();
            if (ImGui::DragFloat2("Position", &pos.x)) {
                w->setPosition(pos);
            }

            Vec2 anchor = w->anchor();
            if (ImGui::DragFloat2("Anchor", &anchor.x, 0.05f, 0.f, 1.f)) {
                w->setAnchor(anchor);
            }

            Vec2 size = w->size();
            if (ImGui::DragFloat2("Size", &size.x)) {
                w->setSize(size);
            }

            Vec2 scale = w->getScale();
            if (ImGui::DragFloat2("Scale", &scale.x, 0.01f, 0.01f)) {
                w->setScale(scale);
            }

            bool visible = w->isVisible();
            if (ImGui::Checkbox("Visible", &visible)) {
                w->setVisible(visible);
            }

            if (auto label = dynamic_cast<Label*>(w)) {
                ImGui::Separator();

                std::string text = label->getText();
                if (ImGui::InputText("Text", &text)) {
                    label->setText(text);
                }

                auto color = label->getColor().toVec4();
                if (ImGui::ColorEdit4("Color", &color.r)) {
                    label->setColor(Color::fromVec4(color));
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Invalidate")) {
                w->markDirtyRecursive();
            }

            ImGui::SameLine();

            if (ImGui::Button("Re-layout")) {
                w->layout();
            }

            {
                ImGui::Separator();
                auto& lp = w->layoutParams();
                ImGui::DragFloat("Flex Weight", &lp.weight, 0.1f);

                char const* alignItems[] = {"Unset", "Start", "Center", "End", "Stretch"};
                char const* sizeModeItems[] = {"Fit", "Fill"};

                int widthModeIndex = static_cast<int>(lp.widthMode);
                if (ImGui::Combo("Width Mode", &widthModeIndex, sizeModeItems, std::size(sizeModeItems))) {
                    lp.widthMode = static_cast<LayoutParams::SizeMode>(widthModeIndex);
                }

                int heightModeIndex = static_cast<int>(lp.heightMode);
                if (ImGui::Combo("Height Mode", &heightModeIndex, sizeModeItems, std::size(sizeModeItems))) {
                    lp.heightMode = static_cast<LayoutParams::SizeMode>(heightModeIndex);
                }

                int alignSelfIndex = static_cast<int>(lp.alignSelf);
                if (ImGui::Combo("Align Self", &alignSelfIndex, alignItems, std::size(alignItems))) {
                    lp.alignSelf = static_cast<LayoutParams::Align>(alignSelfIndex);
                }
            }

            if (auto box = dynamic_cast<Box*>(w)) {
                ImGui::Separator();

                auto padding = box->padding();
                if (ImGui::DragFloat4("Padding", &padding.left)) {
                    box->setPadding(padding);
                }

                auto alignment = box->alignment();
                char const* alignItems[] = {"Start", "Center", "End", "Stretch"};
                int alignIndex = static_cast<int>(alignment);
                if (ImGui::Combo("Alignment", &alignIndex, alignItems, std::size(alignItems))) {
                    box->setAlignment(static_cast<Alignment>(alignIndex));
                }
            } else if (auto row = dynamic_cast<Row*>(w)) {
                ImGui::Separator();

                auto gap = row->getGap();
                if (ImGui::DragFloat("Gap", &gap, 0.5f)) {
                    row->setGap(gap);
                }

                auto hAlign = row->getHorizontalArrangement();
                char const* hAlignItems[] = {"Start", "End", "Center", "Space Between", "Space Around", "Space Evenly"};
                int hAlignIndex = static_cast<int>(hAlign);
                if (ImGui::Combo("Horizontal Arrangement", &hAlignIndex, hAlignItems, std::size(hAlignItems))) {
                    row->setHorizontalArrangement(static_cast<Arrangement>(hAlignIndex));
                }

                auto vAlign = row->getVerticalAlignment();
                char const* vAlignItems[] = {"Start", "Center", "End", "Stretch"};
                int vAlignIndex = static_cast<int>(vAlign);
                if (ImGui::Combo("Vertical Alignment", &vAlignIndex, vAlignItems, std::size(vAlignItems))) {
                    row->setVerticalAlignment(static_cast<Alignment>(vAlignIndex));
                }
            } else if (auto column = dynamic_cast<Column*>(w)) {
                ImGui::Separator();

                auto gap = column->getGap();
                if (ImGui::DragFloat("Gap", &gap, 0.5f)) {
                    column->setGap(gap);
                }

                auto vAlign = column->getVerticalArrangement();
                char const* vAlignItems[] = {"Start", "End", "Center", "Space Between", "Space Around", "Space Evenly"};
                int vAlignIndex = static_cast<int>(vAlign);
                if (ImGui::Combo("Vertical Arrangement", &vAlignIndex, vAlignItems, std::size(vAlignItems))) {
                    column->setVerticalArrangement(static_cast<Arrangement>(vAlignIndex));
                }

                auto hAlign = column->getHorizontalAlignment();
                char const* hAlignItems[] = {"Start", "Center", "End", "Stretch"};
                int hAlignIndex = static_cast<int>(hAlign);
                if (ImGui::Combo("Horizontal Alignment", &hAlignIndex, hAlignItems, std::size(hAlignItems))) {
                    column->setHorizontalAlignment(static_cast<Alignment>(hAlignIndex));
                }
            }
        }
        ImGui::End();
    }

    static void ShowFontsViewer() {
        static bool showGlyphRects = true;
        static bool showGlyphLabels = false;
        static bool hideEmptyGlyphs = true;
        static float atlasPreviewWidth = 320.f;
        static int glyphFilter = -1;

        if (ImGui::Begin("Fonts")) {
            auto& fm = FontManager::get();
            auto& fonts = fm.getAllFonts();

            ImGui::Checkbox("Show Glyph Rects", &showGlyphRects);
            ImGui::SameLine();
            ImGui::Checkbox("Show Glyph IDs", &showGlyphLabels);
            ImGui::SameLine();
            ImGui::Checkbox("Hide Empty", &hideEmptyGlyphs);
            ImGui::DragFloat("Preview Width", &atlasPreviewWidth, 1.f, 64.f, 2048.f, "%.0f px");
            ImGui::InputInt("Glyph ID Filter", &glyphFilter);
            if (glyphFilter < -1) {
                glyphFilter = -1;
            }
            ImGui::Separator();

            for (auto& font : fonts) {
                if (ImGui::CollapsingHeader(fmt::format("Font ID {}", font.first.value).c_str())) {
                    auto* atlasGroup = font.second.atlasGroup.get();
                    if (!atlasGroup) {
                        ImGui::TextUnformatted("No atlas group");
                        continue;
                    }

                    int i = 1;
                    for (auto& texture : font.second.textures) {
                        if (ImGui::TreeNode(fmt::format("Page {}", i++).c_str())) {
                            auto const pageIndex = static_cast<size_t>(i - 2);
                            auto const& atlas = atlasGroup->page(pageIndex);

                            float previewHeight = atlas.width() > 0
                                ? atlasPreviewWidth * (static_cast<float>(atlas.height()) / static_cast<float>(atlas.width()))
                                : atlasPreviewWidth;
                            ImVec2 imageSize(atlasPreviewWidth, previewHeight);
                            ImVec2 imageMin = ImGui::GetCursorScreenPos();

                            ImGui::Image(
                                texture.value,
                                imageSize,
                                ImVec2(0, 1), ImVec2(1, 0)
                            );

                            ImGui::Text("Cached glyphs: %zu", atlas.regions().size());

                            if (showGlyphRects && atlas.width() > 0 && atlas.height() > 0) {
                                float scaleX = imageSize.x / static_cast<float>(atlas.width());
                                float scaleY = imageSize.y / static_cast<float>(atlas.height());
                                auto* drawList = ImGui::GetWindowDrawList();

                                bool hasHoveredGlyph = false;
                                uint32_t hoveredGlyph = 0;
                                tint::AtlasRegion hoveredRegion{};

                                for (auto const& [glyphIndex, region] : atlas.regions()) {
                                    if (glyphFilter >= 0 && glyphIndex != static_cast<uint32_t>(glyphFilter)) {
                                        continue;
                                    }
                                    if (hideEmptyGlyphs && (region.rect.w == 0 || region.rect.h == 0)) {
                                        continue;
                                    }

                                    float x0 = imageMin.x + static_cast<float>(region.rect.x) * scaleX;
                                    float x1 = x0 + static_cast<float>(region.rect.w) * scaleX;
                                    float y0 = imageMin.y + static_cast<float>(atlas.height() - (region.rect.y + static_cast<int32_t>(region.rect.h))) * scaleY;
                                    float y1 = y0 + static_cast<float>(region.rect.h) * scaleY;

                                    ImVec2 p0(x0, y0);
                                    ImVec2 p1(x1, y1);
                                    drawList->AddRect(p0, p1, IM_COL32(255, 196, 64, 240));

                                    if (showGlyphLabels && region.rect.w > 0 && region.rect.h > 0) {
                                        std::string label = fmt::format("{}", glyphIndex);
                                        drawList->AddText(ImVec2(x0 + 1.f, y0 + 1.f), IM_COL32(255, 255, 255, 240), label.c_str());
                                    }

                                    if (!hasHoveredGlyph && ImGui::IsMouseHoveringRect(p0, p1, false)) {
                                        hasHoveredGlyph = true;
                                        hoveredGlyph = glyphIndex;
                                        hoveredRegion = region;
                                    }
                                }

                                if (hasHoveredGlyph) {
                                    ImGui::BeginTooltip();
                                    ImGui::Text("Glyph: %u", hoveredGlyph);
                                    ImGui::Text("Rect: x=%d y=%d w=%u h=%u",
                                        hoveredRegion.rect.x,
                                        hoveredRegion.rect.y,
                                        hoveredRegion.rect.w,
                                        hoveredRegion.rect.h);
                                    ImGui::Text("Bearing: x=%d y=%d", hoveredRegion.bearingX, hoveredRegion.bearingY);
                                    ImGui::EndTooltip();
                                }
                            }

                            ImGui::TreePop();
                        }
                    }
                }
            }
        }
        ImGui::End();
    }

    static void ShowMetrics() {
        if (ImGui::Begin("Metrics")) {
            auto& metrics = DrawStats::snapshot();
            ImGui::Text("Draw Calls: %zu", metrics.drawCalls);
            ImGui::Text("Vertices: %zu", metrics.vertices);
            ImGui::Text("Indices: %zu", metrics.indices);
            ImGui::Text("Widget Redraws: %zu", metrics.widgetRedraws);
            ImGui::Text("Widget Relayouts: %zu", metrics.widgetRelayouts);
            ImGui::Text("Widgets Submitted: %zu", metrics.widgetsSubmitted);
            ImGui::Text("Commands: %zu", metrics.commands);
            ImGui::Text("Batches: %zu", metrics.batches);
            ImGui::Text("Merged Commands: %zu", metrics.mergedCommands);
            ImGui::Text("Frame CPU Time: %.2f ms", metrics.frameCPUms);
        }
        ImGui::End();
    }

    void DevTools::render() {
        auto& state = DevToolsState::get();
        if (state.captureMainFramebuffer) {
            glBindFramebuffer(GL_FRAMEBUFFER, state.prevFramebuffer);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Settings");
        ImGui::Checkbox("Capture Main Framebuffer", &state.captureMainFramebuffer);
        ImGui::End();

        if (state.captureMainFramebuffer) {
            ImGui::Begin("Framebuffer");
            ImGui::Image(
                state.framebufferTexture,
                ImGui::GetContentRegionAvail(),
                ImVec2(0, 1), ImVec2(1, 0)
            );
            ImGui::End();
        }

        ShowWidgetTree();
        ShowWidgetInspector();
        ShowFontsViewer();
        ShowMetrics();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    std::optional<Rect> DevTools::getSelectedBounds() {
        auto& state = DevToolsState::get();
        auto w = state.selectedWidget;
        if (!w) return std::nullopt;

        return w->worldTransform().transformRect({{0.f, 0.f}, w->size()});
    }
}