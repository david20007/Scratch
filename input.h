#pragma once
// ============================================================
// davoud_input.h — Davoud Samie Darian
// Input Handling, Pen & Drag-Drop System
// ============================================================
// Sprite drag-and-drop, pen stroke recording & drawing,
// surface shrink utility, block texture cleanup.
// Style: explicit boolean flags, direct coordinate math,
//        manual event routing, index-based state checks.
// ============================================================

void FreeBlockTextures();
void Handle_Sprite_Drag_And_Drop(Sprite& sprite, Stage& stage, const SDL_Event& event, int mouseX, int mouseY);
void Clamp_Sprite_To_Stage_Bounds(Sprite& sprite, Stage& stage);
void DrawPenLines(SDL_Renderer* renderer);
void AddPenStroke(Sprite& sprite, double oldX, double oldY, double newX, double newY, SDL_Renderer* renderer);
