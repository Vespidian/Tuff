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

typedef struct Atom{
    Vector3 position;
    Vector3 prev_position;
    Vector3 acceleration;

	Vector3 color;
    float radius;
	float mass;
}Atom;

typedef struct Bond{
	Atom *a;
	Atom *b;

	float distance;
	float stiffness;
}Bond;

#define num_atoms 14
#define num_bonds 6
Atom atoms[num_atoms];
Bond bonds[num_bonds];
float bound_size = 6;
Vector3 bound_max;
Vector3 bound_min;
float gravity = 0;

#define wall_damping 1;
bool paused = true;


Bundle bundle;
GLTF *sphere;
Material *mat;
Model model;

UIState state;

void tmp(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
    if(events & UI_MOUSE_CLICK){
        UIElement *e = UIFindElement(state, "idk");
        e->visible = !e->visible;
    }
}

static void Pause(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
    if((events & UI_MOUSE_CLICK) != 0){
		paused = !paused;
    }
}


static void Grav(EventData event){
	if(event.keyStates[SDL_SCANCODE_SPACE]){
		if(gravity == 0){
			gravity = -2;
		}else{
			gravity = 0;
		}
	}else if(event.keyStates[SDL_SCANCODE_0]){
		paused = !paused;
	}
}



GLTF *plane;
Model model2;

void EngineSetup(){
    UI_WINDOW_HEIGHT = SCREEN_HEIGHT;
    UI_WINDOW_WIDTH = SCREEN_WIDTH;


    bundle = BundleOpen(NULL);
    sphere = BundleGLTFOpen(&bundle, "models/sphere.gltf");
    // plane = BundleGLTFOpen(&bundle, "models/plane.gltf");
    mat = BundleMaterialOpen(&bundle, "materials/default.mat");
    // model = ModelNew(NULL, &sphere->meshes[0], mat);
    model = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/sphere.gltf", true)->meshes[0], mat);
    // model2 = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/plane.gltf", true)->meshes[0], mat);

    MaterialUniformSetVec3(mat, "light_pos", (vec3){1, 1, 1});


    BindEvent(EV_POLL_ACCURATE, SDL_KEYDOWN, Grav);





	InitUIRender();
    UIParse(&state, "../assets/ui/new.ui");

    // // Create a slider
    // UISliderNew(UIFindElement(&state, "b1"), 0, 100, 5, 0.1);
    // UIFindElement(&state, "b1")->slider.modify_width = true;

    // // Implement a show/hide button
    // UIFindElement(&state, "hehe")->event_func = tmp;

    // Play / pause button
    UIFindElement(&state, "pause")->event_func = Pause;



	bound_max = (Vector3){bound_size, 2*bound_size, bound_size};
	bound_min = (Vector3){-bound_size, 0, -bound_size};

	for(int i = 0; i < num_atoms; i++){
		atoms[i].color.x = (float)rand() / (float)RAND_MAX;
		atoms[i].color.y = (float)rand() / (float)RAND_MAX;
		atoms[i].color.z = (float)rand() / (float)RAND_MAX;

		// atoms[i].radius = (float)rand() / (float)RAND_MAX + 0.1;
		// atoms[i].mass = atoms[i].radius;
		atoms[i].radius = 0.5;
		atoms[i].mass = 1;

		atoms[i].acceleration.x = 0;
		atoms[i].acceleration.y = 0;
		atoms[i].acceleration.z = 0;

		atoms[i].position.x = ((float)rand() / (float)RAND_MAX - 0.5) * 16;
		atoms[i].position.y = ((float)rand() / (float)RAND_MAX - 0.5) * 16 + bound_size;
		atoms[i].position.z = ((float)rand() / (float)RAND_MAX - 0.5) * 16;

		atoms[i].prev_position.x = atoms[i].position.x;
		atoms[i].prev_position.y = atoms[i].position.y;
		atoms[i].prev_position.z = atoms[i].position.z;
	}

	atoms[0].position.x = -2;
	atoms[0].position.y = 2;
	atoms[0].position.z = 2;

	atoms[0].prev_position.x = -2;
	atoms[0].prev_position.y = 2;
	atoms[0].prev_position.z = 2;

	// atoms[0].prev_position.z = 0.1;
	atoms[0].mass = 1;
	atoms[0].radius = 0.75;


	atoms[1].position.x = 2;
	atoms[1].position.y = 2;
	atoms[1].position.z = 2;

	atoms[1].prev_position.x = 2;
	atoms[1].prev_position.y = 2;
	atoms[1].prev_position.z = 2.04;

	atoms[1].mass = 1;
	atoms[1].radius = 0.75;

	// atoms[1].prev_position.z = 0.1;
	// for(int i = 0; i < 10; i++){
	// 	bonds[i].a = &atoms[i + 2];
	// 	bonds[i].b = &atoms[i + 3];
	// 	bonds[i].distance = 0.01;
	// 	bonds[i].stiffness = 20;

	// }
	// bonds[9].b = &atoms[2];
	for(int i = 0; i < num_bonds; i++){
		bonds[i].stiffness = 10;
		bonds[i].distance = 0.1;
	}
	bonds[0].a = &atoms[0];
	bonds[0].b = &atoms[1];

	bonds[1].a = &atoms[0];
	bonds[1].b = &atoms[2];

	bonds[2].a = &atoms[0];
	bonds[2].b = &atoms[3];

	bonds[3].a = &atoms[1];
	bonds[3].b = &atoms[3];

	bonds[4].a = &atoms[2];
	bonds[4].b = &atoms[3];

	bonds[5].a = &atoms[1];
	bonds[5].b = &atoms[2];

}

