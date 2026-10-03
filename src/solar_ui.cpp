#include "label_layout.h"
#include <cctype>
#include "solar_ui.h"
#include "anomaly_catalog.h"
#include "settings_persistence.h"
#include "lod_manager.h"
#include "save_state.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

void SolarOdysseyUI::applySpaceTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.WindowRounding = 5.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        style.WindowBorderSize = 0.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;

        style.WindowPadding = ImVec2(14.0f, 14.0f);
        style.FramePadding = ImVec2(10.0f, 6.0f);
        style.ItemSpacing = ImVec2(10.0f, 8.0f);
        style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);

        // Deep space translucent glassmorphism palette
        colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.95f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.55f, 0.65f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.06f, 0.08f, 0.12f, 0.85f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.11f, 0.16f, 0.75f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.07f, 0.09f, 0.14f, 0.94f);
        colors[ImGuiCol_Border]                = ImVec4(0.20f, 0.30f, 0.45f, 0.40f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.16f, 0.24f, 0.75f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.26f, 0.38f, 0.85f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.32f, 0.48f, 0.95f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.06f, 0.09f, 0.14f, 0.95f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.10f, 0.15f, 0.24f, 0.95f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.06f, 0.08f, 0.12f, 0.75f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.08f, 0.11f, 0.16f, 0.90f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.04f, 0.06f, 0.09f, 0.60f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.28f, 0.42f, 0.75f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.28f, 0.40f, 0.58f, 0.85f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.35f, 0.50f, 0.72f, 0.95f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.30f, 0.75f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.28f, 0.68f, 0.95f, 0.85f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.40f, 0.82f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.22f, 0.34f, 0.80f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.22f, 0.36f, 0.55f, 0.90f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.30f, 0.50f, 0.75f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.16f, 0.25f, 0.38f, 0.75f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.24f, 0.38f, 0.56f, 0.85f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.30f, 0.48f, 0.70f, 0.95f);
        colors[ImGuiCol_Separator]             = ImVec4(0.20f, 0.28f, 0.42f, 0.45f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.30f, 0.45f, 0.65f, 0.75f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.40f, 0.60f, 0.85f, 0.95f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.20f, 0.28f, 0.42f, 0.40f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.30f, 0.45f, 0.65f, 0.70f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.40f, 0.60f, 0.85f, 0.90f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.15f, 0.24f, 0.80f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.22f, 0.36f, 0.55f, 0.90f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.18f, 0.30f, 0.48f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.08f, 0.12f, 0.18f, 0.75f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.12f, 0.18f, 0.28f, 0.85f);
    }

    // World to Screen projection helper
bool SolarOdysseyUI::projectWorldToScreen(const glm::vec3& worldPos, const glm::mat4& viewMatrix,
                                         const glm::mat4& projMatrix, float screenWidth, float screenHeight,
                                         glm::vec2& outScreenPos, float& outDistance) {
        glm::vec4 clipPos = projMatrix * viewMatrix * glm::vec4(worldPos, 1.0f);
        if (clipPos.w <= 0.001f) return false; // Behind camera

        glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;
        if (ndc.z < -1.0f || ndc.z > 1.0f) return false;

        outScreenPos.x = (ndc.x * 0.5f + 0.5f) * screenWidth;
        outScreenPos.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * screenHeight;
        outDistance = clipPos.w;
        return true;
    }

    // Render 3D Floating Planet Labels
void SolarOdysseyUI::renderFloatingLabels(const std::vector<PickableBody>& bodies, const CelestialDatabase& db,
                                          const glm::mat4& viewMatrix, const glm::mat4& projMatrix,
                                          float screenWidth, float screenHeight, CameraController& cam) {
        if (!showLabels || cam.photoModeActive || cam.mode == CAM_SPACESHIP) return;

    struct Candidate { const PickableBody* body; glm::vec2 screen; float projectedRadius; int priority; };
    std::vector<Candidate> candidates;
    const float focalLength = .5f * screenHeight * projMatrix[1][1];
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    for (const auto& body : bodies) {
        glm::vec2 screen; float depth;
        if (!projectWorldToScreen(body.position, viewMatrix, projMatrix, screenWidth, screenHeight, screen, depth)) continue;
        if (screen.x < 0 || screen.x > screenWidth || screen.y < 0 || screen.y > screenHeight) continue;
        const bool selected = selectedPlanetName == body.name;
        const float pixels = body.radius * focalLength / std::max(.001f, depth);
        const bool hovered = glm::length(screen - glm::vec2(mouse.x, mouse.y)) < std::max(10.0f, pixels);
        const bool moon = body.index >= 100;
        if (moon && !selected && !hovered && pixels < 2.5f) continue;
        const glm::vec3 offset = body.position - cam.currentEye;
        const float distance = glm::length(offset);
        if (distance <= body.radius) continue;
        Ray ray{cam.currentEye, offset / distance};
        bool hidden = false;
        for (const auto& other : bodies) {
            if (&other == &body) continue;
            const float hit = RaycastPicker::intersectRaySphere(ray, other.position, other.radius);
            if (hit >= 0 && hit < distance - body.radius) { hidden = true; break; }
        }
        if (hidden) continue;
        candidates.push_back({&body, screen, pixels, selected ? 100 : hovered ? 80 : moon ? 10 : 40});
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        return a.priority != b.priority ? a.priority > b.priority : a.projectedRadius > b.projectedRadius;
    });
    std::vector<LabelLayout::Rect> occupied{{0,0,screenWidth,78}, {0,screenHeight-82,screenWidth,82}};
    if (showPlanetCard) occupied.push_back({screenWidth - 400, 78, 400, screenHeight - 160});
    auto* draw = ImGui::GetBackgroundDrawList();
    for (const auto& candidate : candidates) {
        const ImVec2 text = ImGui::CalcTextSize(candidate.body->name.c_str());
        auto rect = LabelLayout::place(candidate.screen.x, candidate.screen.y - std::min(candidate.projectedRadius + 12.0f, screenHeight * .4f),
            text.x + 20, text.y + 12, screenWidth, screenHeight, occupied);
        if (!rect) continue;
        occupied.push_back(*rect);
        const bool selected = candidate.priority == 100;
        const ImVec2 lo(rect->x,rect->y), hi(rect->x + rect->width, rect->y + rect->height);
        draw->AddLine(ImVec2(candidate.screen.x,candidate.screen.y), ImVec2(rect->x + rect->width/2,rect->y + rect->height), IM_COL32(106,136,153,90));
        draw->AddRectFilled(lo,hi, selected ? IM_COL32(27,48,65,220) : IM_COL32(8,16,25,185), 4);
        draw->AddText(ImVec2(lo.x+10,lo.y+6), selected ? IM_COL32(239,247,252,255) : IM_COL32(166,190,204,230), candidate.body->name.c_str());
    }
}

    // Top Navigation & Quick Select Bar
