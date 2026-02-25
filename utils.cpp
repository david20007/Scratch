void pushState(const string& actionDesc) {
    vector<shared_ptr<Block>> blocksClone;
    for (auto& b : scriptBlocks) {
        blocksClone.push_back(b->clone());
    }
    State s;
    s.scriptBlocks = blocksClone;
    s.sprite = allSprites[0].cloneState();
    s.variables = variables;
    s.backdropIndex = mainStage.currentBackdropIndex;

    // If we're not at the end of the stack, remove future states AND their history entries
    if ((int)undoStack.size() > undoIndex + 1) {
        // Remove history entries that point to states beyond undoIndex
        historyLog.erase(
            remove_if(historyLog.begin(), historyLog.end(),
                [](const HistoryItem& h){ return h.stateIndex > undoIndex; }),
            historyLog.end()
        );
        undoStack.erase(undoStack.begin() + undoIndex + 1, undoStack.end());
    }
    undoStack.push_back(s);
    if (undoStack.size() > MAX_UNDO) {
        undoStack.erase(undoStack.begin());
        if (undoIndex > 0) undoIndex--;
        // Fix history stateIndex references
        for (auto& h : historyLog) if (h.stateIndex > 0) h.stateIndex--;
    }
    undoIndex = undoStack.size() - 1;

    logAction(actionDesc, undoIndex);
}

void restoreState(int idx, SDL_Renderer* renderer) {
    if (idx < 0 || idx >= (int)undoStack.size()) return;
    const auto& s = undoStack[idx];
    scriptBlocks.clear();
    for (auto& b : s.scriptBlocks) {
        auto newb = b->clone();
        scriptBlocks.push_back(newb);
        UpdateBlockTexture(newb, renderer);
    }
    for (auto& b : scriptBlocks) {
        if (b->next) b->next->prev = b;
    }
    Sprite& sp = allSprites[0];
    sp.currentCostumeIndex = s.sprite.currentCostumeIndex;
    sp.isVisible = s.sprite.isVisible;
    sp.size = s.sprite.size;
    sp.direction = s.sprite.direction;
    sp.scratchx = s.sprite.scratchx;
    sp.scratchy = s.sprite.scratchy;
    sp.penDown = s.sprite.penDown;
    sp.penColor = s.sprite.penColor;
    sp.penSize = s.sprite.penSize;
    sp.message = s.sprite.message;
    sp.messageTimer = s.sprite.messageTimer;
    sp.isThinking = s.sprite.isThinking;
    variables = s.variables;
    mainStage.currentBackdropIndex = s.backdropIndex;
    undoIndex = idx;
}

void undo(SDL_Renderer* renderer) {
    if (undoIndex > 0) {
        restoreState(undoIndex - 1, renderer);
        string desc = (undoIndex < (int)historyLog.size()) ? historyLog[undoIndex].description : "state " + to_string(undoIndex);
        logAction("Undo: " + desc);
    }
}

void redo(SDL_Renderer* renderer) {
    if (undoIndex < (int)undoStack.size() - 1) {
        restoreState(undoIndex + 1, renderer);
        string desc = (undoIndex < (int)historyLog.size()) ? historyLog[undoIndex].description : "state " + to_string(undoIndex);
        logAction("Redo: " + desc);
    }
}

void resetProject(SDL_Renderer* renderer) {
    FreeBlockTextures();
    scriptBlocks.clear();
    variables.clear();
    historyLog.clear();
    undoStack.clear();
    undoIndex = -1;
    flagScripts.clear(); spaceScripts.clear(); clickScripts.clear(); messageScripts.clear();
    scriptsRunning = false; isPaused = false; stepRequested = false; stepModeActive = false;
    currentExecutingBlock = nullptr;
    draggedBlock = nullptr; snapCandidate = nullptr;
    editing = false; editingBlock = nullptr; editingSpriteProp = -1;
    errorMessage = ""; lastNormalLog = "";
    if (!allSprites.empty()) {
        Sprite& s = allSprites[0];
        s.scratchx = 0; s.scratchy = 0; s.direction = 90.0; s.size = 100.0;
        s.isVisible = true; s.penDown = false; s.message = ""; s.messageTimer = 0;
        s.penStrokes.clear(); s.stamps.clear();
    }
    mainStage.currentBackdropIndex = 0;
    logAction("New project");
}

