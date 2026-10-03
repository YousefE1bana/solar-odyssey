#pragma once

// HUD navigation focus is not a reason to stop flight. Explicit text editing
// and the player's cursor-release gesture are the only UI input owners here.
inline bool flightInputAllowed(bool textEditing, bool cursorReleased) {
    return !textEditing && !cursorReleased;
}
