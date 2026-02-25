void UpdateBlockTexture(shared_ptr<Block> block, SDL_Renderer* renderer) {
    if (block->textTexture) { SDL_DestroyTexture(block->textTexture); block->textTexture = nullptr; }
    if (gFont) {
        SDL_Surface* surf = TTF_RenderUTF8_Blended(gFont, block->label.c_str(), White);
        block->textTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    }
}

void BuildPaletteBlocksForCategory(Category cat, SDL_Renderer* renderer) {
    for (auto& b : paletteBlocks) if (b.textTexture) SDL_DestroyTexture(b.textTexture);
    paletteBlocks.clear();

    int x = BlocksFuncs.x+10, y = BlocksFuncs.y+10, w = BlocksFuncs.w-20, h = BLOCK_HEIGHT;
    auto add = [&](BlockType type, const string& base, int val, int val2, const string& str, SDL_Color col) {
        Block b;
        b.type = type; b.category = cat; b.baseLabel = base; b.value = val; b.value2 = val2; b.strValue = str;
        b.rect = {x, y, w, h}; b.color = col; b.inPalette = true;
        string label = base;
        // Replace {} with appropriate values/strings
        size_t p1 = label.find("{}");
        if (p1 != string::npos) {
            if (type >= BLOCK_ADD && type <= BLOCK_SQRT) {
                // For operators, we want the label to be the expression itself
                label = str;
            } else if (type == BLOCK_SAY || type == BLOCK_THINK) {
                // say/think: single {} replaced with strValue (text message)
                label.replace(p1, 2, str.empty() ? "Hello!" : str);
            } else if (type == BLOCK_SAY_FOR || type == BLOCK_THINK_FOR) {
                // say/think for: first {} = message (str), second {} = seconds (val)
                label.replace(p1, 2, str.empty() ? "Hello!" : str);
                size_t p2 = label.find("{}", p1 + (str.empty() ? 6 : str.size()));
                if (p2 != string::npos) label.replace(p2, 2, to_string(val));
            } else {
                label.replace(p1, 2, to_string(val));
                size_t p2 = label.find("{}", p1+2);
                if (p2 != string::npos) {
                    label.replace(p2, 2, to_string(val2));
                }
            }
        } else {
            size_t p = label.find("{}");
            if (p != string::npos) label.replace(p, 2, str);
        }
        b.label = label;
        if (gFont) {
            SDL_Surface* surf = TTF_RenderUTF8_Blended(gFont, label.c_str(), White);
            b.textTexture = SDL_CreateTextureFromSurface(renderer, surf);
            SDL_FreeSurface(surf);
        }
        paletteBlocks.push_back(b);
        y += h+5;
    };

    switch (cat) {
        case CAT_MOTION:
            add(BLOCK_MOVE_STEPS, "move {} steps", 10, 0, "", MotionColor);
            add(BLOCK_TURN_RIGHT, "turn right {} degrees", 15, 0, "", MotionColor);
            add(BLOCK_TURN_LEFT, "turn left {} degrees", 15, 0, "", MotionColor);
            add(BLOCK_GOTO_XY, "go to x:{} y:{}", 0, 0, "", MotionColor);
            add(BLOCK_GOTO_RANDOM, "go to random position", 0, 0, "", MotionColor);
            add(BLOCK_GLIDE, "glide {} secs to x:{} y:{}", 1, 0, "", MotionColor);
            add(BLOCK_CHANGE_X, "change x by {}", 10, 0, "", MotionColor);
            add(BLOCK_SET_X, "set x to {}", 0, 0, "", MotionColor);
            add(BLOCK_CHANGE_Y, "change y by {}", 10, 0, "", MotionColor);
            add(BLOCK_SET_Y, "set y to {}", 0, 0, "", MotionColor);
            add(BLOCK_SET_DIR, "point in direction {}", 90, 0, "", MotionColor);
            add(BLOCK_POINT_TO_MOUSE, "point towards mouse", 0, 0, "", MotionColor);
            add(BLOCK_IF_EDGE_BOUNCE, "if on edge, bounce", 0, 0, "", MotionColor);
            break;
        case CAT_LOOKS:
            add(BLOCK_SAY, "say {}", 0, 0, "Hello!", LooksColor);
            add(BLOCK_SAY_FOR, "say {} for {} secs", 2, 0, "Hello!", LooksColor);
            add(BLOCK_THINK, "think {}", 0, 0, "Hmm...", LooksColor);
            add(BLOCK_THINK_FOR, "think {} for {} secs", 2, 0, "Hmm...", LooksColor);
            add(BLOCK_SHOW, "show", 0, 0, "", LooksColor);
            add(BLOCK_HIDE, "hide", 0, 0, "", LooksColor);
            add(BLOCK_SET_SIZE, "set size to {}%", 100, 0, "", LooksColor);
            add(BLOCK_CHANGE_SIZE, "change size by {}", 10, 0, "", LooksColor);
            break;
        case CAT_SOUND:
            add(BLOCK_PLAY_SOUND, "play sound Meow", 0, 0, "Meow", SoundColor);
            add(BLOCK_STOP_ALL_SOUNDS, "stop all sounds", 0, 0, "", SoundColor);
            add(BLOCK_SET_VOLUME, "set volume to {}%", 100, 0, "", SoundColor);
            add(BLOCK_CHANGE_VOLUME, "change volume by {}", -10, 0, "", SoundColor);
            add(BLOCK_SET_PITCH, "set pitch to {}%", 100, 0, "", SoundColor);
            add(BLOCK_CHANGE_PITCH, "change pitch by {}", 10, 0, "", SoundColor);
            break;
        case CAT_EVENTS:
            add(BLOCK_WHEN_FLAG, "when flag clicked", 0, 0, "", EventsColor);
            add(BLOCK_WHEN_KEY, "when space key pressed", 0, 0, "space", EventsColor);
            add(BLOCK_WHEN_KEY, "when up key pressed", 0, 0, "up", EventsColor);
            add(BLOCK_WHEN_KEY, "when down key pressed", 0, 0, "down", EventsColor);
            add(BLOCK_WHEN_KEY, "when left key pressed", 0, 0, "left", EventsColor);
            add(BLOCK_WHEN_KEY, "when right key pressed", 0, 0, "right", EventsColor);
            add(BLOCK_WHEN_CLICKED, "when this sprite clicked", 0, 0, "", EventsColor);
            add(BLOCK_WHEN_RECEIVE, "when I receive message1", 0, 0, "message1", EventsColor);
            add(BLOCK_BROADCAST, "broadcast message1", 0, 0, "message1", EventsColor);
            add(BLOCK_BROADCAST_WAIT, "broadcast message1 and wait", 0, 0, "message1", EventsColor);
            break;
        case CAT_CONTROL:
            add(BLOCK_WAIT, "wait {} secs", 1, 0, "", ControlColor);
            add(BLOCK_REPEAT, "repeat {}", 10, 0, "", ControlColor);
            add(BLOCK_FOREVER, "forever", 0, 0, "", ControlColor);
            add(BLOCK_IF, "if then", 0, 0, "", ControlColor);
            add(BLOCK_IF_ELSE, "if else", 0, 0, "", ControlColor);
            add(BLOCK_WAIT_UNTIL, "wait until", 0, 0, "", ControlColor);
            add(BLOCK_STOP_ALL, "stop all", 0, 0, "", ControlColor);
            add(BLOCK_END, "end", 0, 0, "", ControlColor);
            break;
        case CAT_SENSING:
            add(BLOCK_TOUCHING, "touching mouse-pointer?", 0, 0, "", SensingColor);
            add(BLOCK_TOUCHING_COLOR, "touching color ?", 0, 0, "", SensingColor);
            add(BLOCK_DISTANCE_TO, "distance to mouse-pointer", 0, 0, "", SensingColor);
            add(BLOCK_ASK, "ask {} and wait", 0, 0, "What's your name?", SensingColor);
            add(BLOCK_KEY_PRESSED, "key space pressed?", 0, 0, "space", SensingColor);
            add(BLOCK_KEY_PRESSED, "key up pressed?", 0, 0, "up", SensingColor);
            add(BLOCK_KEY_PRESSED, "key down pressed?", 0, 0, "down", SensingColor);
            add(BLOCK_KEY_PRESSED, "key left pressed?", 0, 0, "left", SensingColor);
            add(BLOCK_KEY_PRESSED, "key right pressed?", 0, 0, "right", SensingColor);
            add(BLOCK_MOUSE_DOWN, "mouse down?", 0, 0, "", SensingColor);
            break;
        case CAT_OPERATORS:
            add(BLOCK_ADD, "{}", 0, 0, "0+0", OperatorsColor);
            add(BLOCK_SUBTRACT, "{}", 0, 0, "0-0", OperatorsColor);
            add(BLOCK_MULTIPLY, "{}", 0, 0, "1*1", OperatorsColor);
            add(BLOCK_DIVIDE, "{}", 0, 0, "1/1", OperatorsColor);
            add(BLOCK_RANDOM, "{}", 0, 0, "1,10", OperatorsColor);
            add(BLOCK_LESS_THAN, "{}", 0, 0, "0<1", OperatorsColor);
            add(BLOCK_EQUAL, "{}", 0, 0, "0=0", OperatorsColor);
            add(BLOCK_GREATER_THAN, "{}", 0, 0, "1>0", OperatorsColor);
            add(BLOCK_AND, "{}", 1, 1, "1&1", OperatorsColor);
            add(BLOCK_OR, "{}", 1, 1, "0|1", OperatorsColor);
            add(BLOCK_NOT, "not {}", 0, 0, "not 0", OperatorsColor);
            add(BLOCK_MOD, "{}", 0, 0, "10%3", OperatorsColor);
            add(BLOCK_ROUND, "round {}", 0, 0, "round 3.7", OperatorsColor);
            add(BLOCK_ABS, "abs {}", 0, 0, "abs -5", OperatorsColor);
            add(BLOCK_SQRT, "sqrt {}", 0, 0, "sqrt 9", OperatorsColor);
            add(BLOCK_JOIN, "join {} {}", 0, 0, "hello,world", OperatorsColor);
            add(BLOCK_LETTER_OF, "letter {} of {}", 1, 0, "1,world", OperatorsColor);
            add(BLOCK_LENGTH, "length of {}", 0, 0, "hello", OperatorsColor);
            add(BLOCK_CONTAINS, "{} contains {}?", 0, 0, "abc,ab", OperatorsColor);
            break;
        case CAT_VARIABLES:
            add(BLOCK_DEFINE_VARIABLE, "make variable {}", 0, 0, "var", VariablesColor);
            add(BLOCK_SET_VAR, "set {} to {}", 0, 0, "var=0", VariablesColor);
            add(BLOCK_CHANGE_VAR, "change {} by {}", 1, 0, "var=1", VariablesColor);
            add(BLOCK_SHOW_VARIABLE, "show variable {}", 0, 0, "var", VariablesColor);
            add(BLOCK_HIDE_VARIABLE, "hide variable {}", 0, 0, "var", VariablesColor);
            add(BLOCK_VAR, "{}", 0, 0, "var", VariablesColor);
            break;
        case CAT_PEN:
            add(BLOCK_PEN_DOWN, "pen down", 0, 0, "", PenColor);
            add(BLOCK_PEN_UP, "pen up", 0, 0, "", PenColor);
            add(BLOCK_SET_PEN_COLOR, "set pen color to {}", 0, 0, "", PenColor);
            add(BLOCK_CHANGE_PEN_SIZE, "change pen size by {}", 1, 0, "", PenColor);
            add(BLOCK_SET_PEN_SIZE, "set pen size to {}", 1, 0, "", PenColor);
            add(BLOCK_ERASE_ALL, "erase all", 0, 0, "", PenColor);
            add(BLOCK_STAMP, "stamp", 0, 0, "", PenColor);
            break;
        case CAT_MYBLOCKS:
            add(BLOCK_MYBLOCK_DEFINE, "define {}", 0, 0, "my block", MyBlocksColor);
            add(BLOCK_MYBLOCK_CALL, "{}", 0, 0, "my block", MyBlocksColor);
            break;
        default: break;
    }
}

