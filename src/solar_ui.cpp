#include "solar_ui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

SolarOdysseyUI::SolarOdysseyUI() {}

void SolarOdysseyUI::applySpaceTheme() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        style.WindowRounding = 9.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        style.WindowBorderSize = 1.0f;
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

        ImDrawList* drawList = ImGui::GetBackgroundDrawList();

        for (const auto& body : bodies) {
            glm::vec2 screenPos;
            float distToCam;
            // Position label slightly above the planet top
            glm::vec3 labelWorldPos = body.position + glm::vec3(0.0f, body.radius * 1.35f + 0.25f, 0.0f);

            if (projectWorldToScreen(labelWorldPos, viewMatrix, projMatrix, screenWidth, screenHeight, screenPos, distToCam)) {
                // Distance fade calculation (fades gently if too far)
                float alpha = 1.0f;
                if (distToCam > 120.0f) {
                    alpha = std::max(0.0f, 1.0f - (distToCam - 120.0f) / 100.0f);
                }
                if (alpha <= 0.05f) continue;

                const CelestialBodyData* data = db.getBody(body.name);
                ImU32 accentCol = data ? IM_COL32((int)(data->themeColor.r * 255),
                                                  (int)(data->themeColor.g * 255),
                                                  (int)(data->themeColor.b * 255),
                                                  (int)(alpha * 255))
                                       : IM_COL32(200, 220, 255, (int)(alpha * 255));

                bool isSelected = (cam.focusedBodyName == body.name || selectedPlanetName == body.name);

                // Label Text (body.name is already a std::string — no copy needed)
                const char* labelText = body.name.c_str();
                ImVec2 textSize = ImGui::CalcTextSize(labelText);

                // Badge background rectangle
                float padX = 8.0f;
                float padY = 4.0f;
                ImVec2 minPt(screenPos.x - textSize.x * 0.5f - padX, screenPos.y - textSize.y * 0.5f - padY);
                ImVec2 maxPt(screenPos.x + textSize.x * 0.5f + padX, screenPos.y + textSize.y * 0.5f + padY);

                // Badge background
                ImU32 bgCol = isSelected ? IM_COL32(25, 45, 75, (int)(alpha * 220))
                                         : IM_COL32(12, 16, 25, (int)(alpha * 190));
                ImU32 borderCol = isSelected ? accentCol : IM_COL32(70, 95, 130, (int)(alpha * 120));

                drawList->AddRectFilled(minPt, maxPt, bgCol, 5.0f);
                drawList->AddRect(minPt, maxPt, borderCol, 5.0f, 0, isSelected ? 2.0f : 1.0f);

                // Planet color accent dot
                drawList->AddCircleFilled(ImVec2(minPt.x + 5.0f, screenPos.y), 3.0f, accentCol);

                // Text
                ImU32 textCol = isSelected ? IM_COL32(255, 255, 255, (int)(alpha * 255))
                                           : IM_COL32(220, 230, 245, (int)(alpha * 230));
                drawList->AddText(ImVec2(minPt.x + 12.0f, minPt.y + padY - 1.0f), textCol, labelText);

                // Subtle, refined selection reticle (18% outside projected radius, thin stroke, lower alpha)
                if (isSelected) {
                    glm::vec2 bodyCenterScreen;
                    float dummyDist;
                    if (projectWorldToScreen(body.position, viewMatrix, projMatrix, screenWidth, screenHeight, bodyCenterScreen, dummyDist)) {
                        float ringRadius = std::max(14.0f, (body.radius / distToCam) * screenHeight * 1.18f);
                        ImU32 reticleCol = data ? IM_COL32((int)(data->themeColor.r * 255), (int)(data->themeColor.g * 255), (int)(data->themeColor.b * 255), (int)(alpha * 150))
                                                : IM_COL32(200, 220, 255, (int)(alpha * 150));
                        drawList->AddCircle(ImVec2(bodyCenterScreen.x, bodyCenterScreen.y), ringRadius, reticleCol, 48, 1.0f);
                    }
                }
            }
        }
    }

    // Top Navigation & Quick Select Bar
