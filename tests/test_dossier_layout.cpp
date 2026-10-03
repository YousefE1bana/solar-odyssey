#include "catch.hpp"
#include "solar_ui.h"
#include <imgui.h>
#include <imgui_internal.h>

TEST_CASE("Dossier metric cells remain inside the viewport at supported scales", "[ui][dossier]") {
    const auto screen = GENERATE(ImVec2(1280, 720), ImVec2(1600, 900), ImVec2(1920, 1080));
    const float scale = GENERATE(1.0f, 1.25f, 1.5f);
    const int tabIndex = GENERATE(0, 1, 2);
    auto* context = ImGui::CreateContext();
    struct Cleanup {
        ImGuiContext* context;
        ~Cleanup() { ImGui::DestroyContext(context); }
    } cleanup{context};
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = screen;
    io.FontGlobalScale = scale;
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    CelestialDatabase db;
    CameraController camera;
    PresentationController presentation;
    SolarOdysseyUI ui;
    ui.showPlanetCard = true;
    ui.selectedPlanetName = "Tethys";
    for (int frame = 0; frame < 4; ++frame) {
        ImGui::NewFrame();
        ui.renderPlanetCard(screen.x, screen.y, db, camera, {}, {}, {}, {}, presentation, BodyLayerId::Natural);
        ImGui::Render();
        if (frame == 0) {
            auto* tabs = context->TabBars.GetByIndex(0);
            REQUIRE(tabs->Tabs.Size >= 3);
            tabs->NextSelectedTabId = tabs->Tabs[tabIndex].ID;
        }
    }
    auto* window = ImGui::FindWindowByName("Tethys - Planetary Dossier###PlanetCard");
    REQUIRE(window != nullptr);
    REQUIRE(window->Pos.x >= 0);
    REQUIRE(window->Pos.y >= 0);
    REQUIRE(window->Pos.x + window->Size.x <= screen.x);
    REQUIRE(window->Pos.y + window->Size.y <= screen.y);
    ImGuiTable* table = nullptr;
    for (int index = 0; index < context->Tables.GetMapSize(); ++index) {
        auto* candidate = context->Tables.TryGetMapData(index);
        if (candidate && candidate->LastFrameActive == context->FrameCount) table = candidate;
    }
    REQUIRE(table != nullptr);
    REQUIRE(table->ColumnsCount == 2);
    for (int column = 0; column < 2; ++column) {
        REQUIRE(table->Columns[column].MinX >= window->Pos.x);
        REQUIRE(table->Columns[column].MaxX <= window->Pos.x + window->Size.x);
        REQUIRE(table->Columns[column].WidthGiven > 100);
    }
}