void DrawBlock(const Block& block, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, block.color.r, block.color.g, block.color.b, 255);
    SDL_RenderFillRect(renderer, &block.rect);
    // Draw blue border if this is the currently executing block in step mode
    if (stepModeActive && currentExecutingBlock && currentExecutingBlock.get() == &block) {
        SDL_SetRenderDrawColor(renderer, 0, 120, 255, 255);
        SDL_Rect r1 = {block.rect.x-2, block.rect.y-2, block.rect.w+4, block.rect.h+4};
        SDL_RenderDrawRect(renderer, &r1);
        SDL_Rect r2 = {block.rect.x-3, block.rect.y-3, block.rect.w+6, block.rect.h+6};
        SDL_RenderDrawRect(renderer, &r2);
    } else {
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderDrawRect(renderer, &block.rect);
    }
    if (block.textTexture) {
        int tw, th;
        SDL_QueryTexture(block.textTexture, NULL, NULL, &tw, &th);
        SDL_Rect r = {block.rect.x+5, block.rect.y+(block.rect.h-th)/2, tw, th};
        SDL_RenderCopy(renderer, block.textTexture, NULL, &r);
    }
}

void DrawAllBlocks(SDL_Renderer* renderer) {
    for (auto& b : paletteBlocks) DrawBlock(b, renderer);
    for (auto& b : scriptBlocks) DrawBlock(*b, renderer);
    if (draggedBlock) DrawBlock(*draggedBlock, renderer);
    if (draggedBlock && snapCandidate) {
        bool above = snapCandidate->rect.y < draggedBlock->rect.y;
        int x1 = (above ? snapCandidate->rect.x : draggedBlock->rect.x) + (above ? snapCandidate->rect.w : draggedBlock->rect.w)/2;
        int y1 = above ? snapCandidate->rect.y + snapCandidate->rect.h : draggedBlock->rect.y + draggedBlock->rect.h;
        int x2 = (above ? draggedBlock->rect.x : snapCandidate->rect.x) + (above ? draggedBlock->rect.w : snapCandidate->rect.w)/2;
        int y2 = above ? draggedBlock->rect.y : snapCandidate->rect.y;
        thickLineRGBA(renderer, x1, y1, x2, y2, 3, 255,255,0,255);
    }
    if (editing && editingBlock) {
        int fieldX, fieldY, fieldW = 60, fieldH = editingBlock->rect.h-10;
        if (editingFieldIndex == 0) {
            fieldX = editingBlock->rect.x + editingBlock->rect.w - 130;
        } else if (editingFieldIndex == 1) {
            fieldX = editingBlock->rect.x + editingBlock->rect.w - 65;
        } else {
            fieldX = editingBlock->rect.x + editingBlock->rect.w - 130;
            fieldW = 120;
        }
        fieldY = editingBlock->rect.y + 5;
        SDL_Rect r = {fieldX, fieldY, fieldW, fieldH};
        SDL_SetRenderDrawColor(renderer,255,255,255,255); SDL_RenderFillRect(renderer,&r);
        SDL_SetRenderDrawColor(renderer,0,0,0,255); SDL_RenderDrawRect(renderer,&r);
        if (gFont) {
            SDL_Surface* s = TTF_RenderUTF8_Blended(gFont, editInputString.c_str(), Black);
            SDL_Texture* t = SDL_CreateTextureFromSurface(renderer,s);
            int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
            SDL_Rect tr = {r.x+2, r.y+(r.h-th)/2, min(tw,r.w-4), th};
            SDL_RenderCopy(renderer,t,NULL,&tr);
            SDL_DestroyTexture(t); SDL_FreeSurface(s);
        }
    }
}

void FreeBlockTexture(Block& b) { if (b.textTexture) { SDL_DestroyTexture(b.textTexture); b.textTexture = nullptr; } }

void InsertBlockAtSnap(shared_ptr<Block> block, shared_ptr<Block> target, bool above) {
    if (above) {
        block->next = target->next;
        if (target->next) target->next->prev = block;
        target->next = block;
        block->prev = target;
        block->rect.y = target->rect.y + target->rect.h + GAP;
    } else {
        block->prev = target->prev.lock();
        if (auto p = target->prev.lock()) p->next = block;
        target->prev = block;
        block->next = target;
        block->rect.y = target->rect.y - block->rect.h - GAP;
    }
    block->rect.x = target->rect.x;
    pushState("Snapped block: " + block->label);
}

