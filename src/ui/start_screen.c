#include <dirent.h>

#include "../global.h"
#include "../debug.h"
#include "../renderer/quad.h"
#include "ui.h"
#include "elements/button.h"
#include "elements/textbox.h"
#include "elements/slider.h"
#include "elements/checkbox.h"

#include "start_screen.h"

const int dirStringSize = 260;
char **sandboxDirs;
int numSandboxDirs = 0;

int menu_index = 0;

void ListSandboxes();

Textbox_t newWorldInput;
Textbox_t tmp_textbox;

float value = 0.75;
bool is_text = false;
void RenderStartScreen(){
	RenderTilesheet(builtin_tilesheet, 0, NULL, 0, (Vector4){0.54, 0.84, 0.93, 1});

	// float value = 4.0;
	// RenderSlider(&value, 0, 10, (Vector4_i){200, 200, 200, 30});
	RenderSlider(&value, 0.0, 360, (Vector4_i){200, 200, 200, 30});
	// RenderTilesheet(builtin_tilesheet, 1, &(SDL_Rect){300, 200, 64, 64}, RNDR_UI, (Vector4){1, 1, 1, 1});
	RenderQuad(builtin_tilesheet.texture, &(SDL_Rect){16, 0, 16, 16}, &(SDL_Rect){450, 200, 64, 64}, RNDR_UI, (Vector4){1, 1, 1, 1}, glm_rad(value));

    Vector2 button_size = {200, 32};
    Vector2 origin = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    switch(menu_index){
        case 1:// Play
            for(int i = 0; i < numSandboxDirs; i++){
                if(Button_format(VerticalRectList(numSandboxDirs + 3, i, button_size, origin, 6), sandboxDirs[i])){
                }
            }
            newWorldInput.pos = VerticalRectList(numSandboxDirs + 3, numSandboxDirs, button_size, origin, 6);
            Textbox_f(&newWorldInput);


            tmp_textbox.pos = VerticalRectList(numSandboxDirs + 3, numSandboxDirs + 4, button_size, origin, 6);
            Textbox_f(&tmp_textbox);


            if(Button(VerticalRectList(numSandboxDirs + 3, numSandboxDirs + 1, button_size, origin, 6), "Create World")){
                if(newWorldInput.text != NULL){
                    if(strlen(newWorldInput.text) > 2){
                    }
                }
            }
            if(Button(VerticalRectList(numSandboxDirs + 3, numSandboxDirs + 2, button_size, origin, 6), "Back")){menu_index = 0;}
            break;


        case 2:// Options

            Checkbox(&is_text, "Testing 123", VerticalRectList(2, 0, button_size, origin, 6));
            if(Button(VerticalRectList(2, 1, button_size, origin, 6), "Back")){menu_index = 0;}
            break;

            
        default:
            if(Button(VerticalRectList(3, 0, button_size, origin, 6), "Play")){
                menu_index = 1;
                ListSandboxes();
            }
            if(Button(VerticalRectList(3, 1, button_size, origin, 6), "Options")){menu_index = 2;}
            if(Button(VerticalRectList(3, 2, button_size, origin, 6), "Quit")){Quit();}
            break;
    }
}

void ListSandboxes(){
    DIR *dir;
    struct dirent *ent;
    if(numSandboxDirs > 0){
        for(int i = 0; i < numSandboxDirs; i++){
            free(sandboxDirs[i]);
        }
    }
    numSandboxDirs = 0;
    if((dir = opendir("../saves/")) != NULL){
        while((ent = readdir(dir)) != NULL){
            if(strlen(ent->d_name) >= 3){
                sandboxDirs = realloc(sandboxDirs, sizeof(char) * (dirStringSize) * (numSandboxDirs + 1));
                sandboxDirs[numSandboxDirs] = malloc(sizeof(char) * dirStringSize);
                strcpy(sandboxDirs[numSandboxDirs], ent->d_name);
                numSandboxDirs++;
            }
        }
        closedir(dir);
    }else{
        DebugLog(D_ERR, "Could not open saves directory");
    }
}