void SolarOdysseyUI::renderTopNavBar(float screenWidth, CameraController& cam, const CelestialDatabase& db,
                                     std::vector<std::pair<std::string, int>>& planetIndexMap,
                                     const PresentationController& psm) {
    if (cam.photoModeActive) return;
    ImGui::SetNextWindowPos(ImVec2(16,16), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenWidth - 32, screenWidth < 1000 ? 96 : 54), ImGuiCond_Always);
    if (ImGui::Begin("Navigation", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImVec4(.65f,.82f,.9f,1), "SOLAR ODYSSEY"); ImGui::SameLine();
        if (ImGui::Button("Find a world")) ImGui::OpenPopup("Object finder");
        if (ImGui::BeginPopup("Object finder")) {
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##SearchWorld", "Name a planet or moon...", finderSearch, sizeof(finderSearch));
            std::string query(finderSearch);
            std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c){return std::tolower(c);});
            ImGui::BeginChild("FinderResults", ImVec2(350, 320));
            std::vector<std::string> names = db.getOrder();
            for (const auto& root : db.getOrder()) {
                if (!onQueryChildren) continue;
                for (const auto& child : onQueryChildren(root))
                    if (std::find(names.begin(), names.end(), child) == names.end()) names.push_back(child);
            }
            for (const auto& name : names) {
                std::string lower = name;
                std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){return std::tolower(c);});
                if (lower.find(query) == std::string::npos) continue;
                if (ImGui::Selectable(name.c_str(), selectedPlanetName == name)) {
                    if (onEnterBodyMode) onEnterBodyMode(name);
                    showPlanetCard = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndChild(); ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button(psm.isSystem() ? "Explorer (Y)" : "System (Y)")) { if (onToggleSystemView) onToggleSystemView(); }
        ImGui::SameLine();
        if (ImGui::Button("Codex")) showCodex = !showCodex;
        ImGui::SameLine();
        if (ImGui::Button("Explore")) ImGui::OpenPopup("Exploration tools");
        if (ImGui::BeginPopup("Exploration tools")) {
            if (ImGui::MenuItem("Free camera", "F")) cam.toggleFreeCam();
            if (ImGui::MenuItem("Spacecraft", "X")) cam.mode = CAM_SPACESHIP;
            if (ImGui::MenuItem(cam.tourActive ? "Stop tour" : "Cinematic tour", "T")) { if (cam.tourActive) cam.stopTour(); else cam.startTour(); }
            if (ImGui::MenuItem("Black hole", "B") && onFocusBody) onFocusBody("Black Hole");
            if (ImGui::MenuItem("Wormhole", "K") && onFocusBody) onFocusBody("Wormhole");
            if (ImGui::MenuItem("Photography", "P")) cam.togglePhotoMode();
            if (ImGui::MenuItem("Open captured photos") && onOpenPhotos) onOpenPhotos();
            if (ImGui::MenuItem("Diagnostics")) showDiagnostics = !showDiagnostics;
            ImGui::EndPopup();
        }
        if (screenWidth < 1000) ImGui::NewLine(); else ImGui::SameLine();
        ImGui::TextDisabled("%s", psm.isBody() ? selectedPlanetName.c_str() : psm.isSystem() ? "SYSTEM" : "EXPLORER");
        ImGui::SameLine();
        if (ImGui::Button("Menu (Esc)")) { if (onOpenMenu) onOpenMenu(); }
    }
    ImGui::End();
}

void SolarOdysseyUI::renderBottomControlBar(float screenWidth, float screenHeight, CameraController& cam) {
        if (cam.photoModeActive || cam.mode == CAM_SPACESHIP) return;

        float barWidth = std::min(870.0f, screenWidth - 32.0f);
        float barHeight = 52.0f;
        ImGui::SetNextWindowPos(ImVec2((screenWidth - barWidth) * 0.5f, screenHeight - barHeight - 16.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(barWidth, barHeight), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("BottomControlBar", nullptr, flags)) {
            // Play / Pause Button
            if (ImGui::Button(isPaused ? " Play " : " Pause ")) {
                isPaused = !isPaused;
            }

            ImGui::SameLine(0, 16);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Speed:");
            ImGui::SameLine(0, 8);

            // Speed Presets
            float presets[] = {0.25f, 0.5f, 1.0f, 2.0f, 5.0f, 10.0f, 25.0f, 50.0f};
            const char* presetLabels[] = {"0.25x", "0.5x", "1x", "2x", "5x", "10x", "25x", "50x"};

            for (int i = 0; i < 8; ++i) {
                bool active = (std::abs(timeMultiplier - presets[i]) < 0.01f);
                if (active) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.55f, 0.85f, 0.90f));
                }
                if (ImGui::Button(presetLabels[i])) {
                    timeMultiplier = presets[i];
                }
                if (active) {
                    ImGui::PopStyleColor();
                }
                ImGui::SameLine(0, 4);
            }

            ImGui::SameLine(0, 12);
            // Elapsed time indicator
            ImGui::TextColored(ImVec4(0.70f, 0.80f, 0.90f, 1.0f), "Day %.0f", elapsedSimDays);

            if (physicsMode == 1) {
                ImGui::SameLine(0, 8);
                ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.20f, 1.0f), "[N-BODY GRAVITY]");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Numerical gravity integrates the 13 roots; moons remain analytic live-parent children.");
                }
            }
        }
        ImGui::End();
    }

    // Floating Planetary Dossier Card
