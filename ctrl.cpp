int EvaluateCondition(shared_ptr<Block> condBlock, Sprite& sprite) {
    if (!condBlock) return lastOperatorResult;
    switch (condBlock->type) {
        case BLOCK_KEY_PRESSED: {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            for (int i = 0; i < 5; i++) {
                if (condBlock->strValue == keyNames[i])
                    return keys[keyScancodes[i]] ? 1 : 0;
            }
            return 0;
        }
        case BLOCK_MOUSE_DOWN:
            return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) ? 1 : 0;
        case BLOCK_TOUCHING: {
            return 0;
        }
        case BLOCK_LESS_THAN:
            return (condBlock->value < condBlock->value2) ? 1 : 0;
        case BLOCK_GREATER_THAN:
            return (condBlock->value > condBlock->value2) ? 1 : 0;
        case BLOCK_EQUAL:
            return (condBlock->value == condBlock->value2) ? 1 : 0;
        case BLOCK_AND:
            return (condBlock->value != 0 && condBlock->value2 != 0) ? 1 : 0;
        case BLOCK_OR:
            return (condBlock->value != 0 || condBlock->value2 != 0) ? 1 : 0;
        case BLOCK_NOT:
            return (condBlock->value == 0) ? 1 : 0;
        default:
            return lastOperatorResult;
    }
}

