void setError(const string& msg) {
    errorMessage = msg;
    errorMessageTimer = SDL_GetTicks() + ERROR_MESSAGE_DURATION;
}

void addHistory(const string& desc, int stateIdx) {
    historyLog.push_back({desc, stateIdx});
    if (historyLog.size() > MAX_HISTORY) {
        historyLog.erase(historyLog.begin());
    }
}

void logAction(const string& msg, int stateIdx = -1) {
    lastNormalLog = msg;
    if (stateIdx >= 0) {
        addHistory(msg, stateIdx);
    }
}

string getOpenFileName() {
    const char* filters[] = { "*.txt" };
    const char* filename = tinyfd_openFileDialog("Open Project", "", 1, filters, "Text Files", 0);
    return filename ? string(filename) : "";
}

string getSaveFileName() {
    const char* filters[] = { "*.txt" };
    const char* filename = tinyfd_saveFileDialog("Save Project", "project.txt", 1, filters, "Text Files");
    return filename ? string(filename) : "";
}

// Helper function to get color based on category
SDL_Color getCategoryColor(Category c) {
    switch(c) {
        case CAT_MOTION:    return {74,155,252,255};
        case CAT_LOOKS:     return {154,106,255,255};
        case CAT_SOUND:     return {207,99,207,255};
        case CAT_EVENTS:    return {255,213,25,255};
        case CAT_CONTROL:   return {255,171,25,255};
        case CAT_SENSING:   return {85,180,250,255};
        case CAT_OPERATORS: return {89,205,105,255};
        case CAT_VARIABLES: return {255,140,26,255};
        case CAT_PEN:       return {0,200,0,255};
        case CAT_MYBLOCKS:  return {255,0,0,255};
        default:            return {255,255,255,255};
    }
}

void saveProject(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        setError("Failed to save file");
        return;
    }
    // version 2: supports value2 for GOTO_XY/GLIDE, strValue for arrow KEY_PRESSED, POINT_TO_MOUSE
    file << "#ScratchProject 2\n";
    Sprite& sp = allSprites[0];
    file << "Sprite " << sp.scratchx << " " << sp.scratchy << " " << sp.direction << " " << sp.size << " "
         << sp.isVisible << " " << sp.penDown << " " << (int)sp.penColor.r << " " << (int)sp.penColor.g << " " << (int)sp.penColor.b << " " << sp.penSize << "\n";
    file << "Backdrop " << mainStage.currentBackdropIndex << "\n";
    file << "Variables " << variables.size() << "\n";
    for (auto& v : variables) {
        file << "Var " << v.name << " " << v.value << " " << v.isShown << "\n";
    }
    file << "Blocks " << scriptBlocks.size() << "\n";
    map<shared_ptr<Block>, int> idMap;
    for (size_t i = 0; i < scriptBlocks.size(); i++) {
        idMap[scriptBlocks[i]] = i;
    }
    for (size_t i = 0; i < scriptBlocks.size(); i++) {
        auto& b = scriptBlocks[i];
        int nextId = (b->next ? idMap[b->next] : -1);
        file << "Block " << i << " " << (int)b->type << " " << (int)b->category << " "
             << "|" << b->baseLabel << "|" << "|" << b->label << "|" << "|" << b->strValue << "|"
             << " " << b->value << " " << b->value2
             << " " << b->rect.x << " " << b->rect.y << " " << b->rect.w << " " << b->rect.h
             << " " << nextId << "\n";
    }
    file.close();
    logAction("Project saved to " + filename);
}

void loadProject(const string& filename, SDL_Renderer* renderer) {
    ifstream file(filename);
    if (!file.is_open()) {
        setError("Failed to load file");
        return;
    }
    string header;
    int version;
    file >> header >> version;
    if (header != "#ScratchProject" || (version != 1 && version != 2)) {
        setError("Invalid or unsupported project file (expected version 1 or 2)");
        return;
    }
    scriptBlocks.clear();
    variables.clear();

    string token;
    file >> token;
    Sprite& sp = allSprites[0];
    file >> sp.scratchx >> sp.scratchy >> sp.direction >> sp.size >> sp.isVisible >> sp.penDown;
    int r, g, b;
    file >> r >> g >> b >> sp.penSize;
    sp.penColor = { (Uint8)r, (Uint8)g, (Uint8)b, 255 };

    file >> token;
    file >> mainStage.currentBackdropIndex;

    file >> token;
    int varCount;
    file >> varCount;
    for (int i = 0; i < varCount; i++) {
        file >> token;
        Variable v;
        file >> v.name >> v.value >> v.isShown;
        variables.push_back(v);
    }

    file >> token;
    int blockCount;
    file >> blockCount;
    vector<shared_ptr<Block>> blocks(blockCount);
    vector<int> nextIds(blockCount);
    for (int i = 0; i < blockCount; i++) {
        file >> token; // "Block"
        int id, typeInt, catInt;
        file >> id >> typeInt >> catInt;
        auto b = make_shared<Block>();
        b->type = (BlockType)typeInt;
        b->category = (Category)catInt;

        auto readPiped = [&]() {
            string s; char c;
            file >> c;
            getline(file, s, '|');
            return s;
        };
        b->baseLabel = readPiped();
        b->label     = readPiped();
        b->strValue  = readPiped();
        file >> b->value >> b->value2 >> b->rect.x >> b->rect.y >> b->rect.w >> b->rect.h >> nextIds[i];
        b->color = getCategoryColor((Category)catInt);
        b->inPalette = false;
        b->textTexture = nullptr;
        b->next = nullptr;
        blocks[id] = b;
    }
    for (int i = 0; i < blockCount; i++) {
        if (nextIds[i] != -1) {
            blocks[i]->next = blocks[nextIds[i]];
        }
    }
    for (auto& b : blocks) {
        if (b->next) b->next->prev = b;
    }
    for (auto& b : blocks) {
        UpdateBlockTexture(b, renderer);
    }
    scriptBlocks = blocks;

    logAction("Project loaded from " + filename);
    pushState("Loaded project");
}