bool showNewFileDialog(SDL_Renderer* renderer) {
    // Returns true if user confirmed new file
    const int DIALOG_W = 420, DIALOG_H = 180;
    int scrW, scrH;
    SDL_GetRendererOutputSize(renderer, &scrW, &scrH);
    SDL_Rect dialogRect = { scrW/2 - DIALOG_W/2, scrH/2 - DIALOG_H/2, DIALOG_W, DIALOG_H };
    SDL_Rect yesBtn = { dialogRect.x + 60, dialogRect.y + DIALOG_H - 60, 120, 38 };
    SDL_Rect noBtn  = { dialogRect.x + DIALOG_W - 180, dialogRect.y + DIALOG_H - 60, 120, 38 };
    bool dialogRunning = true, result = false;
    SDL_Event e;
    while (dialogRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { result = false; dialogRunning = false; break; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) { result = false; dialogRunning = false; break; }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) { result = true; dialogRunning = false; break; }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x, y = e.button.y;
                if (x>=yesBtn.x && x<=yesBtn.x+yesBtn.w && y>=yesBtn.y && y<=yesBtn.y+yesBtn.h) { result = true; dialogRunning = false; }
                if (x>=noBtn.x  && x<=noBtn.x+noBtn.w   && y>=noBtn.y  && y<=noBtn.y+noBtn.h)  { result = false; dialogRunning = false; }
            }
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0,0,0,150);
        SDL_Rect fs = {0,0,scrW,scrH}; SDL_RenderFillRect(renderer, &fs);
        SDL_SetRenderDrawColor(renderer, 245,248,255,255); SDL_RenderFillRect(renderer, &dialogRect);
        SDL_SetRenderDrawColor(renderer, 77,151,255,255);  SDL_RenderDrawRect(renderer, &dialogRect);
        SDL_Rect titleBar = {dialogRect.x, dialogRect.y, dialogRect.w, 35};
        SDL_SetRenderDrawColor(renderer, 77,151,255,255); SDL_RenderFillRect(renderer, &titleBar);
        RenderText(renderer, dialogRect.x+15, dialogRect.y+8, "New Project", {255,255,255,255});
        RenderText(renderer, dialogRect.x+20, dialogRect.y+50, "Are you sure? Unsaved changes will be lost.", {30,30,30,255});
        SDL_SetRenderDrawColor(renderer, 255,100,100,255); SDL_RenderFillRect(renderer, &yesBtn);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &yesBtn);
        RenderText(renderer, yesBtn.x+25, yesBtn.y+10, "New File", {255,255,255,255});
        SDL_SetRenderDrawColor(renderer, 200,200,200,255); SDL_RenderFillRect(renderer, &noBtn);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &noBtn);
        RenderText(renderer, noBtn.x+30, noBtn.y+10, "Cancel", {30,30,30,255});
        SDL_RenderPresent(renderer); SDL_Delay(10);
    }
    return result;
}

void addBackdropFromFile(SDL_Renderer* renderer, const string& filePath, const string& name) {
    SDL_Surface* surf = IMG_Load(filePath.c_str());
    if (!surf) { setError("Failed to load image: " + string(IMG_GetError())); return; }
    int stageW = mainStage.rect.w, stageH = mainStage.rect.h;
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);
    if (!converted) { setError("Failed to convert image"); return; }
    SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(0, stageW, stageH, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!scaled) { SDL_FreeSurface(converted); setError("Failed to scale image"); return; }
    SDL_BlitScaled(converted, NULL, scaled, NULL);
    SDL_FreeSurface(converted);
    Costume c;
    c.name = name;
    c.width = stageW;
    c.height = stageH;
    c.texture = SDL_CreateTextureFromSurface(renderer, scaled);
    SDL_FreeSurface(scaled);
    if (!c.texture) { setError("Failed to create backdrop texture"); return; }
    mainStage.backdrops.push_back(c);
    mainStage.currentBackdropIndex = (int)mainStage.backdrops.size() - 1;
    logAction("Added backdrop: " + name);
}