void ExecuteScripts(SDL_Renderer* renderer) {
    if (!scriptsRunning) return;
    if (isPaused && !stepRequested) return;
    bool stepMode = stepRequested;
    if (stepRequested) stepRequested = false;

    // Watchdog:
    auto resetWatchdog = [](vector<ScriptState>& scripts) {
        for (auto& s : scripts)
            for (auto& f : s.stack)
                f.iterCount = 0;
    };
    resetWatchdog(flagScripts);
    resetWatchdog(spaceScripts);
    resetWatchdog(clickScripts);
    for (auto& pair : messageScripts) resetWatchdog(pair.second);

    bool stepDone = false;
    auto stepScripts = [&](vector<ScriptState>& scripts) {
        for (auto it = scripts.begin(); it != scripts.end(); ) {
            auto& s = *it;
            if (s.waitFrames > 0) {
                s.waitFrames--;
                ++it;
                continue;
            }
            if (!s.current) {
                it = scripts.erase(it);
                continue;
            }
            Block& block = *s.current;
            Sprite& sprite = allSprites[0];

            // Track currently executing block for step mode highlight
            if (stepModeActive) {
                currentExecutingBlock = s.current;
            }

            if (!s.waitingForBroadcast.empty()) {
                auto& children = messageScripts[s.waitingForBroadcast];
                if (children.empty()) {
                    s.waitingForBroadcast = "";
                    s.waitingChildren = 0;
                } else {
                    ++it;
                    continue;
                }
            }

            switch (block.type) {
                case BLOCK_REPEAT: {
                    int depth = 1;
                    auto end = block.next;
                    while (end) {
                        if (end->type == BLOCK_REPEAT || end->type == BLOCK_FOREVER || end->type == BLOCK_IF || end->type == BLOCK_IF_ELSE) depth++;
                        if (end->type == BLOCK_END) { depth--; if (depth == 0) break; }
                        end = end->next;
                    }
                    if (end) {
                        CallFrame f;
                        f.returnTo = end->next;
                        f.loopStart = block.next;
                        f.remaining = block.value;
                        s.stack.push_back(f);
                        s.current = block.next;
                    } else {
                        s.current = block.next;
                    }
                    break;
                }
                case BLOCK_FOREVER: {
                    int depth = 1;
                    auto end = block.next;
                    while (end) {
                        if (end->type == BLOCK_REPEAT || end->type == BLOCK_FOREVER || end->type == BLOCK_IF || end->type == BLOCK_IF_ELSE) depth++;
                        if (end->type == BLOCK_END) { depth--; if (depth == 0) break; }
                        end = end->next;
                    }
                    if (end) {
                        CallFrame f;
                        f.returnTo = nullptr;
                        f.loopStart = block.next;
                        f.remaining = 0;
                        s.stack.push_back(f);
                        s.current = block.next;
                    } else {
                        s.current = block.next;
                    }
                    break;
                }
                case BLOCK_IF: {
                    bool cond = (EvaluateCondition(s.conditionBlock, sprite) != 0);
                    int depth = 1;
                    auto end = block.next;
                    while (end) {
                        if (end->type == BLOCK_REPEAT || end->type == BLOCK_FOREVER || end->type == BLOCK_IF || end->type == BLOCK_IF_ELSE) depth++;
                        if (end->type == BLOCK_END) { depth--; if (depth == 0) break; }
                        end = end->next;
                    }
                    if (cond) {
                        s.current = block.next;
                    } else {
                        s.current = end ? end->next : nullptr;
                    }
                    s.conditionBlock = nullptr;
                    break;
                }
                case BLOCK_IF_ELSE: {
                    bool cond = (EvaluateCondition(s.conditionBlock, sprite) != 0);
                    // Find first END (separates if-branch from else-branch)
                    int depth = 1;
                    auto firstEnd = block.next;
                    while (firstEnd) {
                        if (firstEnd->type == BLOCK_REPEAT || firstEnd->type == BLOCK_FOREVER ||
                            firstEnd->type == BLOCK_IF || firstEnd->type == BLOCK_IF_ELSE) depth++;
                        if (firstEnd->type == BLOCK_END) { depth--; if (depth == 0) break; }
                        firstEnd = firstEnd->next;
                    }
                    // Find second END (end of else-branch)
                    shared_ptr<Block> secondEnd = nullptr;
                    if (firstEnd && firstEnd->next) {
                        depth = 1;
                        secondEnd = firstEnd->next;
                        // Check if next block after first END starts another nested structure or is content
                        // We need to find the matching END for the else-branch
                        // The else-branch ends at the next unnested END
                        depth = 0;
                        auto cur = firstEnd->next;
                        while (cur) {
                            if (cur->type == BLOCK_REPEAT || cur->type == BLOCK_FOREVER ||
                                cur->type == BLOCK_IF || cur->type == BLOCK_IF_ELSE) depth++;
                            if (cur->type == BLOCK_END) {
                                if (depth == 0) { secondEnd = cur; break; }
                                depth--;
                            }
                            cur = cur->next;
                        }
                        if (!cur) secondEnd = nullptr; // no second END found
                    }
                    if (cond) {
                        // Execute if-branch; when reaching firstEnd, push frame to skip to after secondEnd
                        if (firstEnd) {
                            CallFrame f;
                            f.returnTo = secondEnd ? secondEnd->next : nullptr;
                            f.loopStart = nullptr;
                            f.remaining = -1; // special: skip marker, not a loop
                            s.stack.push_back(f);
                        }
                        s.current = block.next;
                    } else {
                        // Skip to else-branch (after firstEnd)
                        if (firstEnd) {
                            if (secondEnd) {
                                // Push frame so when we hit secondEnd, we jump past it
                                CallFrame f;
                                f.returnTo = secondEnd->next;
                                f.loopStart = nullptr;
                                f.remaining = -1; // special: skip marker
                                s.stack.push_back(f);
                                s.current = firstEnd->next; // start of else-branch
                            } else {
                                s.current = firstEnd->next;
                            }
                        } else {
                            s.current = nullptr;
                        }
                    }
                    s.conditionBlock = nullptr;
                    break;
                }
                case BLOCK_END: {
                    if (!s.stack.empty()) {
                        auto& f = s.stack.back();
                        if (f.remaining == -1) {
                            // Special IF_ELSE frame: jump to returnTo (skip remaining branch)
                            s.current = f.returnTo;
                            s.stack.pop_back();
                        } else if (f.returnTo == nullptr) {
                            // FOREVER loop
                            f.iterCount++;
                            if (f.iterCount > WATCHDOG_MAX_ITERS_PER_FRAME) {
                                // watchdog:
                                setError("Watchdog: infinite loop detected, forcing frame break");
                                f.iterCount = 0;
                                s.waitFrames = 1;
                                s.current = f.loopStart;
                                ++it;
                                if (stepMode) { stepDone = true; return; }
                                continue;
                            }
                            s.current = f.loopStart;
                        } else if (f.remaining > 0) {
                            f.iterCount++;
                            if (f.iterCount > WATCHDOG_MAX_ITERS_PER_FRAME) {
                                setError("Watchdog: repeat loop too heavy, forcing frame break");
                                f.iterCount = 0;
                                s.waitFrames = 1;
                                s.current = f.loopStart;
                                ++it;
                                if (stepMode) { stepDone = true; return; }
                                continue;
                            }
                            f.remaining--;
                            if (f.remaining > 0) s.current = f.loopStart;
                            else {
                                s.current = f.returnTo;
                                s.stack.pop_back();
                            }
                        } else {
                            s.current = f.returnTo;
                            s.stack.pop_back();
                        }
                    } else {
                        s.current = s.current->next;
                    }
                    break;
                }
                case BLOCK_WAIT:
                    s.waitFrames = block.value * FPS;
                    s.current = s.current->next;
                    break;
                case BLOCK_STOP_ALL:
                    scriptsRunning = false; isPaused = false; stepRequested = false;
                    flagScripts.clear(); spaceScripts.clear(); clickScripts.clear(); messageScripts.clear();
                    return; // exit ExecuteScripts entirely
                case BLOCK_WAIT_UNTIL: {

                    shared_ptr<Block> condSrc = s.conditionBlock;
                    if (!condSrc) {
                        auto prev = s.current->prev.lock();
                        if (prev && (prev->type == BLOCK_KEY_PRESSED || prev->type == BLOCK_MOUSE_DOWN ||
                                     prev->type == BLOCK_TOUCHING || prev->type == BLOCK_LESS_THAN ||
                                     prev->type == BLOCK_EQUAL || prev->type == BLOCK_GREATER_THAN ||
                                     prev->type == BLOCK_AND || prev->type == BLOCK_OR || prev->type == BLOCK_NOT)) {
                            condSrc = prev;
                            s.conditionBlock = prev;
                        }
                    }
                    bool cond = (EvaluateCondition(condSrc, sprite) != 0);
                    if (cond) {
                        s.current = s.current->next;
                        s.conditionBlock = nullptr;
                    }
                    break;
                }
                case BLOCK_BROADCAST_WAIT: {
                    string msg = block.strValue;
                    vector<ScriptState> newReceivers;
                    for (auto& b : scriptBlocks) {
                        if (b->type == BLOCK_WHEN_RECEIVE && b->strValue == msg && !b->prev.lock()) {
                            ScriptState newS;
                            newS.current = b->next;
                            newS.waitFrames = 0;
                            newS.waitingForBroadcast = "";
                            newS.waitingChildren = 0;
                            newReceivers.push_back(newS);
                        }
                    }
                    if (!newReceivers.empty()) {
                        messageScripts[msg].insert(messageScripts[msg].end(), newReceivers.begin(), newReceivers.end());
                        scriptsRunning = true;
                        s.waitingForBroadcast = msg;
                        s.waitingChildren = newReceivers.size();
                    }
                    break;
                }
                default:

                    if (block.type == BLOCK_KEY_PRESSED || block.type == BLOCK_MOUSE_DOWN ||
                        block.type == BLOCK_TOUCHING || block.type == BLOCK_TOUCHING_COLOR ||
                        block.type == BLOCK_LESS_THAN || block.type == BLOCK_EQUAL ||
                        block.type == BLOCK_GREATER_THAN || block.type == BLOCK_AND ||
                        block.type == BLOCK_OR || block.type == BLOCK_NOT) {
                        s.conditionBlock = s.current;
                    } else {
                        s.conditionBlock = nullptr;
                    }
                    ExecuteBlock(block, sprite, renderer);
                    logAction("Executed: " + block.label);
                    s.current = s.current->next;
                    if (stepMode) { stepDone = true; ++it; return; }
                    break;
            }
            ++it;
        }
    };

    stepScripts(flagScripts);
    if (!stepDone) stepScripts(spaceScripts);
    if (!stepDone) stepScripts(clickScripts);
    if (!stepDone) {
        for (auto& pair : messageScripts) {
            stepScripts(pair.second);
            if (stepDone) break;
        }
    }

    // In step mode, after executing one step, pause until next click
    if (stepModeActive && !stepRequested) {
        isPaused = true;
    }

    for (auto it = messageScripts.begin(); it != messageScripts.end(); ) {
        if (it->second.empty()) it = messageScripts.erase(it);
        else ++it;
    }

    for (auto& s : allSprites) {
        if (s.messageTimer > 0) {
            s.messageTimer--;
            if (s.messageTimer == 0) s.message.clear();
        }
    }
}
