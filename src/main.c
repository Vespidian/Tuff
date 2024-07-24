#include <GL/glew.h>
#include <string.h>

#include "bundle.h"
#include "material.h"
#include "scene.h"
#include "event.h"
#include "renderer/renderer.h"
#include "render_text.h"
#include "ui.h"

#include "engine.h"

Bundle bundle;
GLTF *sphere;
Material *mat;
Model model;


Texture texture;
UIState state;

void tmp(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
    if(events & UI_MOUSE_CLICK){
        UIElement *e = UIFindElement(state, "menu");
        e->visible = !e->visible;
        e->visible_children = e->visible;
    }
}

float data[64] = {0};
void EngineSetup(){
    UI_WINDOW_HEIGHT = 800;
    UI_WINDOW_WIDTH = 800;

    bundle = BundleOpen(NULL);
    sphere = BundleGLTFOpen(&bundle, "models/sphere.gltf");
    mat = BundleMaterialOpen(&bundle, "materials/default.mat");
    model = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/sphere.gltf", true)->meshes[0], mat);

    MaterialUniformSetVec3(mat, "light_pos", (vec3){1, 1, 1});


    texture = TextureOpen("../assets/textures/pause.png", TEXTURE_FILTERING_NEAREST);

    InitUIRender();
    UIParse(&state, "../assets/ui/new.ui");
    
    // Create a slider
    // UISliderNew(UIFindElement(&state, "b1"), 0, 100, 5, 0.1);
    // UIFindElement(&state, "b1")->slider.modify_width = true;

    // Implement a show/hide button
    UIFindElement(&state, "menu-toggle")->event_func = tmp;

}

void EngineExit(){
    BundleFree(&bundle);
}


void EngineLoop(){

	Vector3 pos = {0, 0, 0};
	Vector3 color = {1, 0, 0};
	memcpy(&data[0], pos.v, sizeof(Vector3));
	memcpy(&data[3], color.v, sizeof(Vector3));
	data[6] = 1;
	AppendInstance(model.attr, data, *model.mesh, model.material->shader, 0, NULL);


    RenderText(&default_font, 1.001, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 30, TEXT_ALIGN_LEFT, "testing");


    // UIUpdate(&state);

    // UIInteract(&state);

    // UIRender(&state);

    UIPush(&state);

    PushRender();
}

/** WHEN COME BACK :
 * - Convert 'bundle' namespace into 'tuff' namespace
 * - make resource arrays statically allocated
 * - integrate physics sim
 * - set up UI for physics sim parameters and pause / play, etc..
*/
