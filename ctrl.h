#pragma once
// ============================================================
// hamed_ctrl.h — Hamed Arabpour
// Control Flow Engine & Step-Mode Watchdog
// ============================================================
// Condition evaluator and main script execution loop.
// Handles REPEAT / FOREVER / IF / IF_ELSE stacks,
// watchdog (WATCHDOG_MAX_ITERS_PER_FRAME), step-mode flag.
// Style: explicit call stack, bool flags, early returns,
//        structured if-else chains, iteration counters.
// ============================================================

int  EvaluateCondition(shared_ptr<Block> condBlock, Sprite& sprite);
void ExecuteScripts(SDL_Renderer* renderer);