void SolarOdysseyUI::renderPlanetCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                                      CameraController& cam,
                                      std::function<void(const std::string&)> onFocus,
                                      std::function<void(const std::string&)> onExplorePOV,
                                      std::function<void(const std::string&)> onEnterBodyMode,
                                      std::function<void()> onExitBodyMode,
                                      const PresentationController& psm,
                                      BodyLayerId effectiveLayer) {
        if (!showPlanetCard || selectedPlanetName.empty() || cam.photoModeActive || cam.mode == CAM_SPACESHIP) return;

        // PSM.3: in BODY mode the dossier shows the BODY owned by the
        // PresentationController (single selection state — never a second
        // identity). Outside BODY the existing preview/card behavior applies.
        const bool showingBody = psm.isBody() && !psm.selectedBodyName().empty();
        const std::string& dossierName = showingBody ? psm.selectedBodyName() : selectedPlanetName;
        const CelestialBodyData* data = db.getBody(dossierName);
        if (!data) return;

        ImGui::SetNextWindowPos(ImVec2(screenWidth - 440.0f, 85.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(410.0f, 560.0f), ImGuiCond_FirstUseEver);

        // Cache the window title: rebuilding the string every frame while the card
        // is open causes a needless heap allocation per frame.
        static std::string cachedTitle;
        static std::string cachedForName;
        if (cachedForName != data->name) {
            cachedTitle = data->name + " - Planetary Dossier###PlanetCard";
            cachedForName = data->name;
        }
        if (ImGui::Begin(cachedTitle.c_str(), &showPlanetCard)) {
            // Header with Theme Color Accent
            ImVec4 themeCol(data->themeColor.r, data->themeColor.g, data->themeColor.b, 1.0f);
            ImGui::TextColored(themeCol, "%s", data->name.c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("|  %s", data->type.c_str());

            ImGui::TextColored(ImVec4(0.75f, 0.85f, 0.95f, 1.0f), "%s", data->subtitle.c_str());
            // PSM.3: BODY-mode badge — the dossier is the primary body
            // information panel while presenting this BODY.
            if (showingBody) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.55f, 0.95f, 0.65f, 1.0f), " [BODY MODE]");
            }
            // PSM.7: parent breadcrumb on a moon BODY (e.g. "Earth > Moon").
            // Clicking the parent transfers through the SAME authoritative
            // BODY-entry intent path — no camera manipulation from UI.
            if (showingBody && onQueryParent) {
                const std::string navParent = onQueryParent(dossierName);
                if (!navParent.empty()) {
                    if (ImGui::Button(navParent.c_str(), ImVec2(0, 0))) {
                        if (onEnterBodyMode) onEnterBodyMode(navParent);
                    }
                    ImGui::SameLine(0, 6);
                    ImGui::TextDisabled(">");
                    ImGui::SameLine(0, 6);
                    ImGui::Text("%s", data->name.c_str());
                }
            }
            ImGui::Separator();

            // Quick Actions: Focus Camera & Explore POV
            if (ImGui::Button(" Focus Camera", ImVec2(185, 32))) {
                if (onFocus) onFocus(data->name);
            }
            ImGui::SameLine(0, 10);
            if (ImGui::Button(" Explore POV", ImVec2(185, 32))) {
                if (onExplorePOV) onExplorePOV(data->name);
            }
            // PSM.3: no duplicated BODY control — when this dossier already
            // shows the active BODY, offer the exit instead of re-entry.
            if (showingBody) {
                if (ImGui::Button(" Exit Body Mode (F)", ImVec2(380, 32))) {
                    if (onExitBodyMode) onExitBodyMode();
                }
            } else {
                // PSM.2: records intent only; the updatePresentation() drain
                // enters through enterBodyView.
                if (ImGui::Button(" Enter Body Mode", ImVec2(380, 32))) {
                    if (onEnterBodyMode) onEnterBodyMode(data->name);
                }
            }
            // Cycle 4 Pass 2: science-scan actions for the dossier body.
            // Atmospheric scans on inapplicable bodies toast the N/A message
            // (never a fake scan); gravity runs a Detailed session.
            if (ImGui::Button(" Scan Atmosphere", ImVec2(185, 28))) {
                if (onStartAtmosphericScan) onStartAtmosphericScan(data->name);
            }
            ImGui::SameLine(0, 10);
            if (ImGui::Button(" Gravity Measure", ImVec2(185, 28))) {
                if (onStartGravityScan) onStartGravityScan(data->name);
            }
            // PSM.7: compact Satellites list on a parent BODY. Each child
            // button records intent through the EXISTING authoritative funnel
            // (same as Enter/V/dossier action) — the PSM.2 cinematic transfer
            // path handles the rest. BODY-only; previews stay unchanged.
            if (showingBody && onQueryChildren) {
                const std::vector<std::string> navChildren = onQueryChildren(dossierName);
                if (!navChildren.empty()) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Satellites:");
                    for (const auto& child : navChildren) {
                        if (ImGui::Button(child.c_str(), ImVec2(380, 26))) {
                            if (onEnterBodyMode) onEnterBodyMode(child);
                        }
                    }
                }
            }

            ImGui::Spacing();
            if (ImGui::BeginTabBar("PlanetInfoTabs")) {
                // Tab 1: Overview — lightweight summary only. Detail lives in
                // the Environment / Orbit / Key Facts tabs (PSM.3 progressive
                // disclosure; the card is not a full-screen dashboard).
                if (ImGui::BeginTabItem("About")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped("%s", data->description.c_str());
                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::Columns(2, "MetricColumns", false);
                    ImGui::SetColumnWidth(0, 180);

                    ImGui::TextDisabled("Type:");
                    ImGui::NextColumn();
                    ImGui::TextWrapped("%s", data->type.c_str());
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Physical Diameter:");
                    ImGui::NextColumn();
                    ImGui::Text("%.1f km (%.2fx Earth)", data->realDiameterKm, data->relativeSizeToEarth);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Distance from Sun:");
                    ImGui::NextColumn();
                    if (data->name == "Sun") {
                        ImGui::Text("Central star");
                    } else {
                        ImGui::Text("%.2f AU (%.1f M km)", data->distanceFromSunAU, data->distanceFromSunMillionKm);
                    }
                    ImGui::NextColumn();

                    ImGui::TextDisabled("%s:", data->temperatureReference.c_str());
                    ImGui::NextColumn();
                    // Generic availability (Moon Expansion 1.3): same rule as
                    // the tilt/range rows — no body-name checks in UI.
                    if (data->hasMeanTemperatureData) {
                        ImGui::Text("%.1f deg C", data->meanTemperatureC);
                    } else {
                        ImGui::Text("N/A");
                    }
                    ImGui::NextColumn();

                    ImGui::Columns(1);
                    ImGui::EndTabItem();
                }

                // Tab 2: Environment — atmosphere, surface, gravity, range.
                if (ImGui::BeginTabItem("Climate")) {
                    ImGui::Spacing();
                    ImGui::TextColored(themeCol, "Atmospheric Composition:");
                    ImGui::TextWrapped("%s", data->atmosphericComposition.c_str());

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(themeCol, "Surface & Geological Features:");
                    ImGui::TextWrapped("%s", data->surfaceFeatures.c_str());

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Columns(2, "EnvMetricColumns", false);
                    ImGui::SetColumnWidth(0, 180);

                    ImGui::TextDisabled("Surface Gravity:");
                    ImGui::NextColumn();
                    ImGui::Text("%.2f m/s^2", data->surfaceGravityMs2);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Temperature Range:");
                    ImGui::NextColumn();
                    // Generic availability (Moon Expansion 1.1): CelestialBodyData
                    // declares whether the range is sourced; unsourced ranges
                    // render "N/A" (existing dossier style) — never a fake span.
                    if (data->hasTemperatureRangeData) {
                        ImGui::Text("%.1f .. %.1f deg C", data->minTemperatureC, data->maxTemperatureC);
                    } else {
                        ImGui::Text("N/A");
                    }
                    ImGui::NextColumn();

                    ImGui::Columns(1);
                    ImGui::EndTabItem();
                }

                // Tab 3: Orbit / Motion (PSM.3) — all motion fields, one place.
                if (ImGui::BeginTabItem("Orbit")) {
                    ImGui::Spacing();
                    ImGui::Columns(2, "OrbitMetricColumns", false);
                    ImGui::SetColumnWidth(0, 180);

                    ImGui::TextDisabled("Distance from Sun:");
                    ImGui::NextColumn();
                    if (data->name == "Sun") {
                        ImGui::Text("Central star (system barycenter)");
                    } else {
                        ImGui::Text("%.2f AU (%.1f M km)", data->distanceFromSunAU, data->distanceFromSunMillionKm);
                    }
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Orbital Period:");
                    ImGui::NextColumn();
                    if (data->orbitalPeriodDays > 0.0f) {
                        ImGui::Text("%.1f Earth days (~%.2f yrs)",
                                    data->orbitalPeriodDays, data->orbitalPeriodDays / 365.25f);
                    } else {
                        ImGui::Text("N/A");
                    }
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Rotation Period:");
                    ImGui::NextColumn();
                    if (data->rotationPeriodHours < 0.0f) {
                        ImGui::Text("%.2f hours (retrograde)", -data->rotationPeriodHours);
                    } else {
                        ImGui::Text("%.2f hours", data->rotationPeriodHours);
                    }
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Axial Tilt:");
                    ImGui::NextColumn();
                    // Generic availability (Moon Expansion 1.1): same rule as
                    // Temperature Range above — no body-name checks in UI.
                    if (data->hasAxialTiltData) {
                        ImGui::Text("%.2f deg", data->axialTiltDeg);
                    } else {
                        ImGui::Text("N/A");
                    }
                    ImGui::NextColumn();

                    // PSM.3 Sun guard: knownMoons holds the major-planet count
                    // for the Sun — never render it as "Confirmed Moons: 8".
                    ImGui::TextDisabled("%s", dossierMoonsRowLabel(*data));
                    ImGui::NextColumn();
                    ImGui::Text("%d", data->knownMoons);
                    ImGui::NextColumn();

                    ImGui::Columns(1);
                    ImGui::EndTabItem();
                }

                // Tab 4: Key Scientific Facts (+ discovery note).
                if (ImGui::BeginTabItem("Facts")) {
                    ImGui::Spacing();
                    for (size_t i = 0; i < data->keyFacts.size(); ++i) {
                        ImGui::Bullet();
                        ImGui::TextWrapped("%s", data->keyFacts[i].c_str());
                        ImGui::Spacing();
                    }
                    ImGui::Separator();
                    ImGui::TextColored(themeCol, "Discovery:");
                    ImGui::TextWrapped("%s", data->discoveryInfo.c_str());
                    ImGui::EndTabItem();
                }

                // Tab 5: Layers (PSM.4) — semantic availability only. The
                // displayed active layer is the EFFECTIVE layer computed
                // Engine-side (declared AND resource-ready); this tab never
                // stores layer state and can never present a resource-missing
                // layer as Active. Unavailable layers render disabled with a
                // reason; selecting an available one calls the single
                // authoritative selection path (Engine-side gate included).
                if (ImGui::BeginTabItem("Layers")) {
                    ImGui::Spacing();
                    const BodyLayerCapabilities caps = declaredBodyLayerCapabilities(data->name);
                    const BodyLayerId current = effectiveLayer;
                    for (int li = 0; li <= static_cast<int>(BodyLayerId::Scientific); ++li) {
                        const BodyLayerId id = static_cast<BodyLayerId>(li);
                        const bool available = isLayerDeclaredAvailable(id, caps);
                        const bool isCurrent = (id == current);
                        // PSM.5/PSM.6: short truthful context for science
                        // datasets (Venus/Earth/Mars only, helper-owned).
                        const char* sliceNote = bodyLayerContextNote(data->name, id);
                        const char* sliceSep = (sliceNote[0] != '\0') ? " — " : "";
                        if (!available || isCurrent) {
                            ImGui::BeginDisabled();
                            ImGui::Button(isCurrent ? "Active" : bodyLayerLabel(id), ImVec2(150, 26));
                            ImGui::EndDisabled();
                        } else {
                            if (ImGui::Button(bodyLayerLabel(id), ImVec2(150, 26))) {
                                if (onSelectLayer) onSelectLayer(id);
                            }
                        }
                        ImGui::SameLine(0, 8);
                        if (isCurrent) {
                            ImGui::TextColored(ImVec4(0.55f, 0.95f, 0.65f, 1.0f), "%s (active)%s%s",
                                               bodyLayerLabel(id), sliceSep, sliceNote);
                        } else if (!available) {
                            ImGui::TextDisabled("%s (%s)", bodyLayerLabel(id),
                                               layerUnavailableReason(id));
                        } else {
                            ImGui::TextDisabled("%s%s%s", bodyLayerLabel(id), sliceSep, sliceNote);
                        }
                    }
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextDisabled("Keys 1-5 select layers while in BODY.");
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

void SolarOdysseyUI::renderPlanetInfoCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                                          CameraController& cam,
                                          std::function<void(const std::string&)> onFocus,
                                          std::function<void(const std::string&)> onExplorePOV,
                                          std::function<void(const std::string&)> onEnterBodyMode,
                                          std::function<void()> onExitBodyMode,
                                          const PresentationController& psm,
                                          BodyLayerId effectiveLayer) {
        renderPlanetCard(screenWidth, screenHeight, db, cam, onFocus, onExplorePOV, onEnterBodyMode, onExitBodyMode, psm, effectiveLayer);
    }

    // Settings & Display Layers Modal / Panel
void SolarOdysseyUI::renderSettingsPanel(PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt,
                                         AtmosphereEffects* atmoEffects, CameraController& cam) {
    if (!showSettingsModal) return;
    const ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(screen.x * .5f, screen.y * .5f), ImGuiCond_Always, ImVec2(.5f,.5f));
    ImGui::SetNextWindowSize(ImVec2(std::min(790.0f, screen.x - 48), std::min(680.0f, screen.y - 64)), ImGuiCond_Always);
    if (ImGui::Begin("SETTINGS", &showSettingsModal, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextDisabled("Changes apply immediately. Esc returns to your menu.");
        ImGui::Spacing();
        if (ImGui::BeginTabBar("Preferences")) {
            if (ImGui::BeginTabItem("DISPLAY")) {
                if (ImGui::Checkbox("Fullscreen", &isFullscreen)) pendingFullscreenToggle = true;
                if (ImGui::Checkbox("VSync / match display refresh", &vsyncEnabled) && onVSyncChanged) onVSyncChanged(vsyncEnabled);
                if (ImGui::SliderFloat("Field of view", &cam.targetFieldOfView, 20, 120, "%.0f degrees")) cam.fieldOfView = cam.targetFieldOfView;
                ImGui::Spacing(); ImGui::Separator();
                const char* styles[] = {"Classic / cinematic", "Scientific / catalog imagery"};
                ImGui::Combo("Starfield style", &starfieldStyle, styles, 2);
                if (starfieldStyle == 0) {
                    const char* panoramas[] = {"Stars", "Stars + Milky Way"};
                    starfieldDataset = std::clamp(starfieldDataset, 0, 1);
                    ImGui::Combo("Panorama", &starfieldDataset, panoramas, 2);
                }
                if (starfieldStyle == 1) {
                    const char* datasets[] = {"Yale / bright stars", "Hipparcos", "Tycho", "Combined catalogs"};
                    ImGui::Combo("Catalog", &starfieldDataset, datasets, 4);
                    ImGui::TextWrapped("Catalog map positions are preserved. The sky is a visualization, not an Earth-location planetarium or a navigational reference.");
                }
                ImGui::Checkbox("Body labels", &showLabels);
                ImGui::Checkbox("Reference orbit circles", &showOrbits);
                ImGui::Checkbox("Dwarf planets", &showDwarfPlanets);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("GRAPHICS")) {
                int quality = qualityPreset;
                const char* tiers[] = {"Low", "Medium", "High", "Ultra"};
                if (ImGui::Combo("Quality", &quality, tiers, 4) && onQualityChanged) onQualityChanged(static_cast<GraphicsQuality>(quality));
                ImGui::Checkbox("Dynamic mesh detail", &enableMeshLOD);
                ImGui::Checkbox("Atmosphere shells", &showAtmospheres);
                ImGui::SliderFloat("Atmosphere glow", &atmosphereGlowScale, 0, 3);
                ImGui::Checkbox("Asteroid belt", &showAsteroids);
                ImGui::Checkbox("Solar particles and comets", &showParticles);
                ImGui::SliderFloat("Sun intensity", &sunIntensity, .1f, 5.0f);
                ImGui::SliderFloat("Ring opacity", &ringOpacity, .1f, 1.0f);
                ImGui::Separator();
                ImGui::Checkbox("Cinematic effects", &postProc.enabled);
                ImGui::BeginDisabled(!postProc.enabled);
                ImGui::Checkbox("Bloom", &postProc.bloomEnabled);
                ImGui::SliderFloat("Bloom intensity", &postProc.bloomIntensity, .1f, 1.2f);
                ImGui::SliderFloat("Bloom threshold", &postProc.bloomThreshold, .5f, 1.2f);
                ImGui::Checkbox("Filmic tone mapping", &postProc.toneMappingEnabled);
                ImGui::SliderFloat("Exposure", &postProc.exposure, .5f, 2.5f);
                ImGui::Checkbox("Vignette", &postProc.vignetteEnabled);
                ImGui::EndDisabled();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("AUDIO")) {
                ImGui::Checkbox("Mute audio", &audioMuted);
                ImGui::SliderFloat("Master volume", &masterVolume, 0, 1);
                ImGui::SliderFloat("Music volume", &musicVolume, 0, 1);
                ImGui::SliderFloat("Effects volume", &sfxVolume, 0, 1);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CONTROLS")) {
                ImGui::SliderFloat("Free camera speed", &cam.freeSpeed, 2, 120, "%.1f units/s");
                ImGui::SliderFloat("Mouse sensitivity", &cam.freeSensitivity, .02f, .35f, "%.3f");
                ImGui::Spacing();
                ImGui::TextWrapped("Drag to orbit. Scroll to zoom. Y: System view. Enter: inspect selected body. F: free camera. X: spacecraft. T: tour. P: photography. F12: capture. F5 / F9: save / load. Esc: pause menu.");
                ImGui::Spacing();
                ImGui::TextWrapped("Free camera: WASD moves, E / Space ascends, Q / C descends. Shift boosts, Ctrl slows. Hold Alt to release the cursor.");
                ImGui::Spacing();
                ImGui::TextWrapped("Spacecraft: W / S thrust, A / D yaw, R / F pitch, Q / E roll, Shift boosts. J: warp / autopilot. H: orbit assist. C: camera view. G: atmosphere scan. Shift+H: gravity measurement.");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("SIMULATION")) {
                const char* modes[] = {"Analytic circular orbits", "Hybrid N-body"};
                if (ImGui::Combo("Motion model", &physicsMode, modes, 2)) pendingPhysicsModeChange = true;
                ImGui::TextWrapped("A compressed, stylized solar system. Numerical mode integrates 13 roots; 18 analytic moons follow their live parents. This is not an ephemeris or mission-planning tool.");
                ImGui::SliderFloat("Visual body scale", &planetScale, .25f, 3.5f);
                ImGui::SliderFloat("Analytic revolution speed", &orbitSpeedScale, 0, 10);
                ImGui::SliderFloat("Axial spin speed", &spinSpeedScale, 0, 10);
                ImGui::Checkbox("Sourced axial tilts", &enableAxialTilt);
                ImGui::Checkbox("Save exploration on exit / main menu", &autoSaveOnExit);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::Spacing(); ImGui::Separator();
        if (ImGui::Button("Restore default settings")) ImGui::OpenPopup("Restore defaults?");
        if (ImGui::BeginPopupModal("Restore defaults?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Restore settings only? Exploration records and photos are kept.");
            if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
            ImGui::SameLine();
            if (ImGui::Button("Restore")) { if (onResetAllSettings) onResetAllSettings(); ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Back to menu")) showSettingsModal = false;
    }
    ImGui::End();
}

void SolarOdysseyUI::renderFreeCamHUD(float screenWidth, float screenHeight, CameraController& cam) {
        if (cam.mode != CAM_FREE || cam.photoModeActive) return;

        float hudW = 380.0f;
        float hudH = 80.0f;
        ImGui::SetNextWindowPos(ImVec2(24.0f, screenHeight - hudH - 24.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(hudW, hudH), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("FreeCamTelemetry", nullptr, flags)) {
            ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.65f, 1.0f), " 6-DOF FREE FLIGHT CAMERA");
            ImGui::Separator();
            ImGui::Text("Speed: %.1f u/s (Scroll to adjust)", cam.freeSpeed);
            ImGui::TextDisabled("WASD: Move | Space/C: Elevate | [Alt] UI Cursor | [F] Exit");
        }
        ImGui::End();
    }

void SolarOdysseyUI::applyQualityPreset(GraphicsQuality q, PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt) {
        if (q == QUALITY_LOW) {
            if (asteroidBelt) asteroidBelt->setQualityCount(150);
            postProc.bloomEnabled = false;
            postProc.vignetteEnabled = false;
        } else if (q == QUALITY_MEDIUM) {
            if (asteroidBelt) asteroidBelt->setQualityCount(400);
            postProc.bloomEnabled = true;
            postProc.bloomIntensity = 0.35f;
            postProc.vignetteEnabled = true;
        } else if (q == QUALITY_HIGH) {
            if (asteroidBelt) asteroidBelt->setQualityCount(800);
            postProc.bloomEnabled = true;
            postProc.bloomIntensity = 0.45f;
            postProc.vignetteEnabled = true;
        } else if (q == QUALITY_ULTRA) {
            if (asteroidBelt) asteroidBelt->setQualityCount(1400);
            postProc.bloomEnabled = true;
            postProc.bloomIntensity = 0.60f;
            postProc.vignetteEnabled = true;
        }
    }

    // Photo Mode Minimalist HUD
void SolarOdysseyUI::renderPhotoModeHUD(float screenWidth, float screenHeight, CameraController& cam,
                                        PostProcessingPipeline& postProc) {
        if (!cam.photoModeActive) return;

        // Bottom photo controls
        float barWidth = 460.0f;
        float barHeight = 56.0f;
        ImGui::SetNextWindowPos(ImVec2((screenWidth - barWidth) * 0.5f, screenHeight - barHeight - 20.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(barWidth, barHeight), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

        if (ImGui::Begin("PhotoModeToolbar", nullptr, flags)) {
            ImGui::AlignTextToFramePadding();
            ImGui::Text("FOV:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(140.0f);
            ImGui::SliderFloat("##FOV", &cam.targetFieldOfView, 20.0f, 85.0f, "%.0f°");

            ImGui::SameLine(0, 16);
            if (ImGui::Button(" Take Screenshot")) {
                if (onPhotoCapture) onPhotoCapture();
                else postProc.triggerScreenshot();
            }

            ImGui::SameLine(0, 12);
            if (ImGui::Button(" Exit (P)")) {
                cam.setPhotoMode(false);
            }
        }
        ImGui::End();

        // Toast notification if screenshot was taken
        if (postProc.screenshotToastTimer > 0.0f) {
            float toastWidth = 480.0f;
            ImGui::SetNextWindowPos(ImVec2((screenWidth - toastWidth) * 0.5f, 30.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(toastWidth, 40.0f), ImGuiCond_Always);
            ImGuiWindowFlags toastFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
            if (ImGui::Begin("ScreenshotToast", nullptr, toastFlags)) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.4f, 1.0f), " %s", postProc.lastScreenshotPath.c_str());
            }
            ImGui::End();
        }
    }

    // Diagnostics / FPS window
void SolarOdysseyUI::renderDiagnostics(float screenWidth, AsteroidBelt* asteroidBelt) {
        if (!showDiagnostics) return;

        ImGui::SetNextWindowPos(ImVec2(16, 85), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(340, 240), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Diagnostics & Telemetry", &showDiagnostics)) {
            float fps = ImGui::GetIO().Framerate;
            float frameMs = 1000.0f / (fps > 0.001f ? fps : 1.0f);
            ImVec4 fpsColor = (fps >= 55.0f) ? ImVec4(0.35f, 0.95f, 0.55f, 1.0f) :
                              (fps >= 30.0f) ? ImVec4(1.0f, 0.85f, 0.35f, 1.0f) :
                                               ImVec4(1.0f, 0.40f, 0.35f, 1.0f);

            ImGui::TextColored(fpsColor, "Performance: %.1f FPS (%.2f ms)", fps, frameMs);
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "Hardware & Driver:");
            ImGui::Text("GPU: %s", (const char*)glGetString(GL_RENDERER));
            ImGui::Text("API: %s", (const char*)glGetString(GL_VERSION));
            ImGui::TextDisabled("Display & Quality:");
            ImGui::Text("Screen: %s (F11)", isFullscreen ? "Fullscreen" : "Windowed");
            ImGui::Text("Preset: %s", qualityPreset == QUALITY_LOW ? "Low" :
                                      qualityPreset == QUALITY_MEDIUM ? "Medium" :
                                      qualityPreset == QUALITY_HIGH ? "High" : "Ultra");
            ImGui::Text("Simulation: %.2fx (Day %.0f)", timeMultiplier, elapsedSimDays);

            if (asteroidBelt) {
                ImGui::Spacing();
                ImGui::Separator();
                const auto& astTelem = asteroidBelt->getTelemetry();
                ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "Asteroid Pipeline 2.0 (Instanced):");
                ImGui::Text("Backend: %s", astTelem.backendName.c_str());
                ImGui::Text("Active: %d (Draw Calls: %d)", astTelem.activeAsteroids, astTelem.asteroidDrawCalls);
                ImGui::BulletText("CPU Orbit Math: %.3f ms", astTelem.cpuUpdateTimeMs);
                ImGui::BulletText("Instance Upload: %.3f ms", astTelem.instanceUploadMs);
                ImGui::BulletText("Fence Wait: %.3f ms", astTelem.instanceFenceWaitMs);
                ImGui::BulletText("Render Submit: %.3f ms", astTelem.renderSubmitMs);
                ImGui::BulletText("Total Subsystem: %.3f ms", astTelem.totalUpdateMs);
                ImGui::BulletText("GPU Readback: %.3f ms (Zero-Readback)", astTelem.gpuSyncReadbackMs);
            }

            ImGui::Spacing();
            ImGui::Separator();
            lod::LODManager& lodMgr = lod::LODManager::instance();
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), "Level-of-Detail (LOD):");
            ImGui::Text("Mode: %s", !enableMeshLOD ? "Disabled (LOD 0)" :
                                   lodOverrideMode == 0 ? "Auto (Dynamic Distance)" :
                                   lodOverrideMode == 1 ? "Forced Ultra (LOD 0)" :
                                   lodOverrideMode == 2 ? "Forced High (LOD 1)" :
                                   lodOverrideMode == 3 ? "Forced Med (LOD 2)" : "Forced Low (LOD 3)");
            ImGui::Text("Drawn: %d tris", lodMgr.getRenderedTrianglesThisFrame());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.55f, 1.0f), "(Saved %d tris)", lodMgr.getSavedTrianglesThisFrame());

            const int* astCounts = lodMgr.getAsteroidTierCounts();
            ImGui::Text("Belt: %d H | %d M | %d L", astCounts[0], astCounts[1], astCounts[2]);

            if (showLODDebugTelemetry && !lodMgr.getBodyTelemetry().empty()) {
                ImGui::Spacing();
                if (ImGui::BeginTable("LODTelemetryTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn("Body", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("Dist", ImGuiTableColumnFlags_WidthFixed, 55.0f);
                    ImGui::TableSetupColumn("Tier", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    for (const auto& telem : lodMgr.getBodyTelemetry()) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%s", telem.name.c_str());
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.1f", telem.distance);
                        ImGui::TableSetColumnIndex(2);
                        glm::vec3 col = lod::LODManager::getTierDebugColor(telem.activeTier);
                        ImGui::TextColored(ImVec4(col.r, col.g, col.b, 1.0f), "%s", lod::LODManager::getSphereTierName(telem.activeTier));
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
    }

    // Spaceship Flight HUD
void SolarOdysseyUI::renderSpaceshipHUD(float screenWidth, float screenHeight, Spaceship& ship, CameraController& cam, const CelestialDatabase& db) {
        if (!ship.active || cam.photoModeActive) return;

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        float cx = screenWidth * 0.5f;
        float cy = screenHeight * 0.5f;

        // 1. Center Flight Crosshair & Reticle
        ImU32 reticleCol = ship.isBoosting ? IM_COL32(80, 210, 255, 220) : IM_COL32(70, 180, 240, 180);
        float reticleSize = 22.0f;
        float gap = 6.0f;

        // Center dot
        drawList->AddCircleFilled(ImVec2(cx, cy), 2.5f, reticleCol);

        // Reticle brackets [ + ]
        drawList->AddLine(ImVec2(cx - reticleSize, cy), ImVec2(cx - gap, cy), reticleCol, 1.5f);
        drawList->AddLine(ImVec2(cx + gap, cy), ImVec2(cx + reticleSize, cy), reticleCol, 1.5f);
        drawList->AddLine(ImVec2(cx, cy - reticleSize), ImVec2(cx, cy - gap), reticleCol, 1.5f);
        drawList->AddLine(ImVec2(cx, cy + gap), ImVec2(cx, cy + reticleSize), reticleCol, 1.5f);

        // Pitch & Roll artificial horizon markers in Cockpit view
        if (ship.cameraView == SHIP_CAM_COCKPIT) {
            ImU32 horizonCol = IM_COL32(60, 160, 230, 90);
            drawList->AddLine(ImVec2(cx - 120.0f, cy), ImVec2(cx - 40.0f, cy), horizonCol, 1.2f);
            drawList->AddLine(ImVec2(cx + 40.0f, cy), ImVec2(cx + 120.0f, cy), horizonCol, 1.2f);
            drawList->AddLine(ImVec2(cx - 80.0f, cy - 40.0f), ImVec2(cx - 50.0f, cy - 40.0f), horizonCol, 1.0f);
            drawList->AddLine(ImVec2(cx + 50.0f, cy - 40.0f), ImVec2(cx + 80.0f, cy - 40.0f), horizonCol, 1.0f);
            drawList->AddLine(ImVec2(cx - 80.0f, cy + 40.0f), ImVec2(cx - 50.0f, cy + 40.0f), horizonCol, 1.0f);
            drawList->AddLine(ImVec2(cx + 50.0f, cy + 40.0f), ImVec2(cx + 80.0f, cy + 40.0f), horizonCol, 1.0f);
        }

        // 2. Flight Telemetry Card (Bottom Left)
        float cardW = 320.0f;
        float cardH = 145.0f;
        ImGui::SetNextWindowPos(ImVec2(24.0f, screenHeight - cardH - 24.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(cardW, cardH), ImGuiCond_Always);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("SpaceshipTelemetry", nullptr, flags)) {
            // Flight Mode Badge
            if (ship.flightMode == FLIGHT_AUTOPILOT) {
                ImGui::TextColored(ImVec4(0.35f, 0.95f, 0.45f, 1.0f), " FLIGHT MODE: AUTOPILOT INTERCEPT");
            } else if (ship.flightMode == FLIGHT_ORBIT_ASSIST) {
                ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.55f, 1.0f), " FLIGHT MODE: ORBIT ASSIST");
            } else {
                ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.00f, 1.0f), " FLIGHT MODE: 6-DOF MANUAL");
            }
            ImGui::Separator();

            // Speedometer
            float spdKmh = ship.getSpeedKmh();
            ImGui::Text("VELOCITY: %.0f km/s", spdKmh);
            
            // Throttle meter
            float throttleVal = std::max(0.0f, ship.currentThrottle);
            char throttleText[32];
            snprintf(throttleText, sizeof(throttleText), "THROTTLE: %.0f%%", throttleVal * 100.0f);
            ImGui::ProgressBar(throttleVal, ImVec2(-1, 14.0f), throttleText);

            // Warp Boost meter
            float boostRatio = ship.boostEnergy / ship.maxBoostEnergy;
            char boostText[32];
            snprintf(boostText, sizeof(boostText), "WARP BOOST: %.0f%%", boostRatio * 100.0f);
            if (ship.isBoosting) {
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.85f, 1.0f, 1.0f));
            } else {
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.18f, 0.55f, 0.85f, 0.8f));
            }
            ImGui::ProgressBar(boostRatio, ImVec2(-1, 14.0f), boostText);
            ImGui::PopStyleColor();
        }
        ImGui::End();

        // 3. Navigation Target Card (Top Right)
        float navW = 360.0f;
        float navH = 175.0f;
        ImGui::SetNextWindowPos(ImVec2(screenWidth - navW - 24.0f, 80.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(navW, navH), ImGuiCond_Always);

        if (ImGui::Begin("SpaceshipNavTarget", nullptr, flags)) {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.45f, 1.0f), " TARGET: %s", ship.targetPlanetName.c_str());
            ImGui::Separator();
            ImGui::Text("Distance: %.2f AU (%.1f M km)", ship.targetDistance * 0.033f, ship.targetDistance * 4.95f);
            ImGui::Text("Closest Body: %s (%.1f units)", ship.nearestPlanetName.c_str(), ship.nearestPlanetDist);

            ImGui::Spacing();

            // Autopilot Button
            if (ship.flightMode == FLIGHT_AUTOPILOT) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.75f, 0.45f, 0.95f));
                if (ImGui::Button(" Cancel Autopilot (J)", ImVec2(165, 26))) {
                    // Route through toggleAutopilot so warp state is properly cancelled
                    // (direct flightMode writes leave warpSystem active and fighting MANUAL)
                    ship.toggleAutopilot();
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Autopilot / Warp (J)", ImVec2(165, 26))) {
                    ship.toggleAutopilot();
                }
            }

            ImGui::SameLine(0, 8);
            // Orbit Assist Button
            if (ship.flightMode == FLIGHT_ORBIT_ASSIST) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.65f, 0.95f, 0.95f));
                if (ImGui::Button(" Disengage (H)", ImVec2(165, 26))) {
                    // Route through toggleOrbitAssist for consistent state cleanup
                    ship.toggleOrbitAssist();
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Orbit Assist (H)", ImVec2(165, 26))) {
                    ship.toggleOrbitAssist();
                }
            }

            // Quick Target Selectors
            ImGui::Spacing();
            if (ImGui::Button("Target Black Hole", ImVec2(165, 22))) {
                if (onSelectBody) onSelectBody("Black Hole");
            }
            ImGui::SameLine(0, 8);
            if (ImGui::Button("Target Earth", ImVec2(165, 22))) {
                if (onSelectBody) onSelectBody("Earth");
            }
        }
        ImGui::End();

        // 4. Proximity Warning Banner (Top Center)
        if (ship.proximityAlertActive) {
            float alertW = 440.0f;
            float alertH = 42.0f;
            ImGui::SetNextWindowPos(ImVec2((screenWidth - alertW) * 0.5f, 80.0f), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(alertW, alertH), ImGuiCond_Always);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.35f, 0.08f, 0.08f, 0.90f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.35f, 0.25f, 1.0f));
            if (ImGui::Begin("ProximityWarning", nullptr, flags)) {
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), " WARNING: CLOSE PROXIMITY TO %s", ship.nearestPlanetName.c_str());
            }
            ImGui::End();
            ImGui::PopStyleColor(2);
        }

        // 5. Controls Helper (Bottom Right)
        float helpW = 340.0f;
        float helpH = 170.0f;
        ImGui::SetNextWindowPos(ImVec2(screenWidth - helpW - 24.0f, screenHeight - helpH - 24.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(helpW, helpH), ImGuiCond_Always);

        if (ImGui::Begin("SpaceshipHelp", nullptr, flags)) {
            ImGui::TextColored(ImVec4(0.65f, 0.80f, 0.95f, 1.0f), " FLIGHT CONTROLS");
            ImGui::Separator();
            ImGui::Text("W/S: Thrust / Brake  |  Shift: Boost");
            ImGui::Text("A/D: Yaw  |  Q/E: Roll  |  R/F: Pitch");
            ImGui::Text("J: Warp / Autopilot  |  H: Orbit Assist");
            const char* camName = (ship.cameraView == SHIP_CAM_CHASE) ? "CHASE" :
                                  (ship.cameraView == SHIP_CAM_CLOSE) ? "CLOSE" : "COCKPIT";
            ImGui::Text("C: View [%s]  |  X: Exit  |  Esc: Pause", camName);
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.45f, 0.95f), "Hold [Alt] to release cursor for UI");
        }
        ImGui::End();
    }