void HandleBlockEvents(SDL_Event& event, int mouseX, int mouseY, bool mouseDown, bool mouseUp, SDL_Renderer* renderer) {
    if (mouseDown) {
        Uint32 now = SDL_GetTicks();
        if (now - lastUpTime < 500 && abs(mouseX - lastUpX) < 5 && abs(mouseY - lastUpY) < 5) {
            for (int i = scriptBlocks.size()-1; i>=0; i--) {
                auto& b = scriptBlocks[i];
                if (mouseX >= b->rect.x && mouseX <= b->rect.x+b->rect.w && mouseY >= b->rect.y && mouseY <= b->rect.y+b->rect.h) {
                    if (auto p = b->prev.lock()) p->next = b->next;
                    if (b->next) b->next->prev = b->prev;
                    FreeBlockTexture(*b);
                    scriptBlocks.erase(scriptBlocks.begin()+i);
                    pushState("Deleted block: " + b->label);
                    return;
                }
            }
        }

        for (size_t i=0; i<paletteBlocks.size(); i++) {
            auto& pb = paletteBlocks[i];
            if (mouseX >= pb.rect.x && mouseX <= pb.rect.x+pb.rect.w && mouseY >= pb.rect.y && mouseY <= pb.rect.y+pb.rect.h) {
                auto nb = make_shared<Block>(pb);
                nb->inPalette = false; nb->rect.x = mouseX; nb->rect.y = mouseY;
                nb->isDragging = true; dragOffX = mouseX - nb->rect.x; dragOffY = mouseY - nb->rect.y;
                if (gFont) {
                    SDL_Surface* s = TTF_RenderUTF8_Blended(gFont, nb->label.c_str(), White);
                    nb->textTexture = SDL_CreateTextureFromSurface(renderer, s);
                    SDL_FreeSurface(s);
                }
                draggedBlock = nb; draggingFromPalette = true; snapCandidate = nullptr;
                return;
            }
        }

        for (int i = scriptBlocks.size()-1; i>=0; i--) {
            auto& b = scriptBlocks[i];
            if (mouseX >= b->rect.x && mouseX <= b->rect.x+b->rect.w && mouseY >= b->rect.y && mouseY <= b->rect.y+b->rect.h) {
                potentialDrag = true;
                clickStartX = mouseX; clickStartY = mouseY; clickStartTime = now; clickBlock = b;
                // Determine if click is in editable area
                if (b->hasTwoEditableValues()) {
                    int field1_x = b->rect.x + b->rect.w - 130;
                    int field2_x = b->rect.x + b->rect.w - 65;
                    int field_w = 60;
                    clickInValueArea = (mouseX >= field1_x && mouseX <= field1_x + field_w);
                    clickInValue2Area = (mouseX >= field2_x && mouseX <= field2_x + field_w);
                    clickInStringArea = false;
                } else if (b->hasEditableValue()) {
                    // For say/think, treat the {} as a string field
                    if (b->type == BLOCK_SAY || b->type == BLOCK_SAY_FOR ||
                        b->type == BLOCK_THINK || b->type == BLOCK_THINK_FOR) {
                        int strX = b->rect.x + b->rect.w - 130;
                        int strW = 120;
                        clickInStringArea = (mouseX >= strX && mouseX <= strX + strW);
                        clickInValueArea = clickInValue2Area = false;
                    } else {
                        int vaX = b->rect.x + b->rect.w - 70;
                        clickInValueArea = (mouseX >= vaX && mouseX <= vaX + 60);
                        clickInValue2Area = false;
                        clickInStringArea = false;
                    }
                } else if (b->hasStringValue()) {
                    int strX = b->rect.x + b->rect.w - 130;
                    int strW = 120;
                    clickInStringArea = (mouseX >= strX && mouseX <= strX + strW);
                    clickInValueArea = clickInValue2Area = false;
                } else {
                    clickInValueArea = clickInValue2Area = clickInStringArea = false;
                }
                return;
            }
        }
    }

    if (potentialDrag && event.type == SDL_MOUSEMOTION) {
        int dx = mouseX - clickStartX, dy = mouseY - clickStartY;
        if (abs(dx) > DRAG_THRESHOLD || abs(dy) > DRAG_THRESHOLD) {
            if (auto p = clickBlock->prev.lock()) p->next = clickBlock->next;
            if (clickBlock->next) clickBlock->next->prev = clickBlock->prev;
            clickBlock->prev.reset(); clickBlock->next = nullptr;
            clickBlock->isDragging = true;
            dragOffX = mouseX - clickBlock->rect.x; dragOffY = mouseY - clickBlock->rect.y;
            draggedBlock = clickBlock; draggingFromPalette = false; snapCandidate = nullptr;
            potentialDrag = false;
        }
    }

    if (draggedBlock && event.type == SDL_MOUSEMOTION) {
        draggedBlock->rect.x = mouseX - dragOffX; draggedBlock->rect.y = mouseY - dragOffY;
        int best = SNAP_DIST; snapCandidate = nullptr;
        for (auto& b : scriptBlocks) {
            if (b == draggedBlock) continue;
            int da = abs((b->rect.y + b->rect.h) - draggedBlock->rect.y);
            if (da < best && abs(b->rect.x - draggedBlock->rect.x) < 40) { best = da; snapCandidate = b; }
            int db = abs((draggedBlock->rect.y + draggedBlock->rect.h) - b->rect.y);
            if (db < best && abs(b->rect.x - draggedBlock->rect.x) < 40) { best = db; snapCandidate = b; }
        }
    }

    if (mouseUp && draggedBlock) {
        draggedBlock->isDragging = false;
        bool inPlate = (mouseX >= Plate.x && mouseX <= Plate.x+Plate.w && mouseY >= Plate.y && mouseY <= Plate.y+Plate.h);
        if (inPlate) {
            if (snapCandidate) {
                bool above = (snapCandidate->rect.y < draggedBlock->rect.y);
                InsertBlockAtSnap(draggedBlock, snapCandidate, above);
            } else {
                draggedBlock->next = nullptr; draggedBlock->prev.reset();
            }
            if (!draggingFromPalette) scriptBlocks.erase(find(scriptBlocks.begin(), scriptBlocks.end(), draggedBlock));
            scriptBlocks.push_back(draggedBlock);
            pushState("Placed block: " + draggedBlock->label);
        } else {
            FreeBlockTexture(*draggedBlock);
            if (!draggingFromPalette) scriptBlocks.erase(find(scriptBlocks.begin(), scriptBlocks.end(), draggedBlock));
        }
        draggedBlock = nullptr; snapCandidate = nullptr;
    }

    if (mouseUp && potentialDrag) {
        if (clickBlock) {
            if (clickBlock->isHat()) {
                // nothing
            } else if (clickBlock->hasTwoEditableValues() && (clickInValueArea || clickInValue2Area)) {
                editingBlock = clickBlock;
                if (clickInValueArea) {
                    editingFieldIndex = 0;
                    editInputString = to_string(editingBlock->value);
                } else {
                    editingFieldIndex = 1;
                    editInputString = to_string(editingBlock->value2);
                }
                editing = true; SDL_StartTextInput();
            } else if (clickBlock->hasEditableValue() && clickInValueArea) {
                editingBlock = clickBlock;
                editingFieldIndex = 0;
                // For say/think blocks, always use string editing
                if (clickBlock->type == BLOCK_SAY || clickBlock->type == BLOCK_SAY_FOR ||
                    clickBlock->type == BLOCK_THINK || clickBlock->type == BLOCK_THINK_FOR) {
                    editingFieldIndex = 2;
                    editInputString = editingBlock->strValue;
                } else {
                    editInputString = to_string(editingBlock->value);
                }
                editing = true; SDL_StartTextInput();
            } else if (clickBlock->hasStringValue() && clickInStringArea) {
                editingBlock = clickBlock;
                editingFieldIndex = 2;
                editInputString = editingBlock->strValue;
                editing = true; SDL_StartTextInput();
            }
        }
        potentialDrag = false; clickBlock = nullptr;
    }
}

