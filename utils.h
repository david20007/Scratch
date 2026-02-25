#pragma once
// ============================================================
// ali_utils.h — Ali Dehghan
// Undo/Redo, Dialogs & Project Lifecycle
// ============================================================
// State snapshot system (pushState/restoreState), undo/redo,
// project reset, backdrop loading, and all modal dialogs.
// Style: snapshot-based state management, defensive checks,
//        consistent comments, moderate abstraction.
// ============================================================

void pushState(const string& actionDesc);
void restoreState(int idx, SDL_Renderer* renderer);
void undo(SDL_Renderer* renderer);
void redo(SDL_Renderer* renderer);

void resetProject(SDL_Renderer* renderer);
bool showNewFileDialog(SDL_Renderer* renderer);
void addBackdropFromFile(SDL_Renderer* renderer, const string& filePath, const string& name);
string showRenameDialog(SDL_Renderer* renderer, const string& currentName);
void showAboutDialog(SDL_Renderer* renderer);
bool showExitDialog(SDL_Renderer* renderer);
