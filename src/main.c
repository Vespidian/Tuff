#include <GL/glew.h>
#include <cglm/cglm.h>
#include <string.h>
#include <stdbool.h>

#include "vectorlib.h"
#include "event.h"
#include "textures.h"
#include "shader.h"
#include "renderer/renderer.h"
#include "render_text.h"
#include "ui.h"

#include "engine.h"

Shader shader;
Texture texture;

UIState state;

void tmp(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
    if(events & UI_MOUSE_CLICK){
        UIElement *e = UIFindElement(state, "idk");
        e->visible = !e->visible;
    }
}

void EngineSetup(){
    UI_WINDOW_HEIGHT = 800;
    UI_WINDOW_WIDTH = 800;

    InitUIRender();

    // Basic Texture
    texture = TextureOpen("../assets/texture.png", TEXTURE_FILTERING_NEAREST);

    // Basic Shader
    shader = ShaderOpen("../assets/shader.shader");



    UIParse(&state, "../assets/ui/new.ui");

    // Create a slider
    UISliderNew(UIFindElement(&state, "b1"), 0, 100, 5, 0.1);
    UIFindElement(&state, "b1")->slider.modify_width = true;

    // Implement a show/hide button
    UIFindElement(&state, "hehe")->event_func = tmp;



    // WHEN COME BACK:
    // * test out multiple classes on an element
    // * implement inherit class property (apprently this was already done)
    // * think about how we're going to do mouse interaction - mouse interaction mostly done (see TODOs)
    // * look into how we are doing text rendering
    // * work on json UI file structure and parsing

    // * test out root origins (positioning elements relative to bottom or right) and window resizing
    // * implement hiding of elements and their children (make sure hidden elements arent interacted with)

    // *** NOW TO ENCAPSULATE ALL OF SDL AND OPENGL STARTUP CODE AS WELL AS ALL INIT FUNCTIONS INTO SINGLE "ENGINE START" FUNC ***

    // - work on UI serialization to json
    // * get borders to render in the shader
    // - start thinking about percent scaling

    // So we want all classes to be stored in a UIState array
    // Classes an elements are only ever referenced by their IDs
    // IDs represent the class or element's position in the
    //  corresponding array in the UIState


}

void EngineExit(){
    // TextureFree(&texture);
    ShaderFree(&shader);
}


void EngineLoop(){

    RenderText(&default_font, 1.001, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 30, TEXT_ALIGN_LEFT, "testing");

    UIPush(&state);

    PushRender();
}