void RenderText(SDL_Renderer* r, int x, int y, const string& txt, SDL_Color col) {
    if (!gFont) return;
    SDL_Surface* s = TTF_RenderUTF8_Blended(gFont, txt.c_str(), col);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
    int tw, th; SDL_QueryTexture(t, NULL, NULL, &tw, &th);
    SDL_Rect rect = {x, y, tw, th};
    SDL_RenderCopy(r, t, NULL, &rect);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

void HandleSpriteInfoEvents(int mouseX, int mouseY, bool mouseUp, SDL_Renderer* renderer) {
    if (!mouseUp) return;
    if (!IsMouseOverRect(Sprite_Info, mouseX, mouseY)) return;

    int infoX = Sprite_Info.x + 15;
    int infoY = Sprite_Info.y + 15;
    int lineH = 24;
    int colW = 90;
    int valW = 56;

    SDL_Rect rects[4] = {
        {infoX + 30, infoY, valW, lineH},
        {infoX + 30 + colW, infoY, valW, lineH},
        {infoX + 30, infoY + lineH, valW, lineH},
        {infoX + 30 + colW, infoY + lineH, valW, lineH}
    };
    for (int i=0; i<4; i++) {
        if (IsMouseOverRect(rects[i], mouseX, mouseY)) {
            editingSpriteProp = i;
            Sprite& s = allSprites[0];
            switch (i) {
                case 0: spriteEditString = to_string((int)s.scratchx); break;
                case 1: spriteEditString = to_string((int)s.scratchy); break;
                case 2: spriteEditString = to_string((int)s.direction); break;
                case 3: spriteEditString = to_string((int)s.size); break;
            }
            SDL_StartTextInput();
            return;
        }
    }
}

void DrawSpriteInfo(SDL_Renderer* renderer) {
    SDL_Rect bg = Sprite_Info;
    roundedBoxRGBA(renderer, bg.x, bg.y, bg.x+bg.w, bg.y+bg.h, 8, 255,255,255,255);
    roundedRectangleRGBA(renderer, bg.x, bg.y, bg.x+bg.w, bg.y+bg.h, 8, 180,180,180,255);

    if (!gFont) return;

    Sprite& s = allSprites[0];
    int infoX = bg.x + 15;
    int infoY = bg.y + 15;
    int lineH = 24;
    int colW = 90;
    SDL_Color labelColor = {80,80,80,255};
    SDL_Color valueColor = {0,0,0,255};

    auto drawText = [&](int x, int y, const string& txt, SDL_Color col) {
        RenderText(renderer, x, y, txt, col);
    };

    drawText(infoX, infoY, "x:", labelColor);
    drawText(infoX + colW, infoY, "y:", labelColor);
    drawText(infoX, infoY + lineH, "dir:", labelColor);
    drawText(infoX + colW, infoY + lineH, "size:", labelColor);

    int valX = infoX + 30;
    int valW = 56;
    SDL_Rect valRects[4] = {
        {valX, infoY, valW, lineH-4},
        {valX + colW, infoY, valW, lineH-4},
        {valX, infoY + lineH, valW, lineH-4},
        {valX + colW, infoY + lineH, valW, lineH-4}
    };
    for (int i=0; i<4; i++) {
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderFillRect(renderer, &valRects[i]);
        SDL_SetRenderDrawColor(renderer, 200,200,200,255);
        SDL_RenderDrawRect(renderer, &valRects[i]);
    }

    drawText(valX + 4, infoY + 2, to_string((int)s.scratchx), valueColor);
    drawText(valX + colW + 4, infoY + 2, to_string((int)s.scratchy), valueColor);
    drawText(valX + 4, infoY + lineH + 2, to_string((int)s.direction), valueColor);
    drawText(valX + colW + 4, infoY + lineH + 2, to_string((int)s.size) + "%", valueColor);

    if (editingSpriteProp != -1) {
        int inputX = valX + (editingSpriteProp % 2) * colW;
        int inputY = infoY + (editingSpriteProp / 2) * lineH;
        SDL_Rect inputRect = {inputX, inputY, valW, lineH-4};
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderFillRect(renderer, &inputRect);
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderDrawRect(renderer, &inputRect);
        if (!spriteEditString.empty() && gFont) {
            SDL_Surface* s = TTF_RenderUTF8_Blended(gFont, spriteEditString.c_str(), Black);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                int tw,th; SDL_QueryTexture(t,NULL,NULL,&tw,&th);
                SDL_Rect tr = {inputRect.x + 4, inputRect.y + (inputRect.h - th)/2, min(tw, inputRect.w-8), th};
                SDL_RenderCopy(renderer, t, NULL, &tr);
                SDL_DestroyTexture(t);
                SDL_FreeSurface(s);
            }
        }
    }
}

// ==================== SPRITE DRAWING ====================
void drawSpriteAtIndex(SDL_Renderer* renderer, const Sprite& sprite, int costumeIndex, int x, int y, double size, double direction) {
    if (!sprite.costumes.empty() && costumeIndex >= 0 && costumeIndex < sprite.costumes.size()) {
        const Costume& c = sprite.costumes[costumeIndex];
        SDL_Rect rect;
        rect.w = static_cast<int>(c.width * size / 100.0);
        rect.h = static_cast<int>(c.height * size / 100.0);
        rect.x = x - rect.w / 2;
        rect.y = y - rect.h / 2;
        double ang = direction - 90.0;
        SDL_RenderCopyEx(renderer, c.texture, NULL, &rect, ang, NULL, SDL_FLIP_NONE);
    } else {
        int w = static_cast<int>(50 * size / 100.0);
        int h = static_cast<int>(50 * size / 100.0);
        int cx = x;
        int cy = y;
        int eyeSize = w / 8;
        int pupilSize = eyeSize / 2;
        int mouthY = cy + h / 8;
        int mouthW = w / 4;

        roundedBoxRGBA(renderer, x - w / 2, y - h / 2, x + w / 2, y + h / 2, h / 4,
                       255, 200, 100, 255);

        Sint16 leftEarX[3] = { (Sint16)(cx - w / 4), (Sint16)(cx - w / 3), (Sint16)(cx - w / 6) };
        Sint16 leftEarY[3] = { (Sint16)(cy - h / 3), (Sint16)(cy - h / 2), (Sint16)(cy - h / 3) };
        filledPolygonRGBA(renderer, leftEarX, leftEarY, 3, 255, 160, 80, 255);
        Sint16 rightEarX[3] = { (Sint16)(cx + w / 4), (Sint16)(cx + w / 3), (Sint16)(cx + w / 6) };
        Sint16 rightEarY[3] = { (Sint16)(cy - h / 3), (Sint16)(cy - h / 2), (Sint16)(cy - h / 3) };
        filledPolygonRGBA(renderer, rightEarX, rightEarY, 3, 255, 160, 80, 255);

        filledCircleRGBA(renderer, cx - w / 6, cy - h / 8, eyeSize, 255, 255, 255, 255);
        filledCircleRGBA(renderer, cx + w / 6, cy - h / 8, eyeSize, 255, 255, 255, 255);
        filledCircleRGBA(renderer, cx - w / 6, cy - h / 8, pupilSize, 0, 0, 0, 255);
        filledCircleRGBA(renderer, cx + w / 6, cy - h / 8, pupilSize, 0, 0, 0, 255);

        Sint16 noseX[3] = { (Sint16)(cx - w / 16), (Sint16)(cx + w / 16), (Sint16)cx };
        Sint16 noseY[3] = { (Sint16)(cy - h / 16), (Sint16)(cy - h / 16), (Sint16)(cy + h / 16) };
        filledPolygonRGBA(renderer, noseX, noseY, 3, 255, 100, 100, 255);

        for (int i = -mouthW / 2; i <= mouthW / 2; i += 2) {
            int x1 = cx + i;
            int y1 = mouthY + abs(i) / 3;
            int x2 = cx + i + 2;
            int y2 = mouthY + abs(i + 2) / 3;
            lineRGBA(renderer, x1, y1, x2, y2, 0, 0, 0, 255);
        }

        lineRGBA(renderer, cx - w / 5, cy, cx - w / 2, cy - h / 10, 0, 0, 0, 255);
        lineRGBA(renderer, cx - w / 5, cy, cx - w / 2, cy + h / 10, 0, 0, 0, 255);
        lineRGBA(renderer, cx + w / 5, cy, cx + w / 2, cy - h / 10, 0, 0, 0, 255);
        lineRGBA(renderer, cx + w / 5, cy, cx + w / 2, cy + h / 10, 0, 0, 0, 255);

        int ex = cx + (int)(w / 2 * cos(direction * M_PI / 180));
        int ey = cy - (int)(h / 2 * sin(direction * M_PI / 180));
        lineRGBA(renderer, cx, cy, ex, ey, 0, 0, 0, 255);
    }
}

void Draw_Sprite(SDL_Renderer* renderer, Sprite& sprite, Stage& stage) {
    for (const auto& stamp : sprite.stamps) {
        drawSpriteAtIndex(renderer, sprite, stamp.costumeIndex, stamp.x, stamp.y, stamp.size, stamp.direction);
    }

    if (!sprite.isVisible) return;
    Update_Sprite_Render_Rect(sprite, stage);
    int cx = sprite.rect.x + sprite.rect.w / 2;
    int cy = sprite.rect.y + sprite.rect.h / 2;
    drawSpriteAtIndex(renderer, sprite, sprite.currentCostumeIndex, cx, cy, sprite.size, sprite.direction);

    if (!sprite.message.empty() && sprite.messageTimer > 0) {
        int bx = sprite.rect.x + sprite.rect.w/2 - 60;
        int by = sprite.rect.y - 40;
        int bw = 120, bh = 30;
        if (sprite.isThinking) {
            roundedBoxRGBA(renderer, bx, by, bx+bw, by+bh, 8, 255,255,255,255);
            roundedRectangleRGBA(renderer, bx, by, bx+bw, by+bh, 8, 0,0,0,255);
            filledCircleRGBA(renderer, bx-10, by+bh-10, 8, 255,255,255,255);
        } else {
            roundedBoxRGBA(renderer, bx, by, bx+bw, by+bh, 8, 255,255,255,255);
            roundedRectangleRGBA(renderer, bx, by, bx+bw, by+bh, 8, 0,0,0,255);
            filledTrigonRGBA(renderer, bx+bw/2-5, by+bh, bx+bw/2+5, by+bh, sprite.rect.x+sprite.rect.w/2, by+bh+10, 255,255,255,255);
        }
        RenderText(renderer, bx+10, by+8, sprite.message, Black);
    }
}

