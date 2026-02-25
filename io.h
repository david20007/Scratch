#pragma once
// ============================================================
// ali_io.h — Ali Dehghan
// File I/O, Error Log & Category Colors
// ============================================================
// Project save/load (versioned text format), file dialogs,
// error and action logging system, category color lookup.
// Style: defensive checks, clear error messages,
//        consistent naming, guarded file operations.
// ============================================================

void setError(const string& msg);
void addHistory(const string& desc, int stateIdx);
void logAction(const string& msg, int stateIdx = -1);

string getOpenFileName();
string getSaveFileName();
SDL_Color getCategoryColor(Category c);

void saveProject(const string& filename);
void loadProject(const string& filename, SDL_Renderer* renderer);
