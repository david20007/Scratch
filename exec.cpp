bool isNumber(const string& s) {
    if (s.empty()) return false;
    char* end;
    strtod(s.c_str(), &end);
    return *end == 0;
}

// Evaluate a simple expression (numbers and variables, binary operators + - * / % < > = & |)
// Operators are scanned from right to left, so precedence is not standard but sufficient.
float evalExpr(const string& expr) {
    if (expr.empty()) return 0.0f;

    // Trim spaces
    string e = expr;
    e.erase(remove(e.begin(), e.end(), ' '), e.end());

    // Operators in order of scanning (rightmost first)
    const char* ops[] = {"|", "&", "<", ">", "=", "+", "-", "*", "/", "%"};
    for (int oi = 0; oi < 10; oi++) {
        string op = ops[oi];
        size_t pos = e.rfind(op);
        if (pos != string::npos && pos > 0 && pos < e.length()-1) {
            string left = e.substr(0, pos);
            string right = e.substr(pos + op.length());
            float l = evalExpr(left);
            float r = evalExpr(right);
            if (op == "+") return l + r;
            if (op == "-") return l - r;
            if (op == "*") return l * r;
            if (op == "/") {
                if (r == 0) { setError("Division by zero"); return 0; }
                return l / r;
            }
            if (op == "%") {
                if (r == 0) { setError("Mod by zero"); return 0; }
                return fmod(l, r);
            }
            if (op == "<") return (l < r) ? 1.0f : 0.0f;
            if (op == ">") return (l > r) ? 1.0f : 0.0f;
            if (op == "=") return (fabs(l - r) < 1e-6) ? 1.0f : 0.0f;
            if (op == "&") return (l != 0 && r != 0) ? 1.0f : 0.0f;
            if (op == "|") return (l != 0 || r != 0) ? 1.0f : 0.0f;
        }
    }
    // Handle unary 'not'
    if (e.substr(0,3) == "not") {
        string arg = e.substr(3);
        float val = evalExpr(arg);
        return (val == 0) ? 1.0f : 0.0f;
    }
    // Check if it's a number
    if (isNumber(e)) return (float)atof(e.c_str());
    // Otherwise, treat as variable
    int idx = findVariable(e);
    if (idx != -1) return (float)variables[idx].value;
    setError("Undefined variable: " + e);
    return 0;
}

// Split assignment string like "var=10+score" into variable name and expression
bool parseAssignment(const string& str, string& varName, string& expr) {
    size_t eq = str.find('=');
    if (eq == string::npos) return false;
    varName = str.substr(0, eq);
    // trim spaces
    varName.erase(remove(varName.begin(), varName.end(), ' '), varName.end());
    expr = str.substr(eq + 1);
    return true;
}

// Check if variable name is valid (not all digits)
bool isValidVarName(const string& name) {
    if (name.empty()) return false;
    bool allDigits = true;
    for (char c : name) if (!isdigit(c)) { allDigits = false; break; }
    return !allDigits;
}

// ==================== UNDO/REDO ====================

// Variable helpers
int findVariable(const string& name) {
    for (size_t i = 0; i < variables.size(); i++)
        if (variables[i].name == name) return i;
    return -1;
}
bool defineVariable(const string& name) {
    if (findVariable(name) != -1) return false;
    if (!isValidVarName(name)) {
        setError("Variable name cannot be a number");
        return false;
    }
    Variable v;
    v.name = name;
    v.value = 0;
    v.isShown = false;
    v.rect = {0,0,0,0};
    variables.push_back(v);
    pushState("Variable defined: " + name);
    return true;
}

void startScriptsForHat(BlockType hatType, const string& param = "") {
    for (auto& b : scriptBlocks) {
        if (b->type == hatType && !b->prev.lock()) {
            if (hatType == BLOCK_WHEN_KEY) {
                if (b->strValue != param) continue;
            } else if (hatType == BLOCK_WHEN_RECEIVE) {
                if (b->strValue != param) continue;
            }
            ScriptState s;
            s.current = b->next;
            s.waitFrames = 0;
            s.waitingForBroadcast = "";
            s.waitingChildren = 0;
            if (hatType == BLOCK_WHEN_FLAG) flagScripts.push_back(s);
            else if (hatType == BLOCK_WHEN_KEY) spaceScripts.push_back(s);
            else if (hatType == BLOCK_WHEN_CLICKED) clickScripts.push_back(s);
            else if (hatType == BLOCK_WHEN_RECEIVE) messageScripts[param].push_back(s);
        }
    }
}

