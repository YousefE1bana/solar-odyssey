#pragma once

// Application navigation is independent of the simulation's timeline pause.
// This small owner also defines the single gameplay-input/update gate.
class SessionShell {
public:
    enum class Page { MainMenu, Gameplay, Pause, Settings };
    Page page = Page::MainMenu;
    Page settingsReturn = Page::MainMenu;
    bool sessionStarted = false;
    bool playing() const { return page == Page::Gameplay; }
    void start() { sessionStarted = true; page = Page::Gameplay; }
    void pause() { if (playing()) page = Page::Pause; }
    void settings() { settingsReturn = page; page = Page::Settings; }
    void mainMenu() { page = Page::MainMenu; }
    void escape() {
        if (page == Page::Settings) page = settingsReturn;
        else if (page == Page::Gameplay) page = Page::Pause;
        else if (page == Page::Pause) page = Page::Gameplay;
    }
};
