// Scratch Editor - Complete with fixed backdrop textures and improved UI
// Compile: g++ -std=c++11 -o scratch_editor scratch_editor_final.cpp tinyfiledialogs.c -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lSDL2_gfx -lcomdlg32 -lole32 -lm

#define _USE_MATH_DEFINES
#include <windows.h>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfx.h>
#include <SDL2/SDL_syswm.h>
#include <string>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <map>
#include "tinyfiledialogs.h"
#include <cstdio>
#include <iomanip>
using namespace std;

// ==================== ENUMS ====================
enum ButtonID {
    BTN_Play, BTN_Stop, BTN_Pause, BTN_Step,
    BTN_Undo, BTN_Redo,
    BTN_File, BTN_NewFile,
    BTN_LoadFile, BTN_SaveFile, BTN_Edit, BTN_Tutorial, BTN_Motion, BTN_Looks,
    BTN_Sound, BTN_Events, BTN_Control, BTN_Sensing, BTN_Operators,
    BTN_Variables, BTN_Pen, BTN_MyBlocks, BTN_QuestionMark, BTN_About, BTN_AddExt,
    BTN_SChoose, BTN_AddBack,
    BTN_Restore, BTN_Turbo, BTN_PP, BTN_DataSet,
    BTN_TempSave,
    BTN_TempLoad,
};

enum BlockType {
    // Motion
    BLOCK_MOVE_STEPS, BLOCK_TURN_RIGHT, BLOCK_TURN_LEFT, BLOCK_GOTO_XY, BLOCK_GOTO_RANDOM,
    BLOCK_GLIDE,
    BLOCK_CHANGE_X, BLOCK_SET_X, BLOCK_CHANGE_Y, BLOCK_SET_Y, BLOCK_SET_DIR,
    BLOCK_IF_EDGE_BOUNCE, BLOCK_POINT_TO_MOUSE,
    // Looks
    BLOCK_SAY, BLOCK_SAY_FOR, BLOCK_THINK, BLOCK_THINK_FOR, BLOCK_SHOW, BLOCK_HIDE,
    BLOCK_SET_SIZE, BLOCK_CHANGE_SIZE,
    // Sound
    BLOCK_PLAY_SOUND, BLOCK_STOP_ALL_SOUNDS,
    BLOCK_SET_VOLUME, BLOCK_CHANGE_VOLUME, BLOCK_SET_PITCH, BLOCK_CHANGE_PITCH,
    // Events
    BLOCK_WHEN_FLAG, BLOCK_WHEN_KEY, BLOCK_WHEN_CLICKED, BLOCK_WHEN_RECEIVE,
    BLOCK_BROADCAST, BLOCK_BROADCAST_WAIT,
    // Control
    BLOCK_WAIT, BLOCK_REPEAT, BLOCK_FOREVER, BLOCK_IF, BLOCK_IF_ELSE, BLOCK_WAIT_UNTIL, BLOCK_STOP_ALL,
    BLOCK_END,
    // Sensing
    BLOCK_TOUCHING, BLOCK_TOUCHING_COLOR, BLOCK_DISTANCE_TO, BLOCK_ASK, BLOCK_KEY_PRESSED, BLOCK_MOUSE_DOWN,
    // Operators
    BLOCK_ADD, BLOCK_SUBTRACT, BLOCK_MULTIPLY, BLOCK_DIVIDE, BLOCK_RANDOM,
    BLOCK_LESS_THAN, BLOCK_EQUAL, BLOCK_GREATER_THAN, BLOCK_AND, BLOCK_OR, BLOCK_NOT,
    // Extra operators
    BLOCK_MOD, BLOCK_ROUND, BLOCK_ABS, BLOCK_SQRT,
    BLOCK_JOIN, BLOCK_LETTER_OF, BLOCK_LENGTH, BLOCK_CONTAINS,
    // Variables
    BLOCK_DEFINE_VARIABLE, BLOCK_SET_VAR, BLOCK_CHANGE_VAR, BLOCK_VAR, BLOCK_SHOW_VARIABLE, BLOCK_HIDE_VARIABLE,
    // Pen
    BLOCK_PEN_DOWN, BLOCK_PEN_UP, BLOCK_SET_PEN_COLOR, BLOCK_CHANGE_PEN_SIZE, BLOCK_SET_PEN_SIZE,
    BLOCK_ERASE_ALL, BLOCK_STAMP,
    // My Blocks
    BLOCK_MYBLOCK_DEFINE, BLOCK_MYBLOCK_CALL,
};

enum Category { CAT_MOTION, CAT_LOOKS, CAT_SOUND, CAT_EVENTS, CAT_CONTROL,
                CAT_SENSING, CAT_OPERATORS, CAT_VARIABLES, CAT_PEN, CAT_MYBLOCKS, CAT_COUNT };
enum ButtonState { Button_Normal, Button_Pressed, Button_Hovered };

// ==================== STRUCTS ====================
struct Button {
    SDL_Rect rect;
    SDL_Color Normal, Selected, hovered, textcolor;
    ButtonState state;
    ButtonID buttonID;
    string text;
    bool isselected = false;
    SDL_Texture *text_texture1 = nullptr, *text_texture2 = nullptr,
               *Picture1 = nullptr, *Picture2 = nullptr;
};

struct Panel {
    SDL_Rect rect;
    bool isactive = false;
    Button* btn;
    int count;
};

struct Costume {
    string name;
    SDL_Texture* texture;
    int width, height;
};