void ExecuteBlock(Block& block, Sprite& sprite, SDL_Renderer* renderer) {
    switch (block.type) {
        case BLOCK_MOVE_STEPS: {
            double oldX = sprite.scratchx, oldY = sprite.scratchy;
            Move_Sprite(sprite, mainStage, block.value);
            AddPenStroke(sprite, oldX, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("move " + to_string(block.value) + " steps");
            break;
        }
        case BLOCK_TURN_RIGHT: {
            Turn_Sprite(sprite, block.value);
            pushState("turn right " + to_string(block.value) + " degrees");
            break;
        }
        case BLOCK_TURN_LEFT: {
            Turn_Sprite(sprite, -block.value);
            pushState("turn left " + to_string(block.value) + " degrees");
            break;
        }
        case BLOCK_GOTO_XY: {
            double oldX = sprite.scratchx, oldY = sprite.scratchy;
            sprite.scratchx = block.value; sprite.scratchy = block.value2;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, oldX, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("go to x:" + to_string(block.value) + " y:" + to_string(block.value2));
            break;
        }
        case BLOCK_GOTO_RANDOM: {
            double oldX = sprite.scratchx, oldY = sprite.scratchy;
            Update_Sprite_Render_Rect(sprite, mainStage);
            double stageW = mainStage.rect.w / 2.0;
            double stageH = mainStage.rect.h / 2.0;
            double halfW = sprite.rect.w / 2.0;
            double halfH = sprite.rect.h / 2.0;
            double minX = -stageW - halfW/2;
            double maxX =  stageW + halfW/2;
            double minY = -stageH - halfH/2;
            double maxY =  stageH + halfH/2;
            if (maxX < minX) { minX = maxX = 0; }
            if (maxY < minY) { minY = maxY = 0; }
            double rangeX = maxX - minX;
            double rangeY = maxY - minY;
            sprite.scratchx = minX + (rand() / (double)RAND_MAX) * rangeX;
            sprite.scratchy = minY + (rand() / (double)RAND_MAX) * rangeY;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, oldX, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("go to random position");
            break;
        }
        case BLOCK_SET_X: {
            double oldX = sprite.scratchx;
            sprite.scratchx = block.value;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, oldX, sprite.scratchy, sprite.scratchx, sprite.scratchy, renderer);
            pushState("set x to " + to_string(block.value));
            break;
        }
        case BLOCK_SET_Y: {
            double oldY = sprite.scratchy;
            sprite.scratchy = block.value;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, sprite.scratchx, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("set y to " + to_string(block.value));
            break;
        }
        case BLOCK_CHANGE_X: {
            double oldX = sprite.scratchx;
            sprite.scratchx += block.value;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, oldX, sprite.scratchy, sprite.scratchx, sprite.scratchy, renderer);
            pushState("change x by " + to_string(block.value));
            break;
        }
        case BLOCK_CHANGE_Y: {
            double oldY = sprite.scratchy;
            sprite.scratchy += block.value;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, sprite.scratchx, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("change y by " + to_string(block.value));
            break;
        }
        case BLOCK_SET_DIR:
            sprite.direction = block.value;
            pushState("point in direction " + to_string(block.value));
            break;
        case BLOCK_POINT_TO_MOUSE: {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            int cx = mainStage.rect.x + mainStage.rect.w/2;
            int cy = mainStage.rect.y + mainStage.rect.h/2;
            double dx = (mx - cx) - sprite.scratchx;
            double dy = (cy - my) - sprite.scratchy;
            sprite.direction = atan2(dy, dx) * 180.0 / M_PI;
            pushState("point towards mouse");
            break;
        }
        case BLOCK_GLIDE: {
            // Instant glide (no animation) - move to value, value2
            double oldX = sprite.scratchx, oldY = sprite.scratchy;
            sprite.scratchx = block.value; sprite.scratchy = block.value2;
            Clamp_Sprite_To_Stage_Bounds(sprite, mainStage);
            AddPenStroke(sprite, oldX, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("glide to x:" + to_string(block.value) + " y:" + to_string(block.value2));
            break;
        }
        case BLOCK_IF_EDGE_BOUNCE: {
            double oldX = sprite.scratchx, oldY = sprite.scratchy;
            double stageW = mainStage.rect.w / 2.0;
            double stageH = mainStage.rect.h / 2.0;
            double hW = sprite.rect.w / 2.0;
            double hH = sprite.rect.h / 2.0;

            // Boundaries: allow half of the sprite to go outside
            double leftBound   = -stageW - hW/2;
            double rightBound  =  stageW + hW/2;
            double topBound    = -stageH - hH/2;
            double bottomBound =  stageH + hH/2;

            bool bounced = false;
            if (sprite.scratchx - hW < leftBound) {
                sprite.scratchx = leftBound + hW;
                sprite.direction = 180 - sprite.direction;
                bounced = true;
            }
            if (sprite.scratchx + hW > rightBound) {
                sprite.scratchx = rightBound - hW;
                sprite.direction = 180 - sprite.direction;
                bounced = true;
            }
            if (sprite.scratchy - hH < topBound) {
                sprite.scratchy = topBound + hH;
                sprite.direction = -sprite.direction;
                bounced = true;
            }
            if (sprite.scratchy + hH > bottomBound) {
                sprite.scratchy = bottomBound - hH;
                sprite.direction = -sprite.direction;
                bounced = true;
            }
            sprite.direction = fmod(sprite.direction, 360.0);
            if (sprite.direction < 0) sprite.direction += 360.0;
            AddPenStroke(sprite, oldX, oldY, sprite.scratchx, sprite.scratchy, renderer);
            pushState("if on edge, bounce");
            break;
        }
        case BLOCK_PEN_DOWN: sprite.penDown = true; pushState("pen down"); break;
        case BLOCK_PEN_UP: sprite.penDown = false; pushState("pen up"); break;
        case BLOCK_SET_PEN_COLOR:
            sprite.penColor.r = (block.value >> 16) & 0xFF;
            sprite.penColor.g = (block.value >> 8) & 0xFF;
            sprite.penColor.b = block.value & 0xFF;
            pushState("set pen color");
            break;
        case BLOCK_CHANGE_PEN_SIZE:
            sprite.penSize += block.value;
            pushState("change pen size by " + to_string(block.value));
            break;
        case BLOCK_SET_PEN_SIZE:
            sprite.penSize = block.value;
            pushState("set pen size to " + to_string(block.value));
            break;
        case BLOCK_ERASE_ALL:
            sprite.penStrokes.clear();
            sprite.stamps.clear();
            pushState("erase all");
            break;
        case BLOCK_STAMP: {
            StampInfo stamp;
            int cx = mainStage.rect.x + mainStage.rect.w/2;
            int cy = mainStage.rect.y + mainStage.rect.h/2;
            stamp.x = cx + (int)sprite.scratchx;
            stamp.y = cy - (int)sprite.scratchy;
            stamp.costumeIndex = sprite.currentCostumeIndex;
            stamp.size = sprite.size;
            stamp.direction = sprite.direction;
            sprite.stamps.push_back(stamp);
            pushState("stamp");
            break;
        }
        case BLOCK_PLAY_SOUND:
            if (catSound) {
                Mix_VolumeMusic(MIX_MAX_VOLUME * soundVolume / 100);
                Mix_PlayMusic(catSound, 0);
            }
            pushState("play sound");
            break;
        case BLOCK_STOP_ALL_SOUNDS:
            Mix_HaltMusic();
            pushState("stop all sounds");
            break;
        case BLOCK_SET_VOLUME:
            soundVolume = block.value;
            if (soundVolume < 0) soundVolume = 0;
            if (soundVolume > 100) soundVolume = 100;
            Mix_VolumeMusic(MIX_MAX_VOLUME * soundVolume / 100);
            pushState("set volume to " + to_string(block.value) + "%");
            break;
        case BLOCK_CHANGE_VOLUME:
            soundVolume += block.value;
            if (soundVolume < 0) soundVolume = 0;
            if (soundVolume > 100) soundVolume = 100;
            Mix_VolumeMusic(MIX_MAX_VOLUME * soundVolume / 100);
            pushState("change volume by " + to_string(block.value));
            break;
        case BLOCK_SET_PITCH:
            soundPitch = block.value;
            pushState("set pitch to " + to_string(block.value) + "%");
            break;
        case BLOCK_CHANGE_PITCH:
            soundPitch += block.value;
            pushState("change pitch by " + to_string(block.value));
            break;
        case BLOCK_SAY:
            sprite.message = block.strValue;
            sprite.messageTimer = 2 * FPS;
            sprite.isThinking = false;
            pushState("say " + block.strValue);
            break;
        case BLOCK_SAY_FOR:
            sprite.message = block.strValue;
            sprite.messageTimer = block.value * FPS;
            sprite.isThinking = false;
            pushState("say " + block.strValue + " for " + to_string(block.value) + " secs");
            break;
        case BLOCK_THINK:
            sprite.message = block.strValue;
            sprite.messageTimer = 2 * FPS;
            sprite.isThinking = true;
            pushState("think " + block.strValue);
            break;
        case BLOCK_THINK_FOR:
            sprite.message = block.strValue;
            sprite.messageTimer = block.value * FPS;
            sprite.isThinking = true;
            pushState("think " + block.strValue + " for " + to_string(block.value) + " secs");
            break;
        case BLOCK_SHOW:
            sprite.isVisible = true;
            pushState("show");
            break;
        case BLOCK_HIDE:
            sprite.isVisible = false;
            pushState("hide");
            break;
        case BLOCK_SET_SIZE:
            sprite.size = block.value;
            pushState("set size to " + to_string(block.value) + "%");
            break;
        case BLOCK_CHANGE_SIZE:
            sprite.size += block.value;
            pushState("change size by " + to_string(block.value));
            break;
        // Operators using expression evaluator
        case BLOCK_ADD:
        case BLOCK_SUBTRACT:
        case BLOCK_MULTIPLY:
        case BLOCK_DIVIDE:
        case BLOCK_MOD:
        case BLOCK_LESS_THAN:
        case BLOCK_EQUAL:
        case BLOCK_GREATER_THAN:
        case BLOCK_AND:
        case BLOCK_OR:
        case BLOCK_NOT:
        case BLOCK_RANDOM:
        case BLOCK_ROUND:
        case BLOCK_ABS:
        case BLOCK_SQRT:
        case BLOCK_JOIN:
        case BLOCK_LETTER_OF:
        case BLOCK_LENGTH:
        case BLOCK_CONTAINS:
            // Evaluate expression and store result in lastOperatorResult
            // For special operators, we may need custom parsing, but evalExpr handles basic binary ops.
            // We'll handle each individually.
            if (block.type == BLOCK_ADD || block.type == BLOCK_SUBTRACT || block.type == BLOCK_MULTIPLY ||
                block.type == BLOCK_DIVIDE || block.type == BLOCK_MOD || block.type == BLOCK_LESS_THAN ||
                block.type == BLOCK_EQUAL || block.type == BLOCK_GREATER_THAN || block.type == BLOCK_AND ||
                block.type == BLOCK_OR || block.type == BLOCK_NOT) {
                lastOperatorResult = (int)evalExpr(block.strValue);
            }
            else if (block.type == BLOCK_RANDOM) {
                // parse "min,max"
                size_t comma = block.strValue.find(',');
                if (comma != string::npos) {
                    float minVal = evalExpr(block.strValue.substr(0, comma));
                    float maxVal = evalExpr(block.strValue.substr(comma + 1));
                    if (maxVal < minVal) swap(minVal, maxVal);
                    lastOperatorResult = minVal + (rand() / (float)RAND_MAX) * (maxVal - minVal);
                } else {
                    lastOperatorResult = evalExpr(block.strValue);
                }
            }
            else if (block.type == BLOCK_ROUND) {
                lastOperatorResult = (int)round(evalExpr(block.strValue));
            }
            else if (block.type == BLOCK_ABS) {
                lastOperatorResult = abs((int)evalExpr(block.strValue));
            }
            else if (block.type == BLOCK_SQRT) {
                float val = evalExpr(block.strValue);
                if (val >= 0) lastOperatorResult = (int)sqrt(val);
                else { lastOperatorResult = 0; setError("Square root of negative number"); }
            }
            else if (block.type == BLOCK_JOIN) {
                size_t comma = block.strValue.find(',');
                if (comma != string::npos) {
                    string s1 = block.strValue.substr(0, comma);
                    string s2 = block.strValue.substr(comma + 1);
                    // evaluate? No, we want string concatenation. But our blocks store strings, not expressions.
                    // For simplicity, we treat as literal strings.
                    string result = s1 + s2;
                    lastOperatorResult = 0; // Not used
                    // Store in a special variable? Not implemented.
                }
            }
            else if (block.type == BLOCK_LETTER_OF) {
                size_t comma = block.strValue.find(',');
                if (comma != string::npos) {
                    int idx = (int)evalExpr(block.strValue.substr(0, comma));
                    string str = block.strValue.substr(comma + 1);
                    if (idx >= 1 && idx <= (int)str.length())
                        lastOperatorResult = str[idx-1];
                    else {
                        lastOperatorResult = 0;
                        setError("Letter index out of range");
                    }
                }
            }
            else if (block.type == BLOCK_LENGTH) {
                lastOperatorResult = block.strValue.length();
            }
            else if (block.type == BLOCK_CONTAINS) {
                size_t comma = block.strValue.find(',');
                if (comma != string::npos) {
                    string s1 = block.strValue.substr(0, comma);
                    string s2 = block.strValue.substr(comma + 1);
                    lastOperatorResult = (s1.find(s2) != string::npos) ? 1 : 0;
                }
            }
            pushState("Operator executed");
            break;

        case BLOCK_DEFINE_VARIABLE:
            defineVariable(block.strValue);
            break;
        case BLOCK_SET_VAR: {
            string varName, expr;
            if (parseAssignment(block.strValue, varName, expr)) {
                int idx = findVariable(varName);
                if (idx != -1) {
                    variables[idx].value = (int)evalExpr(expr);
                } else {
                    setError("Variable not defined: " + varName);
                }
            } else {
                setError("Invalid assignment format. Use var=expr");
            }
            pushState("set variable " + block.strValue);
            break;
        }
        case BLOCK_CHANGE_VAR: {
            string varName, expr;
            if (parseAssignment(block.strValue, varName, expr)) {
                int idx = findVariable(varName);
                if (idx != -1) {
                    variables[idx].value += (int)evalExpr(expr);
                } else {
                    setError("Variable not defined: " + varName);
                }
            } else {
                setError("Invalid assignment format. Use var=expr");
            }
            pushState("change variable " + block.strValue);
            break;
        }
        case BLOCK_SHOW_VARIABLE: {
            int idx = findVariable(block.strValue);
            if (idx != -1) variables[idx].isShown = true;
            pushState("show variable " + block.strValue);
            break;
        }
        case BLOCK_HIDE_VARIABLE: {
            int idx = findVariable(block.strValue);
            if (idx != -1) variables[idx].isShown = false;
            pushState("hide variable " + block.strValue);
            break;
        }
        case BLOCK_VAR:
            {
                int idx = findVariable(block.strValue);
                lastOperatorResult = (idx != -1) ? variables[idx].value : 0;
            }
            break;
        case BLOCK_ASK:
            sprite.message = block.strValue;
            sprite.messageTimer = 2 * FPS;
            waitingForAnswer = true;
            SDL_StartTextInput();
            pushState("ask " + block.strValue);
            break;
        case BLOCK_KEY_PRESSED: {
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            bool pressed = false;
            if (block.strValue == "space") pressed = keystate[SDL_SCANCODE_SPACE];
            else if (block.strValue == "up") pressed = keystate[SDL_SCANCODE_UP];
            else if (block.strValue == "down") pressed = keystate[SDL_SCANCODE_DOWN];
            else if (block.strValue == "left") pressed = keystate[SDL_SCANCODE_LEFT];
            else if (block.strValue == "right") pressed = keystate[SDL_SCANCODE_RIGHT];
            lastOperatorResult = pressed ? 1 : 0;
            break;
        }
        case BLOCK_MOUSE_DOWN:
            lastOperatorResult = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;
            break;
        case BLOCK_DISTANCE_TO: {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            int cx = mainStage.rect.x + mainStage.rect.w/2;
            int cy = mainStage.rect.y + mainStage.rect.h/2;
            double dx = (mx - cx) - sprite.scratchx;
            double dy = (cy - my) - sprite.scratchy;
            lastOperatorResult = (int)sqrt(dx*dx + dy*dy);
            break;
        }
        case BLOCK_BROADCAST:
            {
                string msg = block.strValue;
                vector<ScriptState> newReceivers;
                for (auto& b : scriptBlocks) {
                    if (b->type == BLOCK_WHEN_RECEIVE && b->strValue == msg && !b->prev.lock()) {
                        ScriptState s;
                        s.current = b->next;
                        s.waitFrames = 0;
                        s.waitingForBroadcast = "";
                        s.waitingChildren = 0;
                        newReceivers.push_back(s);
                    }
                }
                if (!newReceivers.empty()) {
                    messageScripts[msg].insert(messageScripts[msg].end(), newReceivers.begin(), newReceivers.end());
                    scriptsRunning = true;
                }
                pushState("broadcast " + msg);
            }
            break;
        default:
            break;
    }
}
