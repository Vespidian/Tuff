#include "../global.h"
#include "../debug.h"
#include "../gl_utils.h"
#include "../renderer/quad.h"

//UI Elements
#include "resizable_rect.h"

#include "ui.h"

bool ui_hovered = false;
TilesheetObject ui_tilesheet;

void InitUI(){
	ui_tilesheet = LoadTilesheet("../images/ui/ui.png", 16, 16);

	DebugLog(D_ACT, "Initialized UI subsystem");
}

long IntegerLerp(long start, long end, uint16_t amount){
	long lerpLimit = 1;
	long velocity = (end - start) * ((float)amount / 65535.0f);
	if(velocity < lerpLimit && start + lerpLimit <= end){
		return lerpLimit;
	}
	return velocity;
}

void LoopUI(){
	ui_hovered = false;
}

SDL_Rect VerticalRectList(int num_items, int itemIndex, Vector2 size, Vector2 origin, int spacing){
	SDL_Rect rect = {0, 0, size.x, size.y};
	int yOffset = 0;
	if(num_items % 2 == 0){//Even number of elements
		rect.x = origin.x - size.x / 2;
		if(itemIndex < num_items / 2){//Top half
			yOffset = (num_items / 2 - itemIndex);
			rect.y = origin.y - (spacing / 2 + size.y * yOffset + spacing * (yOffset - 1));
		}else{//Bottom half
			yOffset = (itemIndex - num_items / 2);
			rect.y = origin.y + (spacing / 2 + size.y * yOffset + spacing * yOffset);
		}
	}else{//Odd number of elements
		rect.x = origin.x - size.x / 2;
		if(itemIndex < num_items / 2){//top half
			yOffset = (num_items / 2 - itemIndex);
			rect.y = origin.y - (size.y / 2 + size.y * yOffset + spacing * yOffset);
		}else if(itemIndex == (num_items + 1) / 2 - 1){//Middle element
			rect.y = origin.y - (size.y / 2);
		}else{//Bottom half
			yOffset = (itemIndex - 1 - num_items / 2);
			rect.y = origin.y + (size.y / 2 + size.y * yOffset + spacing * (yOffset	+ 1));
		}
	}
	return rect;
}

Vector4_i VerticalRectList_vec(int num_items, int itemIndex, Vector2 size, Vector2 origin, int spacing){
	SDL_Rect tmp = VerticalRectList(num_items, itemIndex, size, origin, spacing);
	return (Vector4_i){tmp.x, tmp.y, tmp.w, tmp.h};
}

void DrawVListBackground(int num_items, Vector2 size, Vector2 origin, int spacing, uint8_t opacity){
	int padding = 8;
	SDL_Rect rect;
	rect.w = size.x + padding * 2;
	rect.h = num_items * size.y + (num_items + 1) * padding;
	rect.x = SCREEN_WIDTH / 2 - rect.w / 2;
	rect.y = SCREEN_HEIGHT / 2 - rect.h / 2;
	RenderTilesheet(ui_tilesheet, 0, &rect, RNDR_UI, (Vector4){1, 1, 1, 1});
}