// ==================== UI FUNCTIONS ====================
void DrawLoading(SDL_Renderer* renderer, TTF_Font* font1) {
    SDL_Surface* s; SDL_Texture* t; SDL_Rect r;
    SDL_SetRenderDrawColor(renderer, Blue.r, Blue.g, Blue.b, Blue.a);
    SDL_RenderClear(renderer);
    s = TTF_RenderText_Blended(font1, "Scratch is loading...", (SDL_Color{255,255,255,255}));
    if (s) {
        t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_QueryTexture(t, NULL, NULL, &r.w, &r.h);
        r.x = (1280 - r.w)/2; r.y = (720 - r.h)/2;
        SDL_RenderCopy(renderer, t, NULL, &r);
        SDL_FreeSurface(s); SDL_DestroyTexture(t);
    }
    TTF_CloseFont(font1);
}
void DrawStage(SDL_Renderer* r, SDL_Rect& s, SDL_Color c) {
    roundedBoxRGBA(r, s.x, s.y, s.x+s.w, s.y+s.h, 10, c.r,c.g,c.b,c.a);
    roundedRectangleRGBA(r, s.x, s.y, s.x+s.w, s.y+s.h, 10, 200,200,200,255);
}
void Draw_Full_Toolbar(SDL_Renderer* r, SDL_Rect& t, Button b[4]) {
    DrawToolbar(r, t, Blue);
    int w,h; if (Scratch_Logo) { SDL_QueryTexture(Scratch_Logo, NULL, NULL, &w, &h); SDL_Rect pic={5,(t.h-h)/2,w,h}; SDL_RenderCopy(r,Scratch_Logo,NULL,&pic); }
    for (int i=0;i<4;i++) DrawMenuButton(r,b[i]);
}
void DrawMenuButton(SDL_Renderer* r, Button& btn) {
    SDL_Color col = Blue;
    if (btn.state==Button_Normal) col=btn.Normal;
    if (btn.isselected) col=btn.Selected;
    if (btn.state==Button_Hovered) col=btn.hovered;
    SDL_Texture* pic = (btn.buttonID==BTN_Stop && Go.isselected) ? btn.Picture2 :
                       (btn.buttonID==BTN_Pause && isPaused) ? btn.Picture2 :
                       (btn.buttonID==BTN_Pause && !isPaused) ? btn.Picture1 :
                       btn.Picture1;
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(r, &btn.rect);
    int w,h;
    if (btn.text_texture1) {
        SDL_QueryTexture(btn.text_texture1, NULL, NULL, &w, &h);
        SDL_Rect tr = {btn.rect.x+(btn.rect.w-w)/2, btn.rect.y+(btn.rect.h-h)/2, w, h};
        SDL_RenderCopy(r, btn.text_texture1, NULL, &tr);
    }
    if (pic) {
        SDL_QueryTexture(pic, NULL, NULL, &w, &h);
        SDL_Rect pr = {btn.rect.x+(btn.rect.w-w)/2, btn.rect.y+(btn.rect.h-h)/2, w, h};
        SDL_RenderCopy(r, pic, NULL, &pr);
    }
}
void DrawBlockButton(SDL_Renderer* r, Button& btn) {
    // تعیین رنگ پس‌زمینه: طوسی وقتی selected، سفید در حالت عادی
    SDL_Color bgCol = btn.Normal; // White
    if (btn.isselected) bgCol = btn.Selected; // Gray
    SDL_SetRenderDrawColor(r, bgCol.r, bgCol.g, bgCol.b, bgCol.a);
    SDL_RenderFillRect(r, &btn.rect);

    // دایره با رنگ کتگوری — همیشه ثابت
    int circleX = btn.rect.x + btn.rect.w / 2;
    int circleY = btn.rect.y + btn.rect.h / 2 - 8;
    aacircleRGBA(r, circleX, circleY, 9, 0,0,0,255);
    filledCircleRGBA(r, circleX, circleY, 8, btn.hovered.r, btn.hovered.g, btn.hovered.b, btn.hovered.a);

    // متن: آبی موقع hover (تحت هر شرایطی)، مشکی در غیر این صورت
    SDL_Texture* tex = (btn.state == Button_Hovered) ? btn.text_texture2 : btn.text_texture1;
    if (tex) {
        int w,h; SDL_QueryTexture(tex, NULL, NULL, &w, &h);
        int tw = w, th = h;
        if (tw > btn.rect.w - 2) {
            float sc = (float)(btn.rect.w - 2) / (float)tw;
            tw = btn.rect.w - 2;
            th = (int)(th * sc);
        }
        // متن زیر دایره، وسط‌چین
        SDL_Rect tr = {btn.rect.x + (btn.rect.w - tw)/2, circleY + 11, tw, th};
        SDL_RenderCopy(r, tex, NULL, &tr);
    }
}
void Draw_Full_BlockBar_Menu(SDL_Renderer* r, SDL_Rect& bb, Button b[11]) {
    DrawBlockbar_Funcs(r, BlockBar, White);
    for (int i=0;i<10;i++) {
        if (b[i].buttonID == BTN_Pen && !penEnabled) continue;
        DrawBlockButton(r,b[i]);
    }
    DrawMenuButton(r,b[10]);
    if (b[10].Picture1) {
        int w,h; SDL_QueryTexture(b[10].Picture1,NULL,NULL,&w,&h);
        SDL_Rect rec = {b[10].rect.x+(b[10].rect.w-w)/2, b[10].rect.y+(b[10].rect.h-h)/2, w, h};
        SDL_RenderCopy(r, b[10].Picture1, NULL, &rec);
    }
    SDL_SetRenderDrawColor(r,200,200,200,255); SDL_RenderDrawRect(r,&bb);
}
void Define_Toolbar_BTN_Text(SDL_Renderer* r, Button btn[4]) {
    TTF_Font* f = TTF_OpenFont("assets/OpenSans-Regular.ttf", 18); if(!f) return;
    for (int i=0;i<4;i++) { SDL_Surface* s = TTF_RenderText_Solid(f, btn[i].text.c_str(), White); btn[i].text_texture1 = SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s); }
    TTF_CloseFont(f);
}
void Draw_Circle_BTN(SDL_Renderer* r, Button& btn) {
    SDL_Color col = Blue;
    if (btn.state==Button_Normal) col=btn.Normal;
    if (btn.state==Button_Hovered) col=btn.hovered;
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    filledCircleRGBA(r, btn.rect.x+btn.rect.w/2, btn.rect.y+btn.rect.h/2, 25, col.r,col.g,col.b,120);
    filledCircleRGBA(r, btn.rect.x+btn.rect.w/2, btn.rect.y+btn.rect.h/2, 21, col.r,col.g,col.b,col.a);
    aacircleRGBA(r, btn.rect.x+btn.rect.w/2, btn.rect.y+btn.rect.h/2, 21, col.r,col.g,col.b,col.a);
    aacircleRGBA(r, btn.rect.x+btn.rect.w/2, btn.rect.y+btn.rect.h/2, 25, col.r,col.g,col.b,120);
    if (btn.Picture1) {
        int w,h; SDL_QueryTexture(btn.Picture1,NULL,NULL,&w,&h);
        SDL_Rect tr = {btn.rect.x+(btn.rect.w-w)/2, btn.rect.y+(btn.rect.h-h)/2, w, h};
        SDL_RenderCopy(r, btn.Picture1, NULL, &tr);
    }
}