string showRenameDialog(SDL_Renderer* renderer, const string& currentName) {
    const int DIALOG_W = 400, DIALOG_H = 160;
    int scrW, scrH;
    SDL_GetRendererOutputSize(renderer, &scrW, &scrH);
    SDL_Rect dialogRect = { scrW/2 - DIALOG_W/2, scrH/2 - DIALOG_H/2, DIALOG_W, DIALOG_H };
    SDL_Rect inputRect = {dialogRect.x + 15, dialogRect.y + 70, DIALOG_W - 30, 30};
    SDL_Rect okBtn = { dialogRect.x + 50, dialogRect.y + DIALOG_H - 45, 100, 32 };
    SDL_Rect cancelBtn = { dialogRect.x + DIALOG_W - 155, dialogRect.y + DIALOG_H - 45, 100, 32 };
    string inputStr = currentName;
    bool dialogRunning = true, confirmed = false;
    SDL_Event e;
    SDL_StartTextInput();
    while (dialogRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { dialogRunning = false; break; }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) { dialogRunning = false; break; }
                if (e.key.keysym.sym == SDLK_RETURN) { confirmed = true; dialogRunning = false; break; }
                if (e.key.keysym.sym == SDLK_BACKSPACE && !inputStr.empty()) inputStr.pop_back();
            }
            if (e.type == SDL_TEXTINPUT) inputStr += e.text.text;
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x, y = e.button.y;
                if (x>=okBtn.x && x<=okBtn.x+okBtn.w && y>=okBtn.y && y<=okBtn.y+okBtn.h) { confirmed = true; dialogRunning = false; }
                if (x>=cancelBtn.x && x<=cancelBtn.x+cancelBtn.w && y>=cancelBtn.y && y<=cancelBtn.y+cancelBtn.h) { dialogRunning = false; }
            }
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0,0,0,150);
        SDL_Rect fs = {0,0,scrW,scrH}; SDL_RenderFillRect(renderer, &fs);
        SDL_SetRenderDrawColor(renderer, 245,248,255,255); SDL_RenderFillRect(renderer, &dialogRect);
        SDL_SetRenderDrawColor(renderer, 77,151,255,255);  SDL_RenderDrawRect(renderer, &dialogRect);
        SDL_Rect titleBar = {dialogRect.x, dialogRect.y, dialogRect.w, 35};
        SDL_SetRenderDrawColor(renderer, 77,151,255,255); SDL_RenderFillRect(renderer, &titleBar);
        RenderText(renderer, dialogRect.x+15, dialogRect.y+8, "Rename Backdrop", {255,255,255,255});
        RenderText(renderer, dialogRect.x+15, dialogRect.y+45, "New name:", {30,30,30,255});
        SDL_SetRenderDrawColor(renderer, 255,255,255,255); SDL_RenderFillRect(renderer, &inputRect);
        SDL_SetRenderDrawColor(renderer, 77,151,255,255); SDL_RenderDrawRect(renderer, &inputRect);
        if (!inputStr.empty()) RenderText(renderer, inputRect.x+5, inputRect.y+7, inputStr, {0,0,0,255});
        // Cursor blink
        if ((SDL_GetTicks()/500) % 2 == 0) {
            if (gFont) {
                SDL_Surface* ms = TTF_RenderUTF8_Blended(gFont, inputStr.c_str(), {0,0,0,255});
                int tw = ms ? ms->w : 0; if (ms) SDL_FreeSurface(ms);
                SDL_SetRenderDrawColor(renderer, 0,0,0,255);
                SDL_RenderDrawLine(renderer, inputRect.x+7+tw, inputRect.y+5, inputRect.x+7+tw, inputRect.y+inputRect.h-5);
            }
        }
        SDL_SetRenderDrawColor(renderer, 77,151,255,255); SDL_RenderFillRect(renderer, &okBtn);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &okBtn);
        RenderText(renderer, okBtn.x+30, okBtn.y+8, "OK", {255,255,255,255});
        SDL_SetRenderDrawColor(renderer, 200,200,200,255); SDL_RenderFillRect(renderer, &cancelBtn);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255); SDL_RenderDrawRect(renderer, &cancelBtn);
        RenderText(renderer, cancelBtn.x+18, cancelBtn.y+8, "Cancel", {30,30,30,255});
        SDL_RenderPresent(renderer); SDL_Delay(10);
    }
    SDL_StopTextInput();
    return confirmed ? inputStr : currentName;
}