struct PenStroke {
    int x1, y1, x2, y2;
    int size;
    SDL_Color color;
};

struct StampInfo {
    int x, y;
    int costumeIndex;
    double size;
    double direction;
};

struct Block {
    BlockType type;
    Category category;
    string baseLabel, label, strValue;
    int value;
    int value2;
    SDL_Rect rect;
    SDL_Color color;
    bool isDragging = false;
    int dragOffX, dragOffY;
    bool inPalette;
    SDL_Texture* textTexture = nullptr;
    shared_ptr<Block> next;
    weak_ptr<Block> prev;

    bool hasEditableValue() const { return baseLabel.find("{}") != string::npos; }
    bool hasTwoEditableValues() const {
        return type == BLOCK_ADD || type == BLOCK_SUBTRACT || type == BLOCK_MULTIPLY || type == BLOCK_DIVIDE ||
               type == BLOCK_LESS_THAN || type == BLOCK_EQUAL || type == BLOCK_GREATER_THAN ||
               type == BLOCK_AND || type == BLOCK_OR || type == BLOCK_MOD || type == BLOCK_JOIN;
    }
    bool hasStringValue() const {
        return type == BLOCK_WHEN_KEY || type == BLOCK_BROADCAST || type == BLOCK_BROADCAST_WAIT || type == BLOCK_WHEN_RECEIVE ||
               type == BLOCK_LETTER_OF || type == BLOCK_LENGTH || type == BLOCK_CONTAINS ||
               type == BLOCK_SAY || type == BLOCK_SAY_FOR || type == BLOCK_THINK || type == BLOCK_THINK_FOR ||
               // operators now also have string value (expression)
               type == BLOCK_ADD || type == BLOCK_SUBTRACT || type == BLOCK_MULTIPLY || type == BLOCK_DIVIDE ||
               type == BLOCK_RANDOM || type == BLOCK_LESS_THAN || type == BLOCK_EQUAL || type == BLOCK_GREATER_THAN ||
               type == BLOCK_AND || type == BLOCK_OR || type == BLOCK_NOT ||
               type == BLOCK_MOD || type == BLOCK_ROUND || type == BLOCK_ABS || type == BLOCK_SQRT ||
               type == BLOCK_JOIN || type == BLOCK_DEFINE_VARIABLE || type == BLOCK_SET_VAR || type == BLOCK_CHANGE_VAR ||
               type == BLOCK_VAR;
    }
    bool isHat() const { return type == BLOCK_WHEN_FLAG || type == BLOCK_WHEN_KEY || type == BLOCK_WHEN_CLICKED || type == BLOCK_WHEN_RECEIVE; }
    Block() : next(nullptr), prev(), value2(0) {}

    shared_ptr<Block> clone() const {
        auto newBlock = make_shared<Block>();
        newBlock->type = type;
        newBlock->category = category;
        newBlock->baseLabel = baseLabel;
        newBlock->label = label;
        newBlock->strValue = strValue;
        newBlock->value = value;
        newBlock->value2 = value2;
        newBlock->rect = rect;
        newBlock->color = color;
        newBlock->inPalette = inPalette;
        newBlock->textTexture = nullptr;
        if (next) newBlock->next = next->clone();
        return newBlock;
    }
};

struct Variable {
    string name;
    int value;
    bool isShown;
    SDL_Rect rect;
};

struct Sprite {
    string name = "Sprite";
    vector<Costume> costumes;
    int currentCostumeIndex = 0;
    bool isVisible = true;
    double size = 100.0, direction = 0.0, scratchx = 0.0, scratchy = 0.0;
    SDL_Rect rect;
    bool isDraggable = true, isBeingDragged = false;
    int dragOffsetX = 0, dragOffsetY = 0;
    bool penDown = false;
    SDL_Color penColor;
    int penSize = 2;
    vector<PenStroke> penStrokes;
    vector<StampInfo> stamps;
    string message;
    int messageTimer = 0;
    bool isThinking = false;

    Sprite cloneState() const {
        Sprite s;
        s.name = name;
        s.currentCostumeIndex = currentCostumeIndex;
        s.isVisible = isVisible;
        s.size = size;
        s.direction = direction;
        s.scratchx = scratchx;
        s.scratchy = scratchy;
        s.rect = rect;
        s.isDraggable = isDraggable;
        s.penDown = penDown;
        s.penColor = penColor;
        s.penSize = penSize;
        s.message = message;
        s.messageTimer = messageTimer;
        s.isThinking = isThinking;
        return s;
    }
};

struct Stage {
    vector<Costume> backdrops;
    int currentBackdropIndex = 0;
    SDL_Rect rect;
};

void UpdateBlockTexture(shared_ptr<Block> block, SDL_Renderer* renderer);
void DrawToolbar(SDL_Renderer* r, SDL_Rect& t, SDL_Color c);
void RenderText(SDL_Renderer* r, int x, int y, const string& txt, SDL_Color col);


// ==================== CONSTANTS ====================
const int Toolbar_Height = 45, Blockbar_Width = 60, Empty_Height = 45,
          BLOCK_HEIGHT = 40, SNAP_DIST = 20, GAP = 2, DRAG_THRESHOLD = 5,
          FPS = 60;