void EngineExit(){
	BundleFree(&bundle);
}

void ApplyBounds(Atom *atom){
	if((atom->position.x - atom->radius) < bound_min.x){
		float tmp = atom->position.x - atom->prev_position.x;
		atom->position.x = bound_min.x + atom->radius;
		atom->prev_position.x = atom->position.x + tmp * wall_damping;
	}
	if((atom->position.x + atom->radius) > bound_max.x){
		float tmp = atom->position.x - atom->prev_position.x;
		atom->position.x = bound_max.x - atom->radius;
		atom->prev_position.x = atom->position.x + tmp * wall_damping;
	}

	if((atom->position.y - atom->radius) < bound_min.y){
		float tmp = atom->position.y - atom->prev_position.y;
		atom->position.y = bound_min.y + atom->radius;
		atom->prev_position.y = atom->position.y + tmp * wall_damping;
	}
	if((atom->position.y + atom->radius) > bound_max.y){
		float tmp = atom->position.y - atom->prev_position.y;
		atom->position.y = bound_max.y - atom->radius;
		atom->prev_position.y = atom->position.y + tmp * wall_damping;
	}

	if((atom->position.z - atom->radius) < bound_min.z){
		float tmp = atom->position.z - atom->prev_position.z;
		atom->position.z = bound_min.z + atom->radius;
		atom->prev_position.z = atom->position.z + tmp * wall_damping;
	}
	if((atom->position.z + atom->radius) > bound_max.z){
		float tmp = atom->position.z - atom->prev_position.z;
		atom->position.z = bound_max.z - atom->radius;
		atom->prev_position.z = atom->position.z + tmp * wall_damping;
	}

}

void ApplyBond(Bond bond){
	Atom *a = bond.a;
	Atom *b = bond.b;

	Vector3 normal;
	glm_vec3_sub(a->position.v, b->position.v, normal.v);

	float distance = glm_vec3_norm(normal.v);
	glm_vec3_normalize(normal.v);

	float total_mass = a->mass + b->mass;

	/** Spring **/

	Vector3 n;
	glm_vec3_copy(normal.v, n.v);
	glm_normalize(n.v);
	glm_vec3_scale(n.v, bond.stiffness * (3 - distance) / (b->mass / total_mass), n.v);

	glm_vec3_add(a->acceleration.v, n.v, a->acceleration.v);


	glm_vec3_scale(n.v, -b->mass / a->mass, n.v);
	glm_vec3_add(b->acceleration.v, n.v, b->acceleration.v);
}

void ApplyCollision(Atom *a, Atom *b, bool preserve, int id, int id2){
	Vector3 normal;
	glm_vec3_sub(a->position.v, b->position.v, normal.v);

	float distance = glm_vec3_norm(normal.v);
	glm_vec3_normalize(normal.v);

	// float total_mass = a->mass + b->mass;

	/** Entity - entity gravity **/
	// Vector3 grav;
	// glm_vec3_copy(normal.v, grav.v);
	// glm_normalize(grav.v);
	// glm_vec3_scale(grav.v, b->mass / -(distance * distance), grav.v);
	// glm_vec3_add(a->acceleration.v, grav.v, a->acceleration.v);

	// /** Spring **/
	// float stiffness = 1;
	// if(id == 0 && id2 == 1){
	// 	Vector3 n;
	// 	glm_vec3_copy(normal.v, n.v);
	// 	glm_normalize(n.v);
	// 	glm_vec3_scale(n.v, stiffness * (3 - distance) / (b->mass / total_mass), n.v);

	// 	glm_vec3_add(a->acceleration.v, n.v, a->acceleration.v);


	// 	glm_vec3_scale(n.v, -b->mass / a->mass, n.v);
	// 	glm_vec3_add(b->acceleration.v, n.v, b->acceleration.v);
	// }


	// If the atoms are too close together, push em away from eachother
	if(distance < (a->radius + b->radius)){


		float delta = a->radius + b->radius - distance;
		glm_vec3_scale(normal.v, delta * 0.5, normal.v);

		Vector3 va;
		Vector3 vb;
		glm_vec3_sub(a->position.v, a->prev_position.v, va.v);
		glm_vec3_sub(b->position.v, b->prev_position.v, vb.v);


			glm_vec3_add(a->position.v, normal.v, a->position.v);
			glm_vec3_sub(b->position.v, normal.v, b->position.v);


		if(preserve){
			glm_vec3_copy(a->position.v, a->prev_position.v);
			glm_vec3_copy(b->position.v, b->prev_position.v);

			float damp = 1;

			glm_normalize(normal.v);
			glm_vec3_scale(normal.v, ((a->mass - b->mass)/(a->mass + b->mass) * glm_vec3_norm(va.v)) + (2*b->mass/(a->mass+b->mass)*glm_vec3_norm(vb.v))*damp, normal.v);
			glm_vec3_add(a->position.v, normal.v, a->position.v);


			glm_normalize(normal.v);
			// glm_vec3_scale(normal.v, glm_vec3_norm(vb.v) * a->mass, normal.v);
			glm_vec3_scale(normal.v, (2*a->mass/(a->mass+b->mass)*glm_vec3_norm(va.v)) + ((b->mass - a->mass)/(a->mass + b->mass) * glm_vec3_norm(vb.v))*damp, normal.v);
			glm_vec3_sub(b->position.v, normal.v, b->position.v);

		}
	}
}

