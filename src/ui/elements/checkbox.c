#include "../../global.h"
#include "../../renderer/quad.h"
#include "../../renderer/render_text.h"
#include "../../event.h"
#include "../ui.h"

#include "checkbox.h"

void Checkbox(bool *value, char *label, SDL_Rect position){
	SDL_Point mouse;
	SDL_GetMouseState(&mouse.x, &mouse.y);

	SDL_Rect checkbox = {position.x - 42, position.y, 32, 32};
	SDL_Rect hitbox = {checkbox.x + 8, checkbox.y + 8, checkbox.w - 16, checkbox.h - 16};

	if(SDL_PointInRect(&mouse, &hitbox)){
		if(mouse_clicked){
			*value = !*value;
		}
	}
	// PushRender_Tilesheet(renderer, FindTilesheet("ui"), !*value + 1, checkbox, RNDR_UI);
	RenderTilesheet(ui_tilesheet, !*value + 1, &checkbox, RNDR_UI, (Vector4){1, 1, 1, 1});

	RenderText(FindFont("default_font"), 1, position.x + 14, position.y + 8, 0, label);
}