SDL_Rect toolbar = {0,0,1280,45}, BlockBar = {0,90,60,660}, BlocksFuncs = {60,90,240,700},
         Plate = {300,90,480,700}, Stage1 = {787,90,487,330}, BackDrop_List = {1194,426,80,300},
         Sprite_Info = {787,426,401,300}, Sprite_List = {787,511,401,300},
         File_Panel = {100,45,200,90}, Edit_Panel = {145,45,200,60}, Help_Panel = {1080,45,200,90};

// Colors
SDL_Color Blue = {77,151,255,255}, DarkerBlue = {66,128,217,255}, Background = {229,240,255,255},
          White = {255,255,255,255}, Gray = {200,200,200,255}, Purple = {128,0,128,255},
          Pink = {255,0,255,255}, Yellow = {255,255,0,255}, Red = {255,0,0,255}, Green = {0,255,0,255},
          DarkerGreen = {0,200,0,255}, Black = {0,0,0,255}, Orange = {255,165,0,255},
          LightBlue = {135,206,250,255}, LightBlue_1 = {135,206,250,122}, DarkOrange = {255,140,0,255};

SDL_Color MotionColor = {74,155,252,255}, LooksColor = {154,106,255,255}, SoundColor = {207,99,207,255},
          EventsColor = {255,213,25,255}, ControlColor = {255,171,25,255}, SensingColor = {85,180,250,255},
          OperatorsColor = {89,205,105,255}, VariablesColor = {255,140,26,255}, PenColor = {0,200,0,255},
          MyBlocksColor = {255,0,0,255};

int MOUUSE_X, MOUUSE_Y;
HWND g_hwnd = NULL;
SDL_Renderer* g_renderer = nullptr;  // global renderer for button callbacks

// UI elements
Button ToolBar_Button[4], BlockBar_Button[11], File_Panel_Button[3], Edit_Panel_Button[2], Help_Panel_Button[3];
Panel File_BTN_Panel, Edit_BTN_Panel, Help_BTN_Panel;
Stage mainStage;
vector<Button*> allButtons;
vector<Sprite> allSprites;
vector<Block> paletteBlocks;
vector<shared_ptr<Block>> scriptBlocks;
SDL_Texture* Scratch_Logo = nullptr;

// Special buttons
Button Sprite_Choosing = { {1125,660,50,50}, Blue, Blue, DarkerGreen, DarkerGreen, Button_Normal, BTN_SChoose, "", false };
Button Add_BackDrop   = { {1209,660,50,50}, Blue, Blue, DarkerGreen, DarkerGreen, Button_Normal, BTN_AddBack, "", false };
Button Go = { {787,55,24,24}, Background, LightBlue, LightBlue_1, Black, Button_Normal, BTN_Play, "", false };
Button Stop = { {814,55,24,24}, Background, Background, LightBlue_1, Black, Button_Normal, BTN_Stop, "", false };
Button Pause = { {841,55,24,24}, Background, LightBlue, LightBlue_1, Black, Button_Normal, BTN_Pause, "||", false };
Button StepBtn = { {868,55,24,24}, Background, LightBlue, LightBlue_1, Black, Button_Normal, BTN_Step, "->", false };
Button UndoBtn = { {895,8,30,30}, Background, LightBlue, LightBlue_1, Black, Button_Normal, BTN_Undo, "↩", false };
Button RedoBtn = { {930,8,30,30}, Background, LightBlue, LightBlue_1, Black, Button_Normal, BTN_Redo, "↪", false };

// Save/Load handled via File panel only

// Sound
Mix_Music* catSound = nullptr;
int soundVolume = 100;
int soundPitch = 100;

// Log/Error bar
string errorMessage = "";
string lastNormalLog = "";
Uint32 errorMessageTimer = 0;
const Uint32 ERROR_MESSAGE_DURATION = 3000;

void setError(const string& msg) {
    errorMessage = msg;
    errorMessageTimer = SDL_GetTicks() + ERROR_MESSAGE_DURATION;
}

// History panel
struct HistoryItem {
    string description;
    int stateIndex;
};
vector<HistoryItem> historyLog;
const int MAX_HISTORY = 20;

// Drag state
shared_ptr<Block> draggedBlock = nullptr;
bool draggingFromPalette = false;
int dragOffX, dragOffY;
shared_ptr<Block> snapCandidate = nullptr;

// Click/drag detection
bool potentialDrag = false;
int clickStartX, clickStartY;
Uint32 clickStartTime;
shared_ptr<Block> clickBlock = nullptr;
bool clickInValueArea = false;
bool clickInValue2Area = false;
bool clickInStringArea = false;

// Script execution
struct CallFrame {
    shared_ptr<Block> returnTo;
    shared_ptr<Block> loopStart;
    int remaining;
    int iterCount = 0; // watchdog
};
// (watchdog)
const int WATCHDOG_MAX_ITERS_PER_FRAME = 10000;
struct ScriptState {
    shared_ptr<Block> current;
    vector<CallFrame> stack;
    int waitFrames = 0;
    string waitingForBroadcast;
    int waitingChildren;
    shared_ptr<Block> conditionBlock = nullptr;
};
vector<ScriptState> flagScripts, spaceScripts, clickScripts;
map<string, vector<ScriptState>> messageScripts;

bool scriptsRunning = false;
bool penEnabled = false;
bool showExtPanel = false;
bool isPaused = false;
bool stepRequested = false;
bool stepModeActive = false;  // when true, all events run step by step
shared_ptr<Block> currentExecutingBlock = nullptr;  // block currently highlighted
Uint32 lastUpTime = 0;
int lastUpX = 0, lastUpY = 0;

