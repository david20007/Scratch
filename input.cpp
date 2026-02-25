
void FreeBlockTextures() {
    for (auto& b : paletteBlocks) if (b.textTexture) SDL_DestroyTexture(b.textTexture);
    for (auto& b : scriptBlocks) if (b->textTexture) SDL_DestroyTexture(b->textTexture);
}

void Handle_Sprite_Drag_And_Drop(Sprite& s, Stage& st, const SDL_Event& e, int mx, int my) {
    if (!s.isDraggable) return;
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT && IsMouseOverRect(s.rect,mx,my)) {
        s.isBeingDragged = true; s.dragOffsetX = mx - s.rect.x; s.dragOffsetY = my - s.rect.y;
    }
    if (e.type == SDL_MOUSEMOTION && s.isBeingDragged) {
        int nx = mx - s.dragOffsetX, ny = my - s.dragOffsetY;
        int cx = st.rect.x+st.rect.w/2, cy = st.rect.y+st.rect.h/2;
        int ncx = nx + s.rect.w/2, ncy = ny + s.rect.h/2;
        s.scratchx = ncx - cx; s.scratchy = cy - ncy;
    }
    if (e.type == SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT) s.isBeingDragged = false;
}
Sprite Create_Sprite(int id, string n) {
    Sprite s; s.name = n; s.direction=90.0; return s;
}
void Clamp_Sprite_To_Stage_Bounds(Sprite& s, Stage& st) {
    Update_Sprite_Render_Rect(s, st);
    double hw = st.rect.w/2.0, hh = st.rect.h/2.0;
    double halfW = s.rect.w/2.0, halfH = s.rect.h/2.0;
    // Allow half of the sprite to go outside
    double minX = -hw - halfW/2;
    double maxX =  hw + halfW/2;
    double minY = -hh - halfH/2;
    double maxY =  hh + halfH/2;
    if (maxX < minX) {
        s.scratchx = 0;
    } else {
        if (s.scratchx < minX) s.scratchx = minX;
        if (s.scratchx > maxX) s.scratchx = maxX;
    }
    if (maxY < minY) {
        s.scratchy = 0;
    } else {
        if (s.scratchy < minY) s.scratchy = minY;
        if (s.scratchy > maxY) s.scratchy = maxY;
    }
    Update_Sprite_Render_Rect(s, st);
}
void DrawPenLines(SDL_Renderer* renderer) {
    for (auto& s : allSprites) {
        for (const auto& stroke : s.penStrokes) {
            thickLineRGBA(renderer, stroke.x1, stroke.y1, stroke.x2, stroke.y2,
                          stroke.size, stroke.color.r, stroke.color.g, stroke.color.b, 255);
        }
    }
}
void AddPenStroke(Sprite& sprite, double oldX, double oldY, double newX, double newY, SDL_Renderer* renderer) {
    if (!sprite.penDown) return;
    int cx = mainStage.rect.x + mainStage.rect.w/2;
    int cy = mainStage.rect.y + mainStage.rect.h/2;
    PenStroke stroke;
    stroke.x1 = cx + (int)oldX;
    stroke.y1 = cy - (int)oldY;
    stroke.x2 = cx + (int)newX;
    stroke.y2 = cy - (int)newY;
    stroke.size = sprite.penSize;
    stroke.color = sprite.penColor;
    sprite.penStrokes.push_back(stroke);
}