float data[64] = {0};
Texture texture_array[16] = {0};
void EngineLoop(){

	float total_energy = 0;
	if(!paused){

		for(int j = 0; j < 1; j++){
			float timestep = 0.1;
			// BONDS
			for(int i = 0; i < num_bonds; i++){
				ApplyBond(bonds[i]);
			}
			for(int i = 0; i < num_atoms; i++){
				atoms[i].acceleration.y += gravity;
				glm_vec3_scale(atoms[i].acceleration.v, timestep * timestep, atoms[i].acceleration.v); // Scale acc by square of tiemstep
				glm_vec3_add(atoms[i].position.v, atoms[i].acceleration.v, atoms[i].position.v);
				glm_vec3_zero(atoms[i].acceleration.v);


				for(int k = 0; k < num_atoms; k++){
					if(k == i){
						continue;
					}
					ApplyCollision(&atoms[i], &atoms[k], true, i, k);
				}

				ApplyBounds(&atoms[i]);

				Vector3 tmp;
				glm_vec3_copy(atoms[i].position.v, tmp.v);

				glm_vec3_scale(atoms[i].position.v, 2, atoms[i].position.v); // Scale pos by 2
				glm_vec3_sub(atoms[i].position.v, atoms[i].prev_position.v, atoms[i].position.v);


				glm_vec3_copy(tmp.v, atoms[i].prev_position.v);

				// for(int k = 0; k < num_atoms; k++){
				// 	if(k == i){
				// 		continue;
				// 	}
				// 	ApplyCollision(&atoms[i], &atoms[k], true);
				// }

				// if(i == 0){
				// 	printf("%f, %f, %f\n", atoms[i].position.x, atoms[i].position.y, atoms[i].position.z);
				// }
				Vector3 v;
				glm_vec3_sub(atoms[i].position.v, atoms[i].prev_position.v, v.v);
				total_energy += (fabs(glm_vec3_norm(v.v)) * fabs(glm_vec3_norm(v.v)) / (timestep * timestep) * atoms[i].mass / 2) + (atoms[i].mass * -gravity * (atoms[i].position.y - atoms[i].radius));

			}
			// total_energy /= (float)num_atoms;
			// printf("%f\n", total_energy);
		}
		// atoms[2].position = (Vector3){0, 13, 0};
	}
	RenderText(&default_font, 1.001, 10, 10, TEXT_ALIGN_LEFT, "Total energy: %f", total_energy);

	for(int i = 0; i < num_atoms; i++){
		memcpy(&data[0], atoms[i].position.v, sizeof(Vector3));
		memcpy(&data[3], atoms[i].color.v, sizeof(Vector3));
		data[6] = atoms[i].radius;

    	AppendInstance(model.attr, data, *model.mesh, model.material->shader, 1, texture_array);
	}

	// Vector3 planepos = {0, -bound_size, 0};
	// memcpy(&data[0], planepos.v, sizeof(Vector3));
	// memcpy(&data[3], atoms[0].color.v, sizeof(Vector3));
	// data[6] = 10;

	// AppendInstance(model.attr, data, *model2.mesh, model2.material->shader, 1, texture_array);
	// RenderText(&default_font, 1.001, 10, 10, TEXT_ALIGN_LEFT, "this is a testt");
	// Vector4 dst = {0, 0, 64, 64};
	// extern AttribArray quad_vao;
	// RenderQuad(undefined_texture, NULL, &dst, 0, (Vector4){1, 1, 1, 1}, 0);
	// AppendInstance(quad_vao, data, *model.mesh, model.material->shader, 1, texture_array);
	// UIUpdate(&state);

    // UIInteract(&state);

    // UIRender(&state);

    UIPush(&state);

    PushRender();
}



// WHEN COME BACK: figure out why we cant render 2 separate instances in the same loop