// Category
Category currentCategory = CAT_MOTION;
TTF_Font* gFont = nullptr;

// Block editing
shared_ptr<Block> editingBlock = nullptr;
string editInputString;
bool editing = false;
int editingFieldIndex = 0;

// Sprite property editing
int editingSpriteProp = -1;
string spriteEditString;

// Backdrop rename editing
int editingBackdropIndex = -1;
string backdropEditString;
bool editingBackdropName = false;

// Backdrop double-click detection
Uint32 lastBackdropClickTime = 0;
int lastBackdropClickIndex = -1;

// Last operator result
int lastOperatorResult = 0;

// Variables
vector<Variable> variables;

// Sensing
string askAnswer;
bool waitingForAnswer = false;

// Key names
const char* keyNames[] = { "space", "up", "down", "left", "right" };
int keyScancodes[] = { SDL_SCANCODE_SPACE, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT };

// ==================== UNDO/REDO ====================
struct State {
    vector<shared_ptr<Block>> scriptBlocks;
    Sprite sprite;
    vector<Variable> variables;
    int backdropIndex;
};

vector<State> undoStack;
int undoIndex = -1;
const int MAX_UNDO = 50;

// =========================================================
// Module Implementation Files
// Included directly so all modules share global state.
// =========================================================
// --- Ali Dehghan: System & Infrastructure ---
#include "ali_io.cpp"
#include "ali_utils.cpp"
// --- Hamed Arabpour: Execution Engine ---
#include "hamed_exec.cpp"
#include "hamed_ctrl.cpp"
// --- Davoud Samie Darian: Graphics & Interaction ---
#include "davoud_render.cpp"
#include "davoud_input.cpp"