void SolarOdysseyUI::renderTopNavBar(float screenWidth, CameraController& cam, const CelestialDatabase& db,
                                     std::vector<std::pair<std::string, int>>& planetIndexMap) {
        (void)planetIndexMap;
        if (cam.photoModeActive) return;

        float minSingleLineWidth = 1180.0f;
        bool isCompactTwoRows = (screenWidth < minSingleLineWidth);
        float barHeight = isCompactTwoRows ? 88.0f : 54.0f;

        ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(screenWidth - 32, barHeight), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("TopNavBar", nullptr, flags)) {
            // App Title with glowing cyan accent
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.00f, 1.0f), " SOLAR ODYSSEY");
            ImGui::SameLine(0, 16);
            ImGui::TextDisabled("|");
            ImGui::SameLine(0, 12);

            // Celestial Body Dropdown Quick Selector (Handles 13+ bodies cleanly with zero overflow)
            std::string previewText = "Select Body ▾";
            if (!cam.focusedBodyName.empty()) {
                previewText = " " + cam.focusedBodyName + " ▾";
            }
            ImGui::SetNextItemWidth(140.0f);
            const auto& order = db.getOrder();
            if (ImGui::BeginCombo("##CelestialBodyCombo", previewText.c_str())) {
                for (size_t i = 0; i < order.size(); ++i) {
                    const std::string& name = order[i];
                    const CelestialBodyData* data = db.getBody(name);
                    if (!showDwarfPlanets && data && data->type.find("Dwarf Planet") != std::string::npos) {
                        continue;
                    }
                    bool isSelected = (cam.focusedBodyName == name);
                    if (ImGui::Selectable(name.c_str(), isSelected)) {
                        selectedPlanetName = name;
                        showPlanetCard = true;
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Dynamically calculate the precise pixel width of all right-aligned action buttons
            float btnSpacing = 5.0f;
            float pad = ImGui::GetStyle().FramePadding.x * 2.0f;
            float actionsWidth =
                (ImGui::CalcTextSize(" Free Cam (F) ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Missions (M) ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Wormhole (K) ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Black Hole (B) ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Spaceship (X) ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Tour ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Reset ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Settings ").x + pad) + btnSpacing +
                (ImGui::CalcTextSize(" Photo ").x + pad);

            float cursorX = ImGui::GetCursorPosX();
            float contentWidth = ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 2.0f;
            float targetX = contentWidth - actionsWidth + ImGui::GetStyle().WindowPadding.x;

            if (!isCompactTwoRows && targetX > cursorX + 16.0f) {
                ImGui::SameLine(targetX);
            } else {
                ImGui::SameLine(0, 10.0f);
            }

            // Free Camera Button
            if (cam.mode == CAM_FREE) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.85f, 0.65f, 0.95f));
                if (ImGui::Button(" Free Cam (F) ")) {
                    cam.toggleFreeCam();
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Free Cam (F) ")) {
                    cam.toggleFreeCam();
                }
            }

            ImGui::SameLine(0, btnSpacing);
            // Missions Button
            if (showMissionModal) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.70f, 0.45f, 0.95f));
                if (ImGui::Button(" Missions (M) ")) {
                    showMissionModal = false;
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Missions (M) ")) {
                    showMissionModal = true;
                }
            }

            ImGui::SameLine(0, btnSpacing);
            // Wormhole Mode Button
            if (cam.mode == CAM_WORMHOLE || selectedPlanetName == "Wormhole") {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.75f, 0.95f, 0.95f));
                if (ImGui::Button(" Wormhole (K) ")) {
                    cam.resetToDefault();
                    selectedPlanetName = "";
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Wormhole (K) ")) {
                    selectedPlanetName = "Wormhole";
                    showPlanetCard = true;
                }
            }

            ImGui::SameLine(0, btnSpacing);
            // Black Hole Mode Button
            if (cam.mode == CAM_BLACK_HOLE || selectedPlanetName == "Black Hole") {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.22f, 0.85f, 0.95f));
                if (ImGui::Button(" Black Hole (B) ")) {
                    cam.resetToDefault();
                    selectedPlanetName = "";
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Black Hole (B) ")) {
                    selectedPlanetName = "Black Hole";
                    showPlanetCard = true;
                }
            }

            ImGui::SameLine(0, btnSpacing);
            // Spaceship Flight Mode Button
            if (cam.mode == CAM_SPACESHIP) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.65f, 0.95f, 0.95f));
                if (ImGui::Button(" Spaceship (X) ")) {
                    cam.setSpaceshipMode(false, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Spaceship (X) ")) {
                    cam.mode = CAM_SPACESHIP;
                }
            }

            ImGui::SameLine(0, btnSpacing);
            // Guided Tour Button
            if (cam.tourActive) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.35f, 0.25f, 0.90f));
                if (ImGui::Button(" Stop Tour ")) {
                    cam.stopTour();
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button(" Tour ")) {
                    cam.startTour();
                }
            }

            ImGui::SameLine(0, btnSpacing);
            if (ImGui::Button(" Reset ")) {
                cam.resetToDefault();
                selectedPlanetName = "";
            }

            ImGui::SameLine(0, btnSpacing);
            if (ImGui::Button(" Settings ")) {
                showSettingsModal = !showSettingsModal;
            }

            ImGui::SameLine(0, btnSpacing);
            if (ImGui::Button(" Photo ")) {
                cam.togglePhotoMode();
            }
        }
        ImGui::End();
    }

    // Bottom Simulation & Playback Control Bar
