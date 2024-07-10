#include <GL/glew.h>
#include <string.h>

#include "bundle.h"
#include "material.h"
#include "scene.h"
#include "event.h"
#include "renderer/renderer.h"
#include "render_text.h"

#include "engine.h"

Bundle bundle;
GLTF *sphere;
Material *mat;
Model model;


float data[64] = {0};
void EngineSetup(){

    bundle = BundleOpen(NULL);
    sphere = BundleGLTFOpen(&bundle, "models/sphere.gltf");
    mat = BundleMaterialOpen(&bundle, "materials/default.mat");
    model = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/sphere.gltf", true)->meshes[0], mat);

    MaterialUniformSetVec3(mat, "light_pos", (vec3){1, 1, 1});

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


    RenderText(&default_font, 1.001, SCREEN_WIDTH - 100, 10, TEXT_ALIGN_LEFT, "testing");

    PushRender();
}