// ==================== MAIN ====================
int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(nullptr)));

    bool running = true;
    SDL_Event event;
    SDL_Surface* surface;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) cout << "Error initializing SDL: " << SDL_GetError() << endl;
    if (TTF_Init() < 0) cout << "Error initializing Font: " << TTF_GetError() << endl;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) cout << "SDL_mixer error: " << Mix_GetError() << endl;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    SDL_Window* window = SDL_CreateWindow("Scratch", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           1280, 720, SDL_WINDOW_HIDDEN);
    if (!window) cout << "Failed to create a window! Error: " << SDL_GetError() << endl;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) cout << "Failed to create a renderer! Error: " << SDL_GetError() << endl;
    g_renderer = renderer;
    SDL_RenderSetLogicalSize(renderer, 1280, 720);
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        g_hwnd = wmInfo.info.win.window;
    }

    catSound = Mix_LoadMUS("assets/freesound_community-cat-meow-6226.mp3");
    if (!catSound) cout << "Failed to load cat sound: " << Mix_GetError() << endl;

    mainStage.rect = Stage1;
    mainStage.currentBackdropIndex = 0;

    auto createBackdrop = [&](const string& name, Uint8 r, Uint8 g, Uint8 b) {
        Costume c;
        c.name = name;
        c.width = mainStage.rect.w;
        c.height = mainStage.rect.h;
        SDL_Surface* surf = SDL_CreateRGBSurface(0, c.width, c.height, 32, 0, 0, 0, 0);
        SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, r, g, b));
        c.texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        mainStage.backdrops.push_back(c);
    };

    createBackdrop("back1", 255, 255, 255);

    auto loadBackdropFromFile = [&](const string& path, const string& name) {
        SDL_Surface* surf = IMG_Load(path.c_str());
        if (!surf) { createBackdrop(name, 200, 200, 200); return; }
        int stageW = mainStage.rect.w, stageH = mainStage.rect.h;
        SDL_Surface* converted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA8888, 0);
        SDL_FreeSurface(surf);
        if (!converted) { createBackdrop(name, 200, 200, 200); return; }
        SDL_Surface* scaled = SDL_CreateRGBSurfaceWithFormat(0, stageW, stageH, 32, SDL_PIXELFORMAT_RGBA8888);
        if (!scaled) { SDL_FreeSurface(converted); createBackdrop(name, 200, 200, 200); return; }
        SDL_BlitScaled(converted, NULL, scaled, NULL);
        SDL_FreeSurface(converted);
        Costume c; c.name = name; c.width = stageW; c.height = stageH;
        c.texture = SDL_CreateTextureFromSurface(renderer, scaled);
        SDL_FreeSurface(scaled);
        if (!c.texture) { createBackdrop(name, 200, 200, 200); return; }
        mainStage.backdrops.push_back(c);
    };
    loadBackdropFromFile("assets/back2.jpg", "back2");
    loadBackdropFromFile("assets/back3.jpg", "back3");

    Sprite defaultSprite = Create_Sprite(1, "Cat");
    defaultSprite.scratchx = 0; defaultSprite.scratchy = 0;
    defaultSprite.penDown = false; defaultSprite.penColor = Orange; defaultSprite.penSize = 2;
    defaultSprite.direction = 90.0;
    Add_Costume_To_Sprite(renderer, defaultSprite, "Cat", "assets/catt.png");
    allSprites.push_back(defaultSprite);

    auto loadTex = [&](const char* file, int fw=1, int fh=1) -> SDL_Texture* {
        SDL_Surface* s = IMG_Load(file);
        if (!s) return nullptr;
        if (fw>1 || fh>1) s = shrinkSurface(s, fw, fh);
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
        return t;
    };
    surface = IMG_Load("assets/icons8-scratch-48.png");
    if (surface) { SDL_SetWindowIcon(window, surface); SDL_FreeSurface(surface); }
    Scratch_Logo = loadTex("assets/pngwing.com.png", 12,12);
    Sprite_Choosing.Picture1 = loadTex("assets/icons8-choose-64.png", 2,2);
    Add_BackDrop.Picture1 = loadTex("assets/icons8-add-picture-96.png", 3,3);
    Go.Picture1 = loadTex("assets/icons8-flag-48.png", 2,2);
    Stop.Picture1 = loadTex("assets/icons8-octagon-48-nonavtive.png", 2,2);
    Stop.Picture2 = loadTex("assets/icons8-octagon-48-active.png", 2,2);
    StepBtn.Picture1 = loadTex("assets/icons8-step-50.png");
    Pause.Picture1 = loadTex("assets/icons8-continue-48.png");
    Pause.Picture2 = loadTex("assets/icons8-pause-26.png");

    Define_Toolbar(renderer, ToolBar_Button);
    Define_BlockBar(BlockBar_Button);
    Define_FilePanel_Btn(File_Panel_Button);
    Define_EditPanel_Btn(Edit_Panel_Button);
    Define_HelpPanel_Btn(Help_Panel_Button);
    Define_Toolbar_BTN_Text(renderer, ToolBar_Button);
    Define_Blockbar_BTN_Text(renderer, BlockBar_Button);
    Define_Panel_BTN_Text(renderer, File_Panel_Button,3);
    Define_Panel_BTN_Text(renderer, Edit_Panel_Button,2);
    Define_Panel_BTN_Text(renderer, Help_Panel_Button,3);

    File_BTN_Panel = {File_Panel, false, File_Panel_Button,3};
    Edit_BTN_Panel = {Edit_Panel, false, Edit_Panel_Button,2};
    Help_BTN_Panel = {Help_Panel, false, Help_Panel_Button,3};

    // Open font before creating textures
    gFont = TTF_OpenFont("assets/OpenSans-Regular.ttf", 14);

    // Create text for buttons (Pause and Step use images only)
    if (gFont) {
        SDL_Surface* s_undo = TTF_RenderUTF8_Blended(gFont, "↩", Black);
        UndoBtn.text_texture1 = SDL_CreateTextureFromSurface(renderer, s_undo);
        SDL_FreeSurface(s_undo);

        SDL_Surface* s_redo = TTF_RenderUTF8_Blended(gFont, "↪", Black);
        RedoBtn.text_texture1 = SDL_CreateTextureFromSurface(renderer, s_redo);
        SDL_FreeSurface(s_redo);
    }
    Pause.text = "";
    StepBtn.text = "";

    for (int i=0; i<4; i++) allButtons.push_back(&ToolBar_Button[i]);
    for (int i=0; i<10; i++) allButtons.push_back(&BlockBar_Button[i]);
    allButtons.push_back(&BlockBar_Button[10]); // AddExt — circle, handled in HandleButtonClick
    for (int i=0; i<3; i++) allButtons.push_back(&File_Panel_Button[i]);
    for (int i=0; i<2; i++) allButtons.push_back(&Edit_Panel_Button[i]);
    for (int i=0; i<3; i++) allButtons.push_back(&Help_Panel_Button[i]);
    allButtons.push_back(&Go);
    allButtons.push_back(&Stop);
    allButtons.push_back(&Pause);
    allButtons.push_back(&StepBtn);
    allButtons.push_back(&UndoBtn);
    allButtons.push_back(&RedoBtn);
    allButtons.push_back(&Sprite_Choosing);
    allButtons.push_back(&Add_BackDrop);

    BuildPaletteBlocksForCategory(currentCategory, renderer);

    TTF_Font* loadFont = TTF_OpenFont("assets/OpenSans-Regular.ttf", 30);
    SDL_ShowWindow(window);
    DrawLoading(renderer, loadFont); SDL_RenderPresent(renderer); SDL_Delay(2000);
    Render(renderer);

    const int FPS = 60, FRAME_DELAY = 1000/FPS;
    while (running) {
        Uint32 frameStart = SDL_GetTicks();
        bool mouseDownThisFrame = false, mouseUpThisFrame = false;
        bool spaceDown = false, upDown = false, downDown = false, leftDown = false, rightDown = false;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                if (showExitDialog(renderer)) {
                    running = false;
                    break;
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                if (showExitDialog(renderer)) {
                    running = false;
                    break;
                }
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) spaceDown = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP) upDown = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_DOWN) downDown = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LEFT) leftDown = true;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RIGHT) rightDown = true;
            if (event.type == SDL_MOUSEBUTTONDOWN) mouseDownThisFrame = true;
            if (event.type == SDL_MOUSEBUTTONUP) { mouseUpThisFrame = true; lastUpTime = event.button.timestamp; lastUpX = event.button.x; lastUpY = event.button.y; }

            if (editing || editingSpriteProp != -1) {
                if (event.type == SDL_TEXTINPUT) {
                    if (editing) editInputString += event.text.text;
                    else spriteEditString += event.text.text;
                }
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        if (editing) {
                            if (editingFieldIndex == 2) {
                                editingBlock->strValue = editInputString;
                                // For blocks that show variable name or expression, use strValue as label
                                if (editingBlock->type == BLOCK_VAR ||
                                    editingBlock->type == BLOCK_SHOW_VARIABLE ||
                                    editingBlock->type == BLOCK_HIDE_VARIABLE ||
                                    editingBlock->type == BLOCK_DEFINE_VARIABLE) {
                                    editingBlock->label = editInputString;
                                } else {
                                    string newLabel = editingBlock->baseLabel;
                                    size_t pos = newLabel.find("{}");
                                    if (pos != string::npos) newLabel.replace(pos, 2, editInputString);
                                    editingBlock->label = newLabel;
                                }
                            } else {
                                int newVal = atoi(editInputString.c_str());
                                if (editingFieldIndex == 0) editingBlock->value = newVal;
                                else editingBlock->value2 = newVal;
                                string newLabel = editingBlock->baseLabel;
                                size_t pos1 = newLabel.find("{}");
                                if (pos1 != string::npos) {
                                    newLabel.replace(pos1, 2, to_string(editingBlock->value));
                                    size_t pos2 = newLabel.find("{}", pos1+2);
                                    if (pos2 != string::npos) newLabel.replace(pos2, 2, to_string(editingBlock->value2));
                                }
                                editingBlock->label = newLabel;
                            }
                            UpdateBlockTexture(editingBlock, renderer);
                            editing = false; SDL_StopTextInput();
                            pushState("Changed block: " + editingBlock->label);
                        } else if (editingSpriteProp != -1) {
                            double newVal = atof(spriteEditString.c_str());
                            Sprite& s = allSprites[0];
                            switch (editingSpriteProp) {
                                case 0: s.scratchx = newVal; break;
                                case 1: s.scratchy = newVal; break;
                                case 2: s.direction = newVal; break;
                                case 3: s.size = newVal; break;
                            }
                            editingSpriteProp = -1; SDL_StopTextInput();
                            pushState("Changed sprite property");
                        }
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        if (editing && !editInputString.empty()) editInputString.pop_back();
                        else if (editingSpriteProp != -1 && !spriteEditString.empty()) spriteEditString.pop_back();
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        if (editing) { editing = false; SDL_StopTextInput(); }
                        else if (editingSpriteProp != -1) { editingSpriteProp = -1; SDL_StopTextInput(); }
                    }
                }
            }

            if (waitingForAnswer) {
                if (event.type == SDL_TEXTINPUT) {
                    askAnswer += event.text.text;
                } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                    waitingForAnswer = false;
                    SDL_StopTextInput();
                    defineVariable("answer");
                    int idx = findVariable("answer");
                    if (idx != -1) variables[idx].value = atoi(askAnswer.c_str());
                    askAnswer.clear();
                    pushState("Answered");
                } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE && !askAnswer.empty()) {
                    askAnswer.pop_back();
                }
            }

            // ---- Backdrop right-click (rename) and double-click (delete) ----
            if (event.type == SDL_MOUSEBUTTONDOWN && IsMouseOverRect(BackDrop_List, event.button.x, event.button.y)) {
                int bx = event.button.x, by = event.button.y;
                int yOff = BackDrop_List.y + 5;
                for (int i = 0; i < (int)mainStage.backdrops.size(); i++) {
                    SDL_Rect itemRect = {BackDrop_List.x + 5, yOff, BackDrop_List.w - 10, 30};
                    if (by >= itemRect.y && by <= itemRect.y + itemRect.h) {
                        if (event.button.button == SDL_BUTTON_RIGHT) {
                            // Right-click: rename
                            string newName = showRenameDialog(renderer, mainStage.backdrops[i].name);
                            if (!newName.empty()) {
                                mainStage.backdrops[i].name = newName;
                                logAction("Renamed backdrop to: " + newName);
                            }
                        } else if (event.button.button == SDL_BUTTON_LEFT) {
                            Uint32 now = SDL_GetTicks();
                            if (lastBackdropClickIndex == i && (now - lastBackdropClickTime) < 400) {
                                // Double-click: delete (keep at least 1 backdrop)
                                if (mainStage.backdrops.size() > 1) {
                                    if (mainStage.backdrops[i].texture) SDL_DestroyTexture(mainStage.backdrops[i].texture);
                                    mainStage.backdrops.erase(mainStage.backdrops.begin() + i);
                                    if (mainStage.currentBackdropIndex >= (int)mainStage.backdrops.size())
                                        mainStage.currentBackdropIndex = (int)mainStage.backdrops.size() - 1;
                                    logAction("Deleted backdrop");
                                } else {
                                    setError("Cannot delete the last backdrop");
                                }
                                lastBackdropClickIndex = -1;
                            } else {
                                lastBackdropClickTime = now;
                                lastBackdropClickIndex = i;
                            }
                        }
                        break;
                    }
                    yOff += 32;
                }
            }
        }

        SDL_GetMouseState(&MOUUSE_X, &MOUUSE_Y);
        if (!editing && editingSpriteProp == -1) HandleBlockEvents(event, MOUUSE_X, MOUUSE_Y, mouseDownThisFrame, mouseUpThisFrame, renderer);
        if (mouseUpThisFrame && !editing && editingSpriteProp == -1) HandleSpriteInfoEvents(MOUUSE_X, MOUUSE_Y, true, renderer);

        for (int i = allSprites.size()-1; i>=0; i--) {
            Clamp_Sprite_To_Stage_Bounds(allSprites[i], mainStage);
            Update_Sprite_Render_Rect(allSprites[i], mainStage);
            if (allSprites[i].isBeingDragged) {
                Handle_Sprite_Drag_And_Drop(allSprites[i], mainStage, event, MOUUSE_X, MOUUSE_Y);
                break;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN && IsMouseOverRect(allSprites[i].rect, MOUUSE_X, MOUUSE_Y)) {
                Handle_Sprite_Drag_And_Drop(allSprites[i], mainStage, event, MOUUSE_X, MOUUSE_Y);
                break;
            }
        }

        bool uiClicked = false;

        if (showExtPanel && mouseUpThisFrame) {
            const int PANEL_W = 260, PANEL_H = 120;
            SDL_Rect panel = {Blockbar_Width + 10, 720 - Blockbar_Width - PANEL_H - 5, PANEL_W, PANEL_H};
            SDL_Rect penBtn = {panel.x + 10, panel.y + 40, PANEL_W - 20, 35};
            SDL_Rect closeBtn = {panel.x + PANEL_W - 25, panel.y + 5, 20, 20};
            SDL_Point pt = {MOUUSE_X, MOUUSE_Y};
            if (SDL_PointInRect(&pt, &closeBtn)) {
                showExtPanel = false;
                uiClicked = true;
            } else if (SDL_PointInRect(&pt, &penBtn) && !penEnabled) {
                penEnabled = true;
                showExtPanel = false;
                logAction("Pen extension added");
                uiClicked = true;
            } else if (!SDL_PointInRect(&pt, &panel)) {
                showExtPanel = false;
            } else {
                uiClicked = true;
            }
        }

        // Fix panel bug: check click inside any of the three panels
        bool clickedInsidePanel =
            (ToolBar_Button[0].isselected && IsMouseOverRect(File_Panel, MOUUSE_X, MOUUSE_Y)) ||
            (ToolBar_Button[1].isselected && IsMouseOverRect(Edit_Panel, MOUUSE_X, MOUUSE_Y)) ||
            (ToolBar_Button[3].isselected && IsMouseOverRect(Help_Panel, MOUUSE_X, MOUUSE_Y));
        if (mouseDownThisFrame && !clickedInsidePanel) False_Selected(ToolBar_Button, 4);
        UpdateAllButtons(allButtons, MOUUSE_X, MOUUSE_Y, mouseDownThisFrame, mouseUpThisFrame, &uiClicked);

        for (int i=0; i<10; i++) {
            if (BlockBar_Button[i].buttonID == BTN_Pen && !penEnabled) continue;
            if (BlockBar_Button[i].isselected && mouseUpThisFrame) {
                Category newCat = CAT_MOTION;
                switch (BlockBar_Button[i].buttonID) {
                    case BTN_Motion: newCat = CAT_MOTION; break; case BTN_Looks: newCat = CAT_LOOKS; break;
                    case BTN_Sound: newCat = CAT_SOUND; break; case BTN_Events: newCat = CAT_EVENTS; break;
                    case BTN_Control: newCat = CAT_CONTROL; break; case BTN_Sensing: newCat = CAT_SENSING; break;
                    case BTN_Operators: newCat = CAT_OPERATORS; break; case BTN_Variables: newCat = CAT_VARIABLES; break;
                    case BTN_Pen: newCat = CAT_PEN; break; case BTN_MyBlocks: newCat = CAT_MYBLOCKS; break;
                    default: break;
                }
                if (newCat != currentCategory) { currentCategory = newCat; BuildPaletteBlocksForCategory(currentCategory, renderer); }
            }
        }
        if (Go.isselected && mouseUpThisFrame) {
            scriptsRunning = true; isPaused = false; stepRequested = false; flagScripts.clear();
            startScriptsForHat(BLOCK_WHEN_FLAG);
            if (stepModeActive) { isPaused = true; stepRequested = true; }
            pushState("Green flag clicked");
        }
        if (Stop.isselected && mouseUpThisFrame) {
            scriptsRunning = false; isPaused = false; stepRequested = false; stepModeActive = false;
            flagScripts.clear(); spaceScripts.clear(); clickScripts.clear(); messageScripts.clear();
            currentExecutingBlock = nullptr; StepBtn.isselected = false;
            pushState("Stop clicked");
        }
        if (Pause.isselected && mouseUpThisFrame) {
            isPaused = !isPaused;
            pushState(isPaused ? "Paused" : "Resumed");
        }
        if (StepBtn.isselected && mouseUpThisFrame) {
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
            } else {
                // Already in step mode: advance exactly one step
                stepRequested = true;
                isPaused = false;
            }
            StepBtn.isselected = stepModeActive;
        }
        if (UndoBtn.isselected && mouseUpThisFrame) {
            undo(renderer);
            UndoBtn.isselected = false;
        }
        if (RedoBtn.isselected && mouseUpThisFrame) {
            redo(renderer);
            RedoBtn.isselected = false;
        }

        // Main File panel buttons
        if (File_Panel_Button[0].isselected && mouseUpThisFrame) {
            File_Panel_Button[0].isselected = false;
            ToolBar_Button[0].isselected = false;
            // Only warn if there are changes (scriptBlocks not empty or undoStack has more than 1 state)
            bool hasChanges = !scriptBlocks.empty() || variables.size() > 0;
            if (hasChanges) {
                if (showNewFileDialog(renderer)) {
                    resetProject(renderer);
                    Define_Blockbar_BTN_Text(renderer, BlockBar_Button);
                    currentCategory = CAT_MOTION;
                    BuildPaletteBlocksForCategory(currentCategory, renderer);
                }
            } else {
                resetProject(renderer);
                Define_Blockbar_BTN_Text(renderer, BlockBar_Button);
                currentCategory = CAT_MOTION;
                BuildPaletteBlocksForCategory(currentCategory, renderer);
            }
        }
        if (File_Panel_Button[1].isselected && mouseUpThisFrame) {
            string filename = getOpenFileName();
            if (!filename.empty()) {
                loadProject(filename, renderer);
            }
            File_Panel_Button[1].isselected = false;
            ToolBar_Button[0].isselected = false; // close File panel
        }
        if (File_Panel_Button[2].isselected && mouseUpThisFrame) {
            string filename = getSaveFileName();
            if (!filename.empty()) {
                saveProject(filename);
            }
            File_Panel_Button[2].isselected = false;
            ToolBar_Button[0].isselected = false; // close File panel
        }

        // About dialog (Help panel button[0])
        if (Help_Panel_Button[0].isselected && mouseUpThisFrame) {
            Help_Panel_Button[0].isselected = false;
            ToolBar_Button[3].isselected = false;
            showAboutDialog(renderer);
        }

        if (spaceDown) { spaceScripts.clear(); startScriptsForHat(BLOCK_WHEN_KEY, "space"); if (!spaceScripts.empty()) scriptsRunning = true; pushState("Space key pressed"); if (stepModeActive) { isPaused = true; stepRequested = true; } }
        if (upDown)    { spaceScripts.clear(); startScriptsForHat(BLOCK_WHEN_KEY, "up");    if (!spaceScripts.empty()) scriptsRunning = true; pushState("Up key pressed");    if (stepModeActive) { isPaused = true; stepRequested = true; } }
        if (downDown)  { spaceScripts.clear(); startScriptsForHat(BLOCK_WHEN_KEY, "down");  if (!spaceScripts.empty()) scriptsRunning = true; pushState("Down key pressed");  if (stepModeActive) { isPaused = true; stepRequested = true; } }
        if (leftDown)  { spaceScripts.clear(); startScriptsForHat(BLOCK_WHEN_KEY, "left");  if (!spaceScripts.empty()) scriptsRunning = true; pushState("Left key pressed");  if (stepModeActive) { isPaused = true; stepRequested = true; } }
        if (rightDown) { spaceScripts.clear(); startScriptsForHat(BLOCK_WHEN_KEY, "right"); if (!spaceScripts.empty()) scriptsRunning = true; pushState("Right key pressed"); if (stepModeActive) { isPaused = true; stepRequested = true; } }

        if (mouseUpThisFrame && !uiClicked && IsMouseOverRect(allSprites[0].rect, MOUUSE_X, MOUUSE_Y)) {
            clickScripts.clear(); startScriptsForHat(BLOCK_WHEN_CLICKED);
            if (!clickScripts.empty()) scriptsRunning = true;
            if (stepModeActive) { isPaused = true; stepRequested = true; }
            pushState("Sprite clicked");
        }

        if (mouseUpThisFrame && !uiClicked && IsMouseOverRect(BackDrop_List, MOUUSE_X, MOUUSE_Y)) {
            int yOff = BackDrop_List.y + 5;
            for (size_t i = 0; i < mainStage.backdrops.size(); i++) {
                SDL_Rect itemRect = {BackDrop_List.x + 5, yOff, BackDrop_List.w - 10, 30};
                if (MOUUSE_Y >= itemRect.y && MOUUSE_Y <= itemRect.y + itemRect.h) {
                    if (i != mainStage.currentBackdropIndex) {
                        mainStage.currentBackdropIndex = i;
                        pushState("Backdrop changed to " + mainStage.backdrops[i].name);
                    }
                    break;
                }
                yOff += 32;
            }
        }

        if (mouseUpThisFrame && !uiClicked && IsMouseOverRect(Sprite_List, MOUUSE_X, MOUUSE_Y)) {
            int hx = Sprite_List.x + 5, hw = Sprite_List.w - 10, lineH = 20;
            int total = historyLog.size();
            int start = max(0, total - 14);
            int y = Sprite_List.y + 5;
            for (int idx = total - 1; idx >= start; idx--) {
                SDL_Rect itemRect = {hx, y, hw, lineH};
                if (MOUUSE_Y >= itemRect.y && MOUUSE_Y <= itemRect.y + itemRect.h) {
                    if (historyLog[idx].stateIndex >= 0 && historyLog[idx].stateIndex != undoIndex) {
                        restoreState(historyLog[idx].stateIndex, renderer);
                    }
                    break;
                }
                y += lineH + 2;
                if (y + lineH > Sprite_List.y + Sprite_List.h - 5) break;
            }
        }

        UpdateSprites(allSprites, mainStage, MOUUSE_X, MOUUSE_Y, mouseDownThisFrame, mouseUpThisFrame, uiClicked, renderer);
        if (scriptsRunning) ExecuteScripts(renderer);
        if (!isPaused && flagScripts.empty() && spaceScripts.empty() && clickScripts.empty() && messageScripts.empty()) {
            scriptsRunning = false; Go.isselected = false;
        }
        if (!running) break;
        Render(renderer);
        int frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) SDL_Delay(FRAME_DELAY - frameTime);
    }

    FreeBlockTextures(); if (gFont) TTF_CloseFont(gFont);
    if (catSound) Mix_FreeMusic(catSound);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window);
    TTF_Quit(); SDL_Quit();
    return 0;
}
