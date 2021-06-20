#include "../global.h"
#include "../gl_utils.h"
#include "../renderer/quad.h"

#include "resizable_rect.h"

Vector2 tilesheetSize;
Vector2 tileSize;

const SDL_Rect regions[3][3] = {
	/*	LEFT		CENTER			RIGHT*/
    {{0, 0, 6, 6}, {6, 0, 4, 6}, {10, 0, 6, 6}},	// TOP
    {{0, 6, 6, 4}, {6, 6, 4, 4}, {10, 6, 6, 4}},	// MIDDLE
    {{0, 10, 6, 6}, {6, 10, 4, 6}, {10, 10, 6, 6}},	// BOTTOM
};

void AddRectOffset(SDL_Rect *rect, Vector2 offset){
    rect->x += offset.x;
    rect->y += offset.y;
}

// Add 'border' argument for all '6' values and 'border * 2' for '12' values
void ResizableRect(TilesheetObject tilesheet, SDL_Rect position, int index, int border, int zpos, Vector4 color){
    Vector2 offset = (Vector2){
        (index % (tilesheet.texture.w / tilesheet.tile_w)) * tilesheet.tile_w, 
        (index / (tilesheet.texture.w / tilesheet.tile_w)) * tilesheet.tile_h
    };

    SDL_Rect src;
    SDL_Rect dst = {position.x, position.y, border, border};
    for(int y = 0; y < 3; y++){
        if(y == 1){//Middle row
            src = regions[y][0];
            AddRectOffset(&src, offset);
            dst = (SDL_Rect){position.x, dst.y, border, position.h - (border * 2)};
            RenderQuad(tilesheet.texture, &src, &dst, zpos, color, 0);

            src = regions[y][1];
            AddRectOffset(&src, offset);
            dst = (SDL_Rect){position.x + border, position.y + border, position.w - (border * 2), position.h - (border * 2)};//Center
            RenderQuad(tilesheet.texture, &src, &dst, zpos, color, 0);

            src = regions[y][2];
            AddRectOffset(&src, offset);
            dst = (SDL_Rect){dst.x + position.w - (border * 2), dst.y, border, position.h - (border * 2)};
            RenderQuad(tilesheet.texture, &src, &dst, zpos, color, 0);


            dst.y += position.h - (border * 2);
        }else{
            src = regions[y][0];
            AddRectOffset(&src, offset);
            dst = (SDL_Rect){position.x, dst.y, border, border};
            RenderQuad(tilesheet.texture, &src, &dst, zpos, color, 0);

            src = regions[y][1];
            AddRectOffset(&src, offset);
            dst = (SDL_Rect){dst.x + dst.w, dst.y, position.w - (border * 2), border};
            RenderQuad(tilesheet.texture, &src, &dst, zpos, color, 0);

            src = regions[y][2];
            AddRectOffset(&src, offset);
            dst = (SDL_Rect){dst.x + position.w - (border * 2), dst.y, border, border};
            RenderQuad(tilesheet.texture, &src, &dst, zpos, color, 0);


            dst.y += border;
        }
    }
}