void Render(SDL_Renderer* r) {
    SDL_SetRenderDrawColor(r, Background.r, Background.g, Background.b, Background.a);
    SDL_RenderClear(r);
    Draw_Stage(r, mainStage);
    Draw_Full_Toolbar(r, toolbar, ToolBar_Button);
    Draw_Full_BlockBar_Menu(r, BlockBar, BlockBar_Button);
    DrawBlockbar_Funcs(r, BlocksFuncs, White);
    DrawBlockbar_Funcs(r, Plate, White);
    DrawStage(r, BackDrop_List, White);
    DrawStage(r, Sprite_Info, White);
    DrawStage(r, Sprite_List, White);
    DrawBlockbar_Funcs(r, Sprite_List, Background);
    DrawMenuButton(r, Go);
    DrawMenuButton(r, Stop);
    DrawMenuButton(r, Pause);
    DrawMenuButton(r, StepBtn);
    // UndoBtn and RedoBtn hidden from UI (functionality preserved)
    // DrawMenuButton(r, UndoBtn);
    // DrawMenuButton(r, RedoBtn);
    DrawAllBlocks(r);
    SDL_RenderSetClipRect(r, &mainStage.rect);
    DrawPenLines(r);
    for (auto& s : allSprites) Draw_Sprite(r, s, mainStage);
    SDL_RenderSetClipRect(r, NULL);
    DrawSpriteInfo(r);

    int varY = 400;
    for (auto& var : variables) {
        if (var.isShown) {
            string text = var.name + " = " + to_string(var.value);
            RenderText(r, 800, varY, text, Black);
            varY += 20;
        }
    }

    // ========== Backdrop panel ==========
    SDL_Rect backdropPanel = BackDrop_List;
    SDL_SetRenderDrawColor(r, 255,255,255,255);
    SDL_RenderFillRect(r, &backdropPanel);
    SDL_SetRenderDrawColor(r, 180,180,180,255);
    SDL_RenderDrawRect(r, &backdropPanel);
    int yOff = backdropPanel.y + 5;
    for (size_t i = 0; i < mainStage.backdrops.size(); i++) {
        SDL_Rect itemRect = {backdropPanel.x + 5, yOff, backdropPanel.w - 10, 30};

        SDL_Color btnColor = {255, 255, 255, 255};

        SDL_SetRenderDrawColor(r, btnColor.r, btnColor.g, btnColor.b, btnColor.a);
        SDL_RenderFillRect(r, &itemRect);

        if (i == mainStage.currentBackdropIndex) {
            SDL_SetRenderDrawColor(r, 0,100,255,255);
            SDL_RenderDrawRect(r, &itemRect);
            SDL_Rect inner = {itemRect.x+1, itemRect.y+1, itemRect.w-2, itemRect.h-2};
            SDL_SetRenderDrawColor(r, 0,100,255,255);
            SDL_RenderDrawRect(r, &inner);
        } else {
            SDL_SetRenderDrawColor(r, 100,100,100,255);
            SDL_RenderDrawRect(r, &itemRect);
        }

        string name = mainStage.backdrops[i].name;
        RenderText(r, itemRect.x + 5, itemRect.y + 5, name, Black);
        yOff += 32;
    }

    // ========== History panel ==========
    SDL_Rect historyPanel = Sprite_List;
    SDL_SetRenderDrawColor(r, 240,240,240,255);
    SDL_RenderFillRect(r, &historyPanel);
    SDL_SetRenderDrawColor(r, 180,180,180,255);
    SDL_RenderDrawRect(r, &historyPanel);
    int hx = historyPanel.x + 5;
    int hw = historyPanel.w - 10;
    int lineH = 20;
    int total = historyLog.size();
    int start = max(0, total - 14);
    int y = historyPanel.y + 5;
    for (int idx = total - 1; idx >= start; idx--) {
        SDL_Rect itemRect = {hx, y, hw, lineH};
        if (idx == undoIndex) {
            SDL_SetRenderDrawColor(r, 200,230,255,255);
            SDL_RenderFillRect(r, &itemRect);
        } else {
            SDL_SetRenderDrawColor(r, 255,255,255,255);
            SDL_RenderFillRect(r, &itemRect);
        }
        SDL_SetRenderDrawColor(r, 0,0,0,255);
        SDL_RenderDrawRect(r, &itemRect);
        string display = historyLog[idx].description;
        if (display.length() > 25) display = display.substr(0,22) + "...";
        RenderText(r, itemRect.x + 2, itemRect.y + 2, display, Black);
        y += lineH + 2;
        if (y + lineH > historyPanel.y + historyPanel.h - 5) break;
    }

    // ========== Error/Log bar ==========
    SDL_Rect logBar = {60, 720-35, 1220, 35};
    SDL_SetRenderDrawColor(r, 255,255,255,255);
    SDL_RenderFillRect(r, &logBar);
    SDL_SetRenderDrawColor(r, 0,0,0,255);
    SDL_RenderDrawRect(r, &logBar);
    if (SDL_GetTicks() < errorMessageTimer && !errorMessage.empty()) {
        RenderText(r, 70, 720-28, errorMessage, Red);
    } else if (!lastNormalLog.empty()) {
        RenderText(r, 70, 720-28, lastNormalLog, Black);
    }

    Draw_Circle_BTN(r, Sprite_Choosing);
    Draw_Circle_BTN(r, Add_BackDrop);

    if (ToolBar_Button[0].isselected) Draw_Panel(r, File_BTN_Panel);
    if (ToolBar_Button[1].isselected) Draw_Panel(r, Edit_BTN_Panel);
    if (ToolBar_Button[3].isselected) Draw_Panel(r, Help_BTN_Panel);

    // ========== Add Extension Panel ==========
    if (showExtPanel) {
        const int PANEL_W = 260, PANEL_H = 120;
        SDL_Rect panel = {Blockbar_Width + 10, 720 - Blockbar_Width - PANEL_H - 5, PANEL_W, PANEL_H};
        SDL_SetRenderDrawColor(r, 245,245,245,255);
        SDL_RenderFillRect(r, &panel);
        SDL_SetRenderDrawColor(r, 100,100,100,255);
        SDL_RenderDrawRect(r, &panel);
        RenderText(r, panel.x + 10, panel.y + 10, "Add Extension:", Black);

        SDL_Rect penBtn = {panel.x + 10, panel.y + 40, PANEL_W - 20, 35};
        if (!penEnabled) {
            SDL_SetRenderDrawColor(r, 0,200,0,255);
            SDL_RenderFillRect(r, &penBtn);
            SDL_SetRenderDrawColor(r, 0,150,0,255);
            SDL_RenderDrawRect(r, &penBtn);
            RenderText(r, penBtn.x + 10, penBtn.y + 8, "Pen", White);
        } else {
            SDL_SetRenderDrawColor(r, 180,220,180,255);
            SDL_RenderFillRect(r, &penBtn);
            SDL_SetRenderDrawColor(r, 100,160,100,255);
            SDL_RenderDrawRect(r, &penBtn);
            RenderText(r, penBtn.x + 10, penBtn.y + 8, "Pen (Added)", Black);
        }

        SDL_Rect closeBtn = {panel.x + PANEL_W - 25, panel.y + 5, 20, 20};
        SDL_SetRenderDrawColor(r, 200,80,80,255);
        SDL_RenderFillRect(r, &closeBtn);
        RenderText(r, closeBtn.x + 4, closeBtn.y + 2, "X", White);
    }

    SDL_RenderPresent(r);
}

