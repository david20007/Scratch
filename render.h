#pragma once
// ============================================================
// davoud_render.h — Davoud Samie Darian
// Rendering Pipeline & UI Drawing
// ============================================================
// SDL rendering, sprite drawing, block palette, UI buttons,
// sprite info panel, stage, toolbar, panels, text rendering.
// Style: index-based loops, direct coordinate math,
//        manual state, struct-heavy, no polymorphism.
// ============================================================

void UpdateBlockTexture(shared_ptr<Block> block, SDL_Renderer* renderer);
void BuildPaletteBlocksForCategory(Category cat, SDL_Renderer* renderer);
void DrawBlock(const Block& block, SDL_Renderer* renderer);
void DrawAllBlocks(SDL_Renderer* renderer);
void FreeBlockTexture(Block& block);
void InsertBlockAtSnap(shared_ptr<Block> block, shared_ptr<Block> target, bool above);
void HandleBlockEvents(SDL_Event& event, int mouseX, int mouseY, bool mouseDown, bool mouseUp, SDL_Renderer* renderer);

void RenderText(SDL_Renderer* r, int x, int y, const string& txt, SDL_Color col);

void HandleSpriteInfoEvents(int mouseX, int mouseY, bool mouseUp, SDL_Renderer* renderer);
void DrawSpriteInfo(SDL_Renderer* renderer);
void drawSpriteAtIndex(SDL_Renderer* renderer, const Sprite& sprite, int costumeIndex, int x, int y, double size, double direction);
void Draw_Sprite(SDL_Renderer* renderer, Sprite& sprite, Stage& stage);

void DrawLoading(SDL_Renderer* renderer, TTF_Font* font1);
void DrawStage(SDL_Renderer* renderer, SDL_Rect& stage, SDL_Color color);
void DrawToolbar(SDL_Renderer* r, SDL_Rect& t, SDL_Color c);
void DrawBlockbar_Funcs(SDL_Renderer* renderer, SDL_Rect& blockbar, SDL_Color color);
void DrawMenuButton(SDL_Renderer* renderer, Button& btn);
void DrawBlockButton(SDL_Renderer* renderer, Button& btn);
void Draw_Circle_BTN(SDL_Renderer* renderer, Button& btn);
void Draw_Full_Toolbar(SDL_Renderer* renderer, SDL_Rect& toolbar, Button toolbarbtn[4]);
void Draw_Full_BlockBar_Menu(SDL_Renderer* renderer, SDL_Rect& blockbar, Button blockbarbtn[11]);
void Draw_Stage(SDL_Renderer* renderer, Stage& stage);
void Draw_Panel(SDL_Renderer* renderer, Panel p);
void Draw_Panel_Btn(SDL_Renderer* renderer, Button& btn);
void Render(SDL_Renderer* renderer);

void Define_Toolbar(SDL_Renderer* renderer, Button toolbar_BTN[4]);
void Define_BlockBar(Button blockbar_BTN[11]);
void Define_FilePanel_Btn(Button btn[3]);
void Define_HelpPanel_Btn(Button btn[3]);
void Define_EditPanel_Btn(Button btn[2]);
void Define_Toolbar_BTN_Text(SDL_Renderer* renderer, Button btn[4]);
void Define_Blockbar_BTN_Text(SDL_Renderer* renderer, Button btn[11]);
void Define_Panel_BTN_Text(SDL_Renderer* renderer, Button btn[], int count);

bool IsMouseOverRect(SDL_Rect& rect, int mouseX, int mouseY);
bool IsMouseOverCircle(Button& btn, int mouseX, int mouseY);
bool IsBlockCategory(ButtonID id);
bool IsMenuButton(ButtonID id);
void UpdateButtonState(Button& btn, int mouseX, int mouseY, bool mouseDown);
void HandleButtonClick(vector<Button*> buttons, Button& btn, int mouseX, int mouseY, bool mouseUp, SDL_Renderer* renderer);
void UpdateAllButtons(vector<Button*> buttons, int mouseX, int mouseY, bool mouseDown, bool mouseUp, bool* uiClicked);
void False_Selected(Button btn[], int count);

void Add_Costume_To_Sprite(SDL_Renderer* renderer, Sprite& sprite, const string& costumeName, const char* filePath);
Sprite Create_Sprite(int id, string name);
void Update_Sprite_Render_Rect(Sprite& sprite, Stage& stage);
void Move_Sprite(Sprite& sprite, Stage& stage, double steps);
void Turn_Sprite(Sprite& sprite, double degrees);
void UpdateSprites(vector<Sprite>& sprites, Stage& stage, int mouseX, int mouseY, bool isMouseDown, bool isMouseUp, bool uiAlreadyHandledClick, SDL_Renderer* renderer);