void showAboutDialog(SDL_Renderer* renderer) {
    const int DIALOG_W = 500, DIALOG_H = 160;
    int scrW, scrH;
    SDL_GetRendererOutputSize(renderer, &scrW, &scrH);
    SDL_Rect dialogRect = { scrW/2 - DIALOG_W/2, scrH/2 - DIALOG_H/2, DIALOG_W, DIALOG_H };
    SDL_Rect okBtn = { dialogRect.x + DIALOG_W/2 - 50, dialogRect.y + DIALOG_H - 50, 100, 35 };
    bool dialogRunning = true;
    SDL_Event e;
    while (dialogRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { dialogRunning = false; break; }
            if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_RETURN)) {
                dialogRunning = false; break;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x, y = e.button.y;
                if (x >= okBtn.x && x <= okBtn.x + okBtn.w && y >= okBtn.y && y <= okBtn.y + okBtn.h) {
                    dialogRunning = false;
                }
            }
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
        SDL_Rect fullscreen = {0, 0, scrW, scrH};
        SDL_RenderFillRect(renderer, &fullscreen);
        SDL_SetRenderDrawColor(renderer, 245, 248, 255, 255);
        SDL_RenderFillRect(renderer, &dialogRect);
        SDL_SetRenderDrawColor(renderer, 77, 151, 255, 255);
        SDL_RenderDrawRect(renderer, &dialogRect);
        SDL_Rect titleBar = {dialogRect.x, dialogRect.y, dialogRect.w, 35};
        SDL_SetRenderDrawColor(renderer, 77, 151, 255, 255);
        SDL_RenderFillRect(renderer, &titleBar);
        RenderText(renderer, dialogRect.x + 15, dialogRect.y + 8, "About", {255,255,255,255});
        RenderText(renderer, dialogRect.x + 20, dialogRect.y + 50, "Mini scratch application using Sdl2/C++,", {30,30,30,255});
        RenderText(renderer, dialogRect.x + 20, dialogRect.y + 75, "semester 14041, Sharif University of Technology", {30,30,30,255});
        SDL_SetRenderDrawColor(renderer, 77, 151, 255, 255);
        SDL_RenderFillRect(renderer, &okBtn);
        SDL_SetRenderDrawColor(renderer, 66, 128, 217, 255);
        SDL_RenderDrawRect(renderer, &okBtn);
        RenderText(renderer, okBtn.x + 35, okBtn.y + 9, "OK", {255,255,255,255});
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
}

bool showExitDialog(SDL_Renderer* renderer) {
    const int DIALOG_W = 400, DIALOG_H = 200;
    int scrW, scrH;
    SDL_GetRendererOutputSize(renderer, &scrW, &scrH);
    SDL_Rect dialogRect = { scrW/2 - DIALOG_W/2, scrH/2 - DIALOG_H/2, DIALOG_W, DIALOG_H };
    SDL_Rect stayBtn = { dialogRect.x + 50, dialogRect.y + 120, 120, 40 };
    SDL_Rect leaveBtn = { dialogRect.x + 230, dialogRect.y + 120, 120, 40 };
    bool dialogRunning = true;
    bool result = false;
    SDL_Event e;

    while (dialogRunning) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                result = true;
                dialogRunning = false;
                break;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                result = false;
                dialogRunning = false;
                break;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x, y = e.button.y;
                if (x >= stayBtn.x && x <= stayBtn.x + stayBtn.w &&
                    y >= stayBtn.y && y <= stayBtn.y + stayBtn.h) {
                    result = false;
                    dialogRunning = false;
                } else if (x >= leaveBtn.x && x <= leaveBtn.x + leaveBtn.w &&
                           y >= leaveBtn.y && y <= leaveBtn.y + leaveBtn.h) {
                    result = true;
                    dialogRunning = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, NULL);
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderFillRect(renderer, &dialogRect);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawRect(renderer, &dialogRect);
        RenderText(renderer, dialogRect.x + 20, dialogRect.y + 30, "Do you want to leave?", Black);
        RenderText(renderer, dialogRect.x + 20, dialogRect.y + 60, "Unsaved changes may be lost.", Black);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &stayBtn);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &stayBtn);
        RenderText(renderer, stayBtn.x + 30, stayBtn.y + 10, "Stay", Black);
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        SDL_RenderFillRect(renderer, &leaveBtn);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &leaveBtn);
        RenderText(renderer, leaveBtn.x + 30, leaveBtn.y + 10, "Leave", Black);
        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }
    return result;
}