void Define_Blockbar_BTN_Text(SDL_Renderer* r, Button btn[11]) {
    // آزاد کردن textures قدیمی
    for (int i=0;i<10;i++) {
        if (btn[i].text_texture1) { SDL_DestroyTexture(btn[i].text_texture1); btn[i].text_texture1 = nullptr; }
        if (btn[i].text_texture2) { SDL_DestroyTexture(btn[i].text_texture2); btn[i].text_texture2 = nullptr; }
    }
    TTF_Font* f = TTF_OpenFont("assets/OpenSans-Regular.ttf", 10); if(!f) return;
    SDL_Surface* s;
    SDL_Color textBlack = {0,0,0,255};
    for (int i=0;i<10;i++) {
        // texture1: متن مشکی (حالت عادی و selected)
        s = TTF_RenderUTF8_Blended(f, btn[i].text.c_str(), textBlack);
        btn[i].text_texture1 = SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s);
        // texture2: متن آبی (موقع hover)
        s = TTF_RenderUTF8_Blended(f, btn[i].text.c_str(), Blue);
        btn[i].text_texture2 = SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s);
    }
    s = IMG_Load("assets/icons8-add-properties-32.png"); if(s) { btn[10].Picture1 = SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s); }
    TTF_CloseFont(f);
}
void Define_Panel_BTN_Text(SDL_Renderer* r, Button btn[], int cnt) {
    TTF_Font* f = TTF_OpenFont("assets/OpenSans-Regular.ttf", 30); if(!f) return;
    for (int i=0;i<cnt;i++) { SDL_Surface* s = TTF_RenderUTF8_Blended(f, btn[i].text.c_str(), btn[i].textcolor); btn[i].text_texture1 = SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s); }
    TTF_CloseFont(f);
}
void Draw_Panel(SDL_Renderer* r, Panel p) { DrawToolbar(r, p.rect, Blue); for (int i=0;i<p.count;i++) Draw_Panel_Btn(r, p.btn[i]); }
void Draw_Panel_Btn(SDL_Renderer* r, Button& btn) {
    SDL_Color col = Blue;
    if (btn.state==Button_Normal) col=btn.Normal;
    else if (btn.state==Button_Pressed) col=btn.Selected;
    else if (btn.state==Button_Hovered) col=btn.hovered;
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a); SDL_RenderFillRect(r, &btn.rect);
    if (btn.buttonID==BTN_NewFile) { SDL_SetRenderDrawColor(r, DarkerBlue.r, DarkerBlue.g, DarkerBlue.b, DarkerBlue.a); SDL_RenderDrawRect(r, &btn.rect); }
    if (btn.text_texture1) {
        int w,h; SDL_QueryTexture(btn.text_texture1,NULL,NULL,&w,&h); w/=2; h/=2;
        SDL_Rect tr = {btn.rect.x+2, btn.rect.y+(btn.rect.h-h)/2, w, h};
        SDL_RenderCopy(r, btn.text_texture1, NULL, &tr);
    }
}
void False_Selected(Button btn[], int c) { for (int i=0;i<c;i++) btn[i].isselected=false; }
void UpdateAllButtons(vector<Button*> bts, int mx, int my, bool md, bool mu, bool* clk) {
    Button* hover = nullptr;
    for (int i=bts.size()-1; i>=0; i--) {
        bool over = false;
        if (i>=15 && i<=17) { if (ToolBar_Button[0].isselected) over = IsMouseOverRect(bts[i]->rect, mx, my); }
        else if (i==18 || i==19) { if (ToolBar_Button[1].isselected) over = IsMouseOverRect(bts[i]->rect, mx, my); }
        else if (i>=20 && i<=22) { if (ToolBar_Button[3].isselected) over = IsMouseOverRect(bts[i]->rect, mx, my); }
        else over = IsMouseOverRect(bts[i]->rect, mx, my);
        if (over) { hover = bts[i]; break; }
    }
    for (auto b : bts) {
        if (b == hover) { UpdateButtonState(*b, mx, my, md); HandleButtonClick(bts, *b, mx, my, mu, g_renderer); *clk = true; }
        else b->state = Button_Normal;
    }
}
void UpdateSprites(vector<Sprite>& sp, Stage& st, int mx, int my, bool isDown, bool isUp, bool uiHandled, SDL_Renderer* renderer) {
    for (auto& s : sp) {
        if (s.messageTimer > 0) {
            s.messageTimer--;
            if (s.messageTimer == 0) s.message.clear();
        }
    }
    for (auto& s : sp) {
        Update_Sprite_Render_Rect(s, st);
        Clamp_Sprite_To_Stage_Bounds(s, st);
    }
    if (isUp) for (auto& s : sp) s.isBeingDragged = false;
    if (isDown && !uiHandled) {
        for (int i = sp.size()-1; i >= 0; i--) {
            if (IsMouseOverRect(sp[i].rect, mx, my)) {
                bool any = false;
                for (auto& s : sp) if (s.isBeingDragged) any = true;
                if (!any && sp[i].isDraggable) {
                    sp[i].isBeingDragged = true;
                    sp[i].dragOffsetX = mx - sp[i].rect.x;
                    sp[i].dragOffsetY = my - sp[i].rect.y;
                    break;
                }
            }
        }
    }
    for (auto& s : sp) {
        if (s.isBeingDragged) {
            int nx = mx - s.dragOffsetX, ny = my - s.dragOffsetY;
            int ncx = nx + s.rect.w/2, ncy = ny + s.rect.h/2;
            int scx = st.rect.x + st.rect.w/2, scy = st.rect.y + st.rect.h/2;
            double newScratchx = ncx - scx;
            double newScratchy = scy - ncy;
            double oldX = s.scratchx;
            double oldY = s.scratchy;
            s.scratchx = newScratchx;
            s.scratchy = newScratchy;
            Clamp_Sprite_To_Stage_Bounds(s, st);
            AddPenStroke(s, oldX, oldY, s.scratchx, s.scratchy, renderer);
        }
    }
}

void Add_Costume_To_Sprite(SDL_Renderer* r, Sprite& s, const string& name, const char* path) {
    Costume c;
    c.name = name;
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) {
        cout << "Failed to load costume: " << IMG_GetError() << endl;
        return;
    }

    SDL_Surface* rgbaSurf = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surf);
    if (!rgbaSurf) {
        cout << "Failed to convert surface to RGBA: " << SDL_GetError() << endl;
        return;
    }

    int origW = rgbaSurf->w;
    int origH = rgbaSurf->h;
    const int MAX_DIM = 110;
    float scale = 1.0f;

    if (origW > MAX_DIM || origH > MAX_DIM) {
        if (origW > origH)
            scale = (float)MAX_DIM / origW;
        else
            scale = (float)MAX_DIM / origH;
    }

    int newW = (int)(origW * scale);
    int newH = (int)(origH * scale);

    SDL_Surface* scaledSurf = SDL_CreateRGBSurfaceWithFormat(0, newW, newH, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!scaledSurf) {
        cout << "Failed to create scaled surface: " << SDL_GetError() << endl;
        SDL_FreeSurface(rgbaSurf);
        return;
    }
    SDL_BlitScaled(rgbaSurf, NULL, scaledSurf, NULL);
    SDL_FreeSurface(rgbaSurf);

    c.texture = SDL_CreateTextureFromSurface(r, scaledSurf);
    c.width = newW;
    c.height = newH;
    SDL_FreeSurface(scaledSurf);

    s.costumes.push_back(c);

    if (s.costumes.size() == 1) {
        s.currentCostumeIndex = 0;
        s.size = 100.0f;
    }
}