void SolarOdysseyUI::renderBottomControlBar(float screenWidth, float screenHeight, CameraController& cam) {
        if (cam.photoModeActive || cam.mode == CAM_SPACESHIP) return;

        float barWidth = 720.0f;
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
        }
        ImGui::End();
    }

    // Floating Planetary Dossier Card
void SolarOdysseyUI::renderPlanetCard(float screenWidth, float screenHeight, const CelestialDatabase& db,
                                      CameraController& cam,
                                      std::function<void(const std::string&)> onFocus,
                                      std::function<void(const std::string&)> onExplorePOV) {
        if (!showPlanetCard || selectedPlanetName.empty() || cam.photoModeActive || cam.mode == CAM_SPACESHIP) return;

        const CelestialBodyData* data = db.getBody(selectedPlanetName);
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
            ImGui::Separator();

            // Quick Actions: Focus Camera & Explore POV
            if (ImGui::Button(" Focus Camera", ImVec2(185, 32))) {
                if (onFocus) onFocus(data->name);
            }
            ImGui::SameLine(0, 10);
            if (ImGui::Button(" Explore POV", ImVec2(185, 32))) {
                if (onExplorePOV) onExplorePOV(data->name);
            }

            ImGui::Spacing();
            if (ImGui::BeginTabBar("PlanetInfoTabs")) {
                // Tab 1: Overview & Metrics
                if (ImGui::BeginTabItem("Overview")) {
                    ImGui::Spacing();
                    ImGui::TextWrapped("%s", data->description.c_str());
                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::Columns(2, "MetricColumns", false);
                    ImGui::SetColumnWidth(0, 180);

                    ImGui::TextDisabled("Physical Diameter:");
                    ImGui::NextColumn();
                    ImGui::Text("%.1f km (%.2fx Earth)", data->realDiameterKm, data->relativeSizeToEarth);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Distance from Sun:");
                    ImGui::NextColumn();
                    ImGui::Text("%.2f AU (%.1f M km)", data->distanceFromSunAU, data->distanceFromSunMillionKm);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Orbital Period:");
                    ImGui::NextColumn();
                    ImGui::Text("%.1f Earth days", data->orbitalPeriodDays);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Rotation Period:");
                    ImGui::NextColumn();
                    ImGui::Text("%.2f hours", data->rotationPeriodHours);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Confirmed Moons:");
                    ImGui::NextColumn();
                    ImGui::Text("%d", data->knownMoons);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Surface Gravity:");
                    ImGui::NextColumn();
                    ImGui::Text("%.2f m/s^2", data->surfaceGravityMs2);
                    ImGui::NextColumn();

                    ImGui::TextDisabled("Mean Temperature:");
                    ImGui::NextColumn();
                    ImGui::Text("%.1f deg C", data->meanTemperatureC);
                    ImGui::NextColumn();

                    ImGui::Columns(1);
                    ImGui::EndTabItem();
                }

                // Tab 2: Atmosphere & Geology
                if (ImGui::BeginTabItem("Environment")) {
                    ImGui::Spacing();
                    ImGui::TextColored(themeCol, "Atmospheric Composition:");
                    ImGui::TextWrapped("%s", data->atmosphericComposition.c_str());

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(themeCol, "Surface & Geological Features:");
                    ImGui::TextWrapped("%s", data->surfaceFeatures.c_str());

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(themeCol, "Historical Discovery:");
                    ImGui::TextWrapped("%s", data->discoveryInfo.c_str());
                    ImGui::EndTabItem();
                }

                // Tab 3: Key Scientific Facts
                if (ImGui::BeginTabItem("Key Facts")) {
                    ImGui::Spacing();
                    for (size_t i = 0; i < data->keyFacts.size(); ++i) {
                        ImGui::Bullet();
                        ImGui::TextWrapped("%s", data->keyFacts[i].c_str());
                        ImGui::Spacing();
                    }
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
                                          std::function<void(const std::string&)> onExplorePOV) {
        renderPlanetCard(screenWidth, screenHeight, db, cam, onFocus, onExplorePOV);
    }

    // Settings & Display Layers Modal / Panel
void SolarOdysseyUI::renderSettingsPanel(PostProcessingPipeline& postProc, AsteroidBelt* asteroidBelt,
                                         AtmosphereEffects* atmoEffects, CameraController& cam) {
        if (!showSettingsModal) return;

        ImGui::SetNextWindowSize(ImVec2(560, 620), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Simulation & Display Settings", &showSettingsModal)) {
            if (ImGui::BeginTabBar("SettingsTabs")) {
                // Tab 1: Planetary Simulation Settings
                if (ImGui::BeginTabItem("Planetary Simulation")) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), " Celestial Bodies & Orbit Physics");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::SliderFloat("Visual Planet Scale", &planetScale, 0.25f, 3.50f, "%.2fx");
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Multiplies the render size of all planets and moons.");

                    ImGui::SliderFloat("Orbit Revolution Speed", &orbitSpeedScale, 0.0f, 10.0f, "%.2fx");
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scales the orbital speed around the Sun (0 = paused orbits).");

                    ImGui::SliderFloat("Axial Spin Speed", &spinSpeedScale, 0.0f, 10.0f, "%.2fx");
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scales the rotation speed around each planet's own axis.");

                    ImGui::Checkbox("Physically Realistic Axial Tilt", &enableAxialTilt);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Applies real physical axial tilts (Earth 23.5°, Mars 25.2°, Saturn 26.7°, Uranus 97.8°).");

                    ImGui::Checkbox("Show Dwarf Planets (Ceres, Haumea, Makemake, Eris)", &showDwarfPlanets);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggles rendering of the 4 dwarf planets in the asteroid & Kuiper belts.");

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), " Atmospheric & Planetary Features");
                    ImGui::Spacing();

                    ImGui::SliderFloat("Atmosphere Glow Intensity", &atmosphereGlowScale, 0.0f, 3.0f, "%.2fx");
                    ImGui::SliderFloat("Saturn Ring Opacity", &ringOpacity, 0.1f, 1.0f, "%.2f");

                    ImGui::Spacing();
                    ImGui::Separator();
                    if (ImGui::Button("Reset Planetary Defaults", ImVec2(200, 26))) {
                        planetScale = 1.0f;
                        orbitSpeedScale = 1.0f;
                        spinSpeedScale = 1.0f;
                        enableAxialTilt = true;
                        atmosphereGlowScale = 1.0f;
                        ringOpacity = 0.90f;
                        showDwarfPlanets = true;
                    }
                    ImGui::EndTabItem();
                }

                // Tab 2: Free Camera Flight Controls
                if (ImGui::BeginTabItem("Free Camera")) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.65f, 1.0f), " 6-DOF Free Flight Tuning");
                    ImGui::Separator();
                    ImGui::Spacing();

                    ImGui::SliderFloat("Base Flight Speed", &cam.freeSpeed, 2.0f, 120.0f, "%.1f units/s");
                    ImGui::SliderFloat("Mouse Look Sensitivity", &cam.freeSensitivity, 0.02f, 0.35f, "%.3f");
                    ImGui::SliderFloat("Acceleration Factor", &cam.freeAcceleration, 2.0f, 20.0f, "%.1f");
                    ImGui::SliderFloat("Inertial Damping", &cam.freeDamping, 1.0f, 15.0f, "%.1f");
                    ImGui::SliderFloat("Shift Turbo Multiplier", &cam.freeSpeedBoost, 1.5f, 6.0f, "%.1fx");
                    ImGui::SliderFloat("Ctrl Precision Multiplier", &cam.freeSpeedSlow, 0.1f, 0.5f, "%.2fx");

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextDisabled("Free Camera Shortcuts:");
                    ImGui::BulletText("W / S : Forward / Backward");
                    ImGui::BulletText("A / D : Strafe Left / Right");
                    ImGui::BulletText("E / Space : Ascend Up");
                    ImGui::BulletText("Q / C : Descend Down");
                    ImGui::BulletText("Shift : 3.5x Turbo Boost  |  Ctrl : Precision Crawl");
                    ImGui::BulletText("Scroll Wheel : Live Adjust Flight Speed");

                    ImGui::Spacing();
                    if (cam.mode != CAM_FREE) {
                        if (ImGui::Button("Enter Free Flight Mode (F)", ImVec2(240, 28))) {
                            cam.enterFreeCam();
                        }
                    } else {
                        ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.55f, 1.0f), " Free Flight Active");
                    }
                    ImGui::EndTabItem();
                }

                // Tab 3: Visual Layers & Overlays
                if (ImGui::BeginTabItem("Display Layers")) {
                    ImGui::Spacing();
                    ImGui::Checkbox("Orbit Paths (O)", &showOrbits);
                    ImGui::Checkbox("Planet Labels (L)", &showLabels);
                    ImGui::Checkbox("Atmospheric Scattering", &showAtmospheres);
                    ImGui::Checkbox("Asteroid Belt", &showAsteroids);
                    ImGui::Checkbox("Solar Particles & Comets", &showParticles);
                    ImGui::Checkbox("Performance Diagnostics", &showDiagnostics);

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Cinematic Post-Processing:");
                    ImGui::Checkbox("Enable Post-Processing Pipeline", &postProc.enabled);
                    if (postProc.enabled) {
                        ImGui::Checkbox("Bloom Lighting & Corona", &postProc.bloomEnabled);
                        if (postProc.bloomEnabled) {
                            ImGui::SliderFloat("Bloom Intensity", &postProc.bloomIntensity, 0.1f, 1.2f);
                            ImGui::SliderFloat("Bloom Threshold", &postProc.bloomThreshold, 0.5f, 1.2f);
                        }
                        ImGui::Checkbox("ACES Filmic Tone Mapping", &postProc.toneMappingEnabled);
                        ImGui::SliderFloat("Exposure", &postProc.exposure, 0.5f, 2.5f);
                        ImGui::Checkbox("Vignette", &postProc.vignetteEnabled);
                    }
                    ImGui::EndTabItem();
                }

                // Tab 4: Graphics Quality Presets
                if (ImGui::BeginTabItem("Graphics Quality")) {
                    ImGui::Spacing();
                    ImGui::Text("Select Quality Preset:");

                    const char* presets[] = {"Low", "Medium", "High", "Ultra"};
                    int currPreset = (int)qualityPreset;
                    if (ImGui::Combo("Preset", &currPreset, presets, 4)) {
                        qualityPreset = (GraphicsQuality)currPreset;
                        applyQualityPreset(qualityPreset, postProc, asteroidBelt);
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::TextDisabled("Preset Details:");
                    if (qualityPreset == QUALITY_LOW) {
                        ImGui::BulletText("Asteroids: 150 count");
                        ImGui::BulletText("Bloom: Disabled");
                        ImGui::BulletText("Atmosphere: Low Tessellation");
                    } else if (qualityPreset == QUALITY_MEDIUM) {
                        ImGui::BulletText("Asteroids: 400 count");
                        ImGui::BulletText("Bloom: Standard 2-pass");
                        ImGui::BulletText("Atmosphere: Standard Multi-layer");
                    } else if (qualityPreset == QUALITY_HIGH) {
                        ImGui::BulletText("Asteroids: 800 count");
                        ImGui::BulletText("Bloom: High-Precision HDR");
                        ImGui::BulletText("Atmosphere: Full Rayleigh Scattering");
                    } else if (qualityPreset == QUALITY_ULTRA) {
                        ImGui::BulletText("Asteroids: 1400 count");
                        ImGui::BulletText("Bloom: Multi-pass Ultra");
                        ImGui::BulletText("Atmosphere: Enhanced Limb & Specular");
                    }
                    ImGui::EndTabItem();
                }

                // Tab 5: Audio Settings
                if (ImGui::BeginTabItem("Audio")) {
                    ImGui::Spacing();
                    ImGui::Checkbox("Mute All Audio", &audioMuted);
                    ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 1.0f);
                    ImGui::SliderFloat("Music Volume", &musicVolume, 0.0f, 1.0f);
                    ImGui::SliderFloat("Sound Effects Volume", &sfxVolume, 0.0f, 1.0f);
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
        }
        ImGui::End();
    }

    // Free Camera Live Flight Telemetry Overlay
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
                postProc.triggerScreenshot();
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
void SolarOdysseyUI::renderDiagnostics(float screenWidth) {
        if (!showDiagnostics) return;

        ImGui::SetNextWindowPos(ImVec2(16, 85), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 180), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Diagnostics", &showDiagnostics)) {
            ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Separator();
            ImGui::Text("OpenGL: %s", (const char*)glGetString(GL_VERSION));
            ImGui::Text("GPU: %s", (const char*)glGetString(GL_RENDERER));
            ImGui::Separator();
            ImGui::Text("Quality: %s", qualityPreset == QUALITY_LOW ? "Low" :
                                      qualityPreset == QUALITY_MEDIUM ? "Medium" :
                                      qualityPreset == QUALITY_HIGH ? "High" : "Ultra");
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
                selectedPlanetName = "Black Hole";
            }
            ImGui::SameLine(0, 8);
            if (ImGui::Button("Target Earth", ImVec2(165, 22))) {
                selectedPlanetName = "Earth";
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
        float helpH = 142.0f;
        ImGui::SetNextWindowPos(ImVec2(screenWidth - helpW - 24.0f, screenHeight - helpH - 24.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(helpW, helpH), ImGuiCond_Always);

        if (ImGui::Begin("SpaceshipHelp", nullptr, flags)) {
            ImGui::TextColored(ImVec4(0.65f, 0.80f, 0.95f, 1.0f), " FLIGHT CONTROLS");
            ImGui::Separator();
            ImGui::Text("W/S: Thrust / Brake    | Shift: Boost");
            ImGui::Text("A/D: Yaw    Q/E: Roll  | R/F: Pitch");
            ImGui::Text("J: Warp to Target      | H: Orbit Assist");
            const char* camName = (ship.cameraView == SHIP_CAM_CHASE) ? "CHASE" :
                                  (ship.cameraView == SHIP_CAM_CLOSE) ? "CLOSE" : "COCKPIT";
            ImGui::Text("C: Cam [%s]  | X: Exit Flight", camName);
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.45f, 0.95f), "Hold [Alt] to release cursor for UI");
        }
        ImGui::End();
    }

    // 6. Mission HUD Telemetry Tracker (Top Left)
