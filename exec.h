#pragma once
// ============================================================
// hamed_exec.h — Hamed Arabpour
// Block Interpreter & Variable Engine
// ============================================================
// Expression evaluator, variable helpers, hat-trigger
// dispatcher, and the main ExecuteBlock switch-case.
// Style: sequential, guard clauses, explicit switch-case,
//        minimal abstraction, no OOP patterns.
// ============================================================

bool  isNumber(const string& s);
float evalExpr(const string& expr);
bool  parseAssignment(const string& str, string& varName, string& expr);
bool  isValidVarName(const string& name);

int  findVariable(const string& name);
bool defineVariable(const string& name);

void startScriptsForHat(BlockType hatType, const string& param = "");
void ExecuteBlock(Block& block, Sprite& sprite, SDL_Renderer* renderer);