void Draw_Stage(SDL_Renderer* r, Stage& st) {
    SDL_RenderSetClipRect(r, &st.rect);
    if (!st.backdrops.empty() && st.currentBackdropIndex>=0) SDL_RenderCopy(r, st.backdrops[st.currentBackdropIndex].texture, NULL, &st.rect);
    else { SDL_SetRenderDrawColor(r,255,255,255,255); SDL_RenderFillRect(r, &st.rect); }
    SDL_RenderSetClipRect(r, NULL);
    roundedRectangleRGBA(r, st.rect.x, st.rect.y, st.rect.x+st.rect.w, st.rect.y+st.rect.h, 10, 200,200,200,255);
}
void DrawToolbar(SDL_Renderer* r, SDL_Rect& t, SDL_Color c) { SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a); SDL_RenderFillRect(r,&t); }
void DrawBlockbar_Funcs(SDL_Renderer* r, SDL_Rect& b, SDL_Color c) { SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a); SDL_RenderFillRect(r,&b); SDL_SetRenderDrawColor(r,200,200,200,255); SDL_RenderDrawRect(r,&b); }
void Define_Toolbar(SDL_Renderer* r, Button b[4]) {
    b[0]={{100,0,45,Toolbar_Height},Blue,DarkerBlue,DarkerBlue,White,Button_Normal,BTN_File,"File"};
    b[1]={{b[0].rect.x+b[0].rect.w,0,45,Toolbar_Height},Blue,DarkerBlue,DarkerBlue,White,Button_Normal,BTN_Edit,"Edit"};
    b[2]={{b[1].rect.x+b[1].rect.w,0,90,Toolbar_Height},Blue,DarkerBlue,DarkerBlue,White,Button_Normal,BTN_Tutorial,"Tutorials"};
    b[3]={{1280-45,0,45,Toolbar_Height},Blue,DarkerBlue,DarkerBlue,White,Button_Normal,BTN_QuestionMark,""};
    SDL_Surface* s = IMG_Load("assets/icons8-help-26.png"); if(s) { b[3].Picture1 = SDL_CreateTextureFromSurface(r,s); SDL_FreeSurface(s); }
}
void Define_FilePanel_Btn(Button b[3]) {
    b[0]={{100,45,200,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_NewFile,"New",false};
    b[1]={{100,b[0].rect.y+b[0].rect.h,b[0].rect.w,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_LoadFile,"Load from your computer",false};
    b[2]={{100,b[1].rect.y+b[1].rect.h,b[0].rect.w,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_SaveFile,"Save to your computer",false};
}
void Define_EditPanel_Btn(Button b[2]) {
    b[0]={{145,45,200,30},Blue,Blue,DarkerBlue,LightBlue,Button_Normal,BTN_Restore,"Restore",false};
    b[1]={{145,b[0].rect.y+b[0].rect.h,200,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_Turbo,"Turn on Turbo Mode",false};
}
void Define_HelpPanel_Btn(Button b[3]) {
    b[0]={{1080,45,200,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_About,"About",false};
    b[1]={{1080,b[0].rect.y+b[0].rect.h,b[0].rect.w,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_PP,"Privacy and Policy",false};
    b[2]={{1080,b[1].rect.y+b[1].rect.h,b[0].rect.w,30},Blue,Blue,DarkerBlue,White,Button_Normal,BTN_DataSet,"Data Settings",false};
}
void Define_BlockBar(Button b[11]) {
    int y = Toolbar_Height + Empty_Height;
    // مربع دقیق ۶۰×۶۰
    const int SZ = Blockbar_Width; // 60
    const int GAP = 2;
    b[0]={{0,y,              SZ,SZ},White,Gray,MotionColor,   Black,Button_Normal,BTN_Motion,   "Motion",  true};
    b[1]={{0,y+  (SZ+GAP),  SZ,SZ},White,Gray,LooksColor,    Black,Button_Normal,BTN_Looks,    "Looks"};
    b[2]={{0,y+2*(SZ+GAP),  SZ,SZ},White,Gray,SoundColor,    Black,Button_Normal,BTN_Sound,    "Sound"};
    b[3]={{0,y+3*(SZ+GAP),  SZ,SZ},White,Gray,EventsColor,   Black,Button_Normal,BTN_Events,   "Events"};
    b[4]={{0,y+4*(SZ+GAP),  SZ,SZ},White,Gray,ControlColor,  Black,Button_Normal,BTN_Control,  "Control"};
    b[5]={{0,y+5*(SZ+GAP),  SZ,SZ},White,Gray,SensingColor,  Black,Button_Normal,BTN_Sensing,  "Sensing"};
    b[6]={{0,y+6*(SZ+GAP),  SZ,SZ},White,Gray,OperatorsColor,Black,Button_Normal,BTN_Operators,"Operatrs"};
    b[7]={{0,y+7*(SZ+GAP),  SZ,SZ},White,Gray,VariablesColor,Black,Button_Normal,BTN_Variables,"Vars"};
    b[8]={{0,y+8*(SZ+GAP),  SZ,SZ},White,Gray,PenColor,      Black,Button_Normal,BTN_Pen,      "Pen"};
    b[9]={{0,y+9*(SZ+GAP),  SZ,SZ},White,Gray,MyBlocksColor, Black,Button_Normal,BTN_MyBlocks, "MyBlks"};
    b[10]={{0,720-SZ,SZ,SZ},Blue,Blue,Blue,White,Button_Normal,BTN_AddExt,""};
}
bool IsMouseOverRect(SDL_Rect& r, int x, int y) { SDL_Point p={x,y}; return SDL_PointInRect(&p,&r); }
bool IsMouseOverCircle(Button& btn, int x, int y) { int cx=btn.rect.x+btn.rect.w/2, cy=btn.rect.y+btn.rect.h/2, dx=x-cx, dy=y-cy, rad=btn.rect.w/2; return dx*dx+dy*dy <= rad*rad; }
void UpdateButtonState(Button& btn, int x, int y, bool d) {
    bool over = (btn.buttonID==BTN_Play||btn.buttonID==BTN_Stop||btn.buttonID==BTN_Pause||btn.buttonID==BTN_Step||btn.buttonID==BTN_SChoose||btn.buttonID==BTN_AddBack) ? IsMouseOverCircle(btn,x,y) : IsMouseOverRect(btn.rect,x,y);
    btn.state = over ? (d ? Button_Pressed : Button_Hovered) : Button_Normal;
}
void HandleButtonClick(vector<Button*> bts, Button& btn, int x, int y, bool up, SDL_Renderer* renderer) {
    if (!up) return;
    bool over = (btn.buttonID==BTN_Play||btn.buttonID==BTN_Stop||btn.buttonID==BTN_Pause||btn.buttonID==BTN_Step||btn.buttonID==BTN_SChoose||btn.buttonID==BTN_AddBack||btn.buttonID==BTN_AddExt||IsBlockCategory(btn.buttonID)) ? IsMouseOverCircle(btn,x,y) : IsMouseOverRect(btn.rect,x,y);
    if (over) {
        if (IsBlockCategory(btn.buttonID) || IsMenuButton(btn.buttonID)) {
            for (auto b : bts) {
                if (IsBlockCategory(btn.buttonID) && IsBlockCategory(b->buttonID)) b->isselected = false;
                if (btn.buttonID==BTN_Stop && b->buttonID==BTN_Play) b->isselected = false;
                if (IsMenuButton(b->buttonID)) b->isselected = false;
            }
            btn.isselected = true;
        } else if (btn.buttonID == BTN_Play) {
            scriptsRunning = true; isPaused = false; stepRequested = false; flagScripts.clear();
            startScriptsForHat(BLOCK_WHEN_FLAG);
            pushState("Green flag clicked");
        } else if (btn.buttonID == BTN_Stop) {
            scriptsRunning = false; isPaused = false; stepRequested = false; stepModeActive = false;
            flagScripts.clear(); spaceScripts.clear(); clickScripts.clear(); messageScripts.clear();
            currentExecutingBlock = nullptr; StepBtn.isselected = false;
            pushState("Stop clicked");
        } else if (btn.buttonID == BTN_Pause) {
            isPaused = !isPaused;
            pushState(isPaused ? "Paused" : "Resumed");
        } else if (btn.buttonID == BTN_Step) {
            if (!stepModeActive) {
                // Enter step mode and execute first step
                stepModeActive = true;
                isPaused = true;
                if (!scriptsRunning) {
                    flagScripts.clear();
                    startScriptsForHat(BLOCK_WHEN_FLAG);
                    scriptsRunning = true;
                }
                stepRequested = true;
                pushState("Step mode ON");
            } else {
                // Already in step mode: advance exactly one step
                stepRequested = true;
                isPaused = false;
                pushState("Step");
            }
        } else if (btn.buttonID == BTN_Undo) {
            if (renderer) undo(renderer);
        } else if (btn.buttonID == BTN_Redo) {
            if (renderer) redo(renderer);
        } else if (btn.buttonID == BTN_AddExt) {
            showExtPanel = !showExtPanel;
        } else if (btn.buttonID == BTN_AddBack) {
            // Open file dialog for image upload
            const char* filters[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif" };
            const char* filePath = tinyfd_openFileDialog("Choose Backdrop Image", "", 5, filters, "Image Files", 0);
            if (filePath && renderer) {
                // Extract filename without extension as name
                string fp(filePath);
                string fname = fp;
                size_t slash = fp.find_last_of("/\\");
                if (slash != string::npos) fname = fp.substr(slash+1);
                size_t dot = fname.find_last_of('.');
                if (dot != string::npos) fname = fname.substr(0, dot);
                addBackdropFromFile(renderer, fp, fname);
            }
        } else {
            btn.isselected = true;
        }
    }
}
bool IsBlockCategory(ButtonID id) { return id==BTN_MyBlocks||id==BTN_Control||id==BTN_Variables||id==BTN_Operators||id==BTN_Looks||id==BTN_Motion||id==BTN_Sensing||id==BTN_Events||id==BTN_Sound||id==BTN_Pen; }
bool IsMenuButton(ButtonID id) { return id==BTN_File||id==BTN_Edit||id==BTN_Tutorial||id==BTN_QuestionMark; }

void Update_Sprite_Render_Rect(Sprite& s, Stage& st) {
    if (s.costumes.empty()) {
        s.rect.w = static_cast<int>(50 * s.size / 100.0);
        s.rect.h = static_cast<int>(50 * s.size / 100.0);
    } else {
        Costume& c = s.costumes[s.currentCostumeIndex];
        s.rect.w = static_cast<int>(c.width * s.size / 100.0);
        s.rect.h = static_cast<int>(c.height * s.size / 100.0);
    }
    int cx = st.rect.x + st.rect.w/2, cy = st.rect.y + st.rect.h/2;
    int sx = cx + (int)s.scratchx, sy = cy - (int)s.scratchy;
    s.rect.x = sx - s.rect.w/2;
    s.rect.y = sy - s.rect.h/2;
}
void Move_Sprite(Sprite& s, Stage& st, double steps) {
    double rad = s.direction * M_PI/180;
    s.scratchx += steps * cos(rad); s.scratchy += steps * sin(rad);
}
void Turn_Sprite(Sprite& s, double deg) { s.direction += deg; s.direction = fmod(s.direction,360); if (s.direction<0) s.direction+=360; }