void SolarOdysseyUI::renderMissionHUDTracker(float screenWidth, float screenHeight, MissionSystem& missions, Spaceship& ship, CameraController& cam) {
        if (cam.photoModeActive) return;

        Mission* activeM = missions.getActiveMission();
        if (!activeM) return;

        float trackerW = 340.0f;
        float trackerH = 92.0f;
        ImGui::SetNextWindowPos(ImVec2(24.0f, 80.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(trackerW, trackerH), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("MissionTrackerHUD", nullptr, flags)) {
            ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.55f, 1.0f), " ACTIVE MISSION [%s]", activeM->category.c_str());
            ImGui::Text("%s", activeM->title.c_str());

            char progText[32];
            snprintf(progText, sizeof(progText), "%.0f%% - %s", activeM->progress * 100.0f, activeM->targetName.c_str());
            ImGui::ProgressBar(activeM->progress, ImVec2(-1, 13.0f), progText);

            ImGui::TextDisabled("[M] Mission Log  |  [N] Next Mission");
        }
        ImGui::End();
    }

    // 7. Mission Completion Toast Notification Popup
void SolarOdysseyUI::renderMissionToast(float screenWidth, float screenHeight, MissionSystem& missions) {
        if (!missions.toast.active) return;

        float toastW = 480.0f;
        float toastH = 58.0f;
        float toastX = (screenWidth - toastW) * 0.5f;
        float toastY = 75.0f;

        ImGui::SetNextWindowPos(ImVec2(toastX, toastY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(toastW, toastH), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoSavedSettings;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.22f, 0.14f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.95f, 0.55f, 1.0f));

        if (ImGui::Begin("MissionToast", nullptr, flags)) {
            ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.65f, 1.0f), " %s", missions.toast.title.c_str());
            ImGui::Text("%s", missions.toast.message.c_str());
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
    }

    // 8. Full Mission Log Modal Dialog