void SolarOdysseyUI::showToast(const std::string& title, const std::string& message, float duration) {
    if (duration <= 0 || (title.empty() && message.empty())) { toasts.clear(); return; }
    for (const auto& toast : toasts) if (toast.title == title && toast.message == message) return;
    if (toasts.size() >= 8) toasts.pop_front();
    toasts.push_back({title, message, std::clamp(duration, 2.0f, 5.0f)});
}

void SolarOdysseyUI::updateNotifications(float deltaTime) {
    const float elapsed = std::isfinite(deltaTime) ? std::max(0.0f, deltaTime) : 0.0f;
    saveStatusToastTimer = std::max(0.0f, saveStatusToastTimer - elapsed);
    for (size_t i = 0; i < std::min<size_t>(2, toasts.size()); ++i) toasts[i].remaining -= elapsed;
    toasts.erase(std::remove_if(toasts.begin(), toasts.end(), [](const Toast& t) { return t.remaining <= 0; }), toasts.end());
}

void SolarOdysseyUI::renderNotificationToast(bool flightHUD) {
    const auto display = ImGui::GetIO().DisplaySize;
    const float width = std::min(370.0f, display.x - 32);
    for (size_t i = 0; i < std::min<size_t>(2, toasts.size()); ++i) {
        ImGui::SetNextWindowPos(ImVec2(20, display.y - (flightHUD ? 268 : 168) - float(i) * 78), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(width, 72), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(.88f);
        const std::string id = "##ObservationToast" + std::to_string(i);
        if (ImGui::Begin(id.c_str(), nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::TextColored(ImVec4(.65f,.82f,.9f,1), "%s", toasts[i].title.c_str());
            ImGui::TextWrapped("%s", toasts[i].message.c_str());
        }
        ImGui::End();
    }
}

void SolarOdysseyUI::renderSaveStatusToast(float screenWidth, float screenHeight) {
    (void)screenHeight;
    if (saveStatusToastTimer <= 0.0f || saveStatusToast.empty()) return;

    float toastWidth = 440.0f;
    ImGui::SetNextWindowPos(ImVec2((screenWidth - toastWidth) * 0.5f, screenHeight - 124.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(toastWidth, 40.0f), ImGuiCond_Always);
    ImGuiWindowFlags toastFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
    if (ImGui::Begin("SaveStatusToast", nullptr, toastFlags)) {
        ImGui::TextColored(ImVec4(.65f,.82f,.9f,1), " [State] %s", saveStatusToast.c_str());
    }
    ImGui::End();
}

namespace {

// Cycle 4 Pass 1: Codex status chip color (functional version).
ImVec4 codexStatusColor(DiscoveryStatus s) {
    switch (s) {
        case DiscoveryStatus::Detected: return ImVec4(0.55f, 0.85f, 0.55f, 1.0f);
        case DiscoveryStatus::Visited: return ImVec4(0.45f, 0.75f, 1.0f, 1.0f);
        case DiscoveryStatus::Scanned: return ImVec4(0.65f, 0.55f, 1.0f, 1.0f);
        case DiscoveryStatus::FullySurveyed: return ImVec4(1.0f, 0.82f, 0.35f, 1.0f);
        case DiscoveryStatus::Unknown:
        default: return ImVec4(0.55f, 0.55f, 0.60f, 1.0f);
    }
}

const char* codexActivityMark(bool done) {
    return done ? "[x]" : "[ ]";
}

} // namespace

void SolarOdysseyUI::renderCodex(float screenWidth, float screenHeight, const CelestialDatabase& db,
                                 const ScienceProgression& prog,
                                 const std::vector<std::string>& roster) {
    if (!showCodex) return;

    const float panelW = 560.0f;
    const float panelH = 560.0f;
    ImGui::SetNextWindowPos(ImVec2((screenWidth - panelW) * 0.5f, (screenHeight - panelH) * 0.5f),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(panelW, panelH), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin("DISCOVERY CODEX", &showCodex, flags)) {
        ImGui::End();
        return;
    }

    ImGui::SetNextItemWidth(220.0f);
    ImGui::InputText("Search", codexSearch, sizeof(codexSearch));
    ImGui::SameLine(0, 12);
    ImGui::SetNextItemWidth(150.0f);
    const char* statusNames[] = {"All statuses", "Unknown", "Detected", "Visited", "Scanned", "Fully Surveyed"};
    ImGui::Combo("Status", &codexStatusFilter, statusNames, 6);

    ImGui::Separator();
    ImGui::BeginChild("CodexListScroll", ImVec2(0, 0), true);

    const std::string needle(codexSearch);
    for (const auto& name : roster) {
        const DiscoveryRecord* rec = prog.getRecord(name);
        const DiscoveryStatus status =
            rec ? rec->status : DiscoveryStatus::Unknown;
        // Status filter (0 = All, else DiscoveryStatus+1).
        if (codexStatusFilter != 0 && static_cast<int>(status) != codexStatusFilter - 1) continue;

        const bool known = (status != DiscoveryStatus::Unknown);
        const std::string shownName = known ? name : "Undiscovered body";
        // Text search matches the visible name (unknown rows match nothing
        // except an empty query — no spoiler leakage through search).
        if (!needle.empty() && shownName.find(needle) == std::string::npos) continue;

        ImGui::PushID(name.c_str());
        ImGui::TextColored(codexStatusColor(status), "[%s]", discoveryStatusLabel(status));
        ImGui::SameLine(0, 8);
        const CelestialBodyData* data = known ? db.getBody(name) : nullptr;
        bool open = false;
        if (known && data != nullptr) {
            open = ImGui::CollapsingHeader(shownName.c_str());
        } else {
            ImGui::TextDisabled("%s", shownName.c_str());
        }

        if (open && data != nullptr) {
            // Progressive disclosure: higher tiers reveal more database
            // science. No per-body branches — field groups by status only.
            ImGui::TextDisabled("%s", data->type.c_str());
            if (status >= DiscoveryStatus::Visited) {
                ImGui::Text("Diameter: %.1f km  |  Gravity: %.3f m/s^2  |  Period: %.2f d",
                            data->realDiameterKm, data->surfaceGravityMs2,
                            data->orbitalPeriodDays);
                ImGui::TextWrapped("%s", data->subtitle.c_str());
            }
            if (status >= DiscoveryStatus::Scanned) {
                ImGui::Spacing();
                ImGui::TextWrapped("%s", data->description.c_str());
                for (const auto& fact : data->keyFacts) {
                    ImGui::BulletText("%s", fact.c_str());
                }
            }
            if (status >= DiscoveryStatus::FullySurveyed) {
                ImGui::Spacing();
                if (data->hasMeanTemperatureData) ImGui::Text("%s: %.1f C", data->temperatureReference.c_str(), data->meanTemperatureC);
                else ImGui::TextDisabled("Temperature: unavailable");
                if (data->hasTemperatureRangeData) ImGui::Text("Range: %.1f / %.1f C", data->minTemperatureC, data->maxTemperatureC);
                else ImGui::TextDisabled("Temperature range: unavailable");
                ImGui::TextWrapped("Air: %s", data->atmosphericComposition.c_str());
                ImGui::TextWrapped("Surface: %s", data->surfaceFeatures.c_str());
                ImGui::TextWrapped("Discovery: %s", data->discoveryInfo.c_str());
            }
            ImGui::Spacing();
            ImGui::Separator();
            // Survey record (always visible once the identity is known).
            ImGui::TextDisabled("Survey record:");
            if (rec != nullptr) {
                ImGui::Text("Visits: %d   |   Best photo: %s", rec->visitCount,
                            rec->bestPhotoScore > 0.0f
                                ? (std::to_string(static_cast<int>(rec->bestPhotoScore))).c_str()
                                : "--");
                ImGui::Text("%s Orbital survey   %s Atmospheric scan%s   %s Close flyby   %s Gravity",
                            codexActivityMark(rec->orbitalSurveyCompleted),
                            codexActivityMark(rec->atmosphericScanCompleted),
                            rec->atmosphereApplicable ? "" : "(N/A airless)",
                            codexActivityMark(rec->closeFlybyCompleted),
                            codexActivityMark(rec->gravityMeasurementCompleted));
                if (!rec->anomalyIds.empty()) {
                    ImGui::Text("Anomalies (%d):", static_cast<int>(rec->anomalyIds.size()));
                    for (const auto& aid : rec->anomalyIds) {
                        // Catalog text when known; the raw id otherwise (ids
                        // are stable keys, never prose to parse).
                        if (const AnomalyDef* def = findAnomaly(aid)) {
                            ImGui::BulletText("%s", def->displayName.c_str());
                            ImGui::TextWrapped("%s", def->codexText.c_str());
                        } else {
                            ImGui::BulletText("%s", aid.c_str());
                        }
                    }
                } else {
                    ImGui::TextDisabled("Anomalies: none recorded");
                }
            }
            ImGui::Separator();
        }
        ImGui::PopID();
    }

    // Non-roster discoveries (e.g. Black Hole / Wormhole anomaly contexts):
    // records exist without a runtime-body roster slot. Listed generically
    // from the progression map — never a second hardcoded body list.
    bool otherHeaderShown = false;
    for (const auto& kv : prog.allRecords()) {
        bool inRoster = false;
        for (const auto& r : roster) {
            if (r == kv.first) { inRoster = true; break; }
        }
        if (inRoster) continue;
        const DiscoveryRecord& rec = kv.second;
        if (rec.anomalyIds.empty() && rec.status == DiscoveryStatus::Unknown) continue;
        if (!otherHeaderShown) {
            otherHeaderShown = true;
            ImGui::Separator();
            ImGui::TextDisabled("Other discoveries:");
        }
        ImGui::PushID(kv.first.c_str());
        ImGui::TextColored(codexStatusColor(rec.status), "[%s]", discoveryStatusLabel(rec.status));
        ImGui::SameLine(0, 8);
        ImGui::Text("%s", kv.first.c_str());
        for (const auto& aid : rec.anomalyIds) {
            if (const AnomalyDef* def = findAnomaly(aid)) {
                ImGui::BulletText("%s", def->displayName.c_str());
            } else {
                ImGui::BulletText("%s", aid.c_str());
            }
        }
        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::End();
}

