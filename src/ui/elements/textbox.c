#include "../../global.h"
#include "../../renderer/renderer.h"
#include "../../renderer/render_text.h"
#include "../../event.h"
#include "../../text_event.h"
#include "../ui.h"
#include "../resizable_rect.h"

#include "textbox.h"

void Textbox_f(Textbox_t *box){
    if(mouse_clicked){
        //if mouse hovered over and clicked, textbox is selected 
        if(SDL_PointInRect(&mouse_pos, &box->pos)){
            if(box->text == NULL){
                box->text[0] = '\0';
            }
			TextObjectClear(&text_object);
            box->selected = true;
        }else{//if mouse clicks elsewhere, textbox no longer selected
            box->selected = false;
            text_input = false;
        }
    }
    if(box->selected){
        text_input = true;
		char tmp;
		while((tmp = TextObjectRead(&text_object)) != 0){
			switch(tmp){
				case 8: // Backspace
					if(strlen(box->text) > 1){
						box->text[strlen(box->text) - 1] = '\0';
					}
					break;
				default:
					if(strlen(box->text) < 32){
						box->text[strlen(box->text)] = tmp;
						box->text[strlen(box->text) + 1] = '\0';
					}
					break;
			}
		}
    }
    // render elements
    if(box->text[0] != '\0'){
        char *scrolledText = box->text;
        int maxShownChars = (box->pos.w + box->pos.w / 14) / 9;
        if(9 * strlen(box->text) > box->pos.w + box->pos.w / 14){
            int offset = ((strlen(box->text) * 9) - (box->pos.w + box->pos.w / 14)) / 9 + 1;
            if(box->selected){// Set text offset to "scroll" the text, not letting it leave the confines of the box
                scrolledText = box->text + offset;
            }
        }
        // RenderTextEx(FindFont("default_font"), 1, box->pos.x + 2, box->pos.y + box->pos.h / 2 - 8, (Vector4){1, 1, 1, 1}, 0, maxShownChars, "%s", scrolledText);
        RenderTextEx(FindFont("default_font"), 1, SCREEN_WIDTH / 2, box->pos.y + box->pos.h / 2 - 8, (Vector4){1, 1, 1, 1}, TEXT_ALIGN_CENTER, maxShownChars, "%s", scrolledText);
	}
    if(box->selected){
        ResizableRect(ui_tilesheet, box->pos, 9, 6, RNDR_UI, (Vector4){1, 1, 1, 1});
    }else{
        ResizableRect(ui_tilesheet, box->pos, 8, 6, RNDR_UI, (Vector4){1, 1, 1, 1});
    }
}