void SolarOdysseyUI::renderMissionModal(float screenWidth, float screenHeight, MissionSystem& missions, Spaceship& ship, CameraController& cam) {
        if (!showMissionModal) return;

        float modalW = 680.0f;
        float modalH = 520.0f;
        ImGui::SetNextWindowPos(ImVec2((screenWidth - modalW) * 0.5f, (screenHeight - modalH) * 0.5f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(modalW, modalH), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        if (ImGui::Begin("INTERSTELLAR MISSION LOG", &showMissionModal, flags)) {
            // Overall Campaign Progress
            float compRate = missions.getOverallCompletionRate();
            char rateText[48];
            snprintf(rateText, sizeof(rateText), "CAMPAIGN PROGRESS: %.0f%%", compRate * 100.0f);
            ImGui::ProgressBar(compRate, ImVec2(-1, 18.0f), rateText);
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::BeginChild("MissionListScroll", ImVec2(0, 0), true);

            for (size_t i = 0; i < missions.missions.size(); ++i) {
                Mission& m = missions.missions[i];
                bool isActive = (missions.activeMissionIndex == (int)i);

                ImGui::PushID((int)i);

                if (m.isCompleted) {
                    ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.55f, 1.0f), " [COMPLETE] #%d: %s", m.id, m.title.c_str());
                } else if (isActive) {
                    ImGui::TextColored(ImVec4(0.35f, 0.85f, 1.0f, 1.0f), " [ACTIVE] #%d: %s", m.id, m.title.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), " [PENDING] #%d: %s", m.id, m.title.c_str());
                }

                ImGui::TextDisabled("Category: %s  |  Target: %s", m.category.c_str(), m.targetName.c_str());
                ImGui::TextWrapped("%s", m.description.c_str());

                char progLabel[32];
                snprintf(progLabel, sizeof(progLabel), "%.0f%%", m.progress * 100.0f);
                ImGui::ProgressBar(m.progress, ImVec2(340.0f, 14.0f), progLabel);

                ImGui::SameLine(0, 16);
                if (!isActive) {
                    if (ImGui::Button("Set Active", ImVec2(100, 22))) {
                        missions.activeMissionIndex = (int)i;
                    }
                } else {
                    ImGui::TextColored(ImVec4(0.25f, 0.95f, 0.55f, 1.0f), "Tracking");
                }

                ImGui::SameLine(0, 12);
                if (ImGui::Button("Target Destination", ImVec2(140, 22))) {
                    selectedPlanetName = m.targetName;
                    if (ship.active || cam.mode == CAM_SPACESHIP) {
                        ship.targetPlanetName = m.targetName;
                    }
                }

                ImGui::Separator();
                ImGui::Spacing();
                ImGui::PopID();
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

