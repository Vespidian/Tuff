#include <GL/glew.h>
#include <cglm/cglm.h>
#include <string.h>

#include "debug.h"
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
    float radius_waals;
	float mass;

	unsigned int num_bonds;
	struct Bond *bonds[8];
}Atom;

typedef enum ATOM_TYPE{
	ATOM_HYDROGEN,
	ATOM_CARBON,
	ATOM_OXYGEN,
	ATOM_NITROGEN,
	ATOM_,
}ATOM_TYPE;

Atom hydrogen_atom = {
	.color = (Vector3){0.8, 0.8, 1},
	.radius = 1, 
	.mass = 0.5,
	.num_bonds = 0,
};

typedef struct Bond{
	Atom *a;
	Atom *b;

	float distance;
	float stiffness;
}Bond;

#define MOLECULE_MAX_ATOMS 999
#define MOLECULE_MAX_BONDS 999
#define MAX_BONDS 8



char *name;

unsigned int num_atoms = 0;
Atom atoms[1000];

unsigned int num_bonds = 0;
Bond bonds[1000];




float bound_size = 12;
Vector3 bound_max;
Vector3 bound_min;
float gravity = 0;

#define wall_damping 1;
bool paused = true;


Bundle bundle;
GLTF *sphere;
GLTF *cylinder;
Material *mat;
Model model;
Model cylinder_model;

UIState state;

static void ToggleMenu(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
    if(events & UI_MOUSE_CLICK){
        UIElement *e = UIFindElement(state, "menu");
        e->visible = !e->visible;
		e->visible_children = e->visible;
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

Atom *NewAtom(Vector3 position, Vector3 color, float radius, float radius_waals, float mass){
	Atom *a = &atoms[num_atoms++];
	a->position = position;
	a->prev_position = position;
	a->acceleration = (Vector3){0, 0, 0};
	a->color = color;
	a->radius = radius;
	a->radius_waals = radius_waals;
	a->mass = mass;

	a->num_bonds = 0;
	for(int i = 0; i < MAX_BONDS; i++){
		a->bonds[i] = NULL;
	}

	return a;
}

Atom *NewAtom_real(Vector3 position, ATOM_TYPE atom){
	Vector3 color = {0.8, 0.8, 0.8};
	float radius = 0.5;
	float radius_waals = 1;
	float mass = 1;

	switch(atom){
		case ATOM_HYDROGEN:
			color = (Vector3){1, 1, 1};
			radius = 			0.25;
			radius_waals = 		1.10;
			mass = 				0.1;
			break;
		case ATOM_CARBON:
			color = (Vector3){0.5, 0.5, 0.5};
			radius = 			0.7;
			radius_waals = 		1.7;
			mass = 				1.2;
			break;
		case ATOM_OXYGEN:
			color = (Vector3){1, 0, 0};
			radius = 			0.6;
			radius_waals = 		1.52;
			mass = 				1.5999;
			break;
		case ATOM_NITROGEN:
			color = (Vector3){0.2, 0.4, 1};
			radius = 			0.65;
			radius_waals = 		1.55;
			mass = 				1.4;
			break;
		// case ATOM_:
		// 	color = (Vector3){1, 1, 1};
		// 	radius = 			;
		// 	radius_waals = 		;
		// 	mass = 				;
		// 	break;
		default:
			break;
	}

	return NewAtom(position, color, radius, radius_waals, mass);
}

Bond *NewBond(Atom *a, Atom *b, float distance, float stiffness){
	Bond *bond = NULL;
	
	if((a->num_bonds < MAX_BONDS) && (b->num_bonds < MAX_BONDS)){
		bond = &bonds[num_bonds++];
		bond->a = a;
		bond->b = b;
		bond->distance = distance;
		bond->stiffness = stiffness;

		a->num_bonds++;
		b->num_bonds++;
		a->bonds[a->num_bonds] = bond;
		b->bonds[b->num_bonds] = bond;
	}

	return bond;
}

int FindChar(char *str, char c){
	int index = -1;
	if(str != NULL){
		for(int i = 0; str[i] != '\0'; i++){
			if(str[i] == c){
				index = i;
				break;
			}
		}
	}

	return index;
}

int ToInt(char *str){
	int num = 0;
	if(str != NULL){
		if(str[0] >= '0' && str[0] <= '9'){
			num += (str[0] - '0') * 100;
		}
		if(str[1] >= '0' && str[1] <= '9'){
			num += (str[1] - '0') * 10;
		}
		if(str[2] >= '0' && str[2] <= '9'){
			num += (str[2] - '0') * 1;
		}
	}

	return num;
}

// typedef struct Molecule{
//  char *name;

// 	unsigned int num_atoms;
// 	Atom atoms[MOLECULE_MAX_ATOMS];

// 	unsigned int num_bonds;
// 	Bond bonds[MOLECULE_MAX_BONDS];
// }Molecule;

void ParseMolfile(char *path){
	if(path != NULL){

		FILE *fp = fopen(path, "r");
		char *molfile_str = NULL;

		if(fp != NULL){
			fseek(fp, 0, SEEK_END);
			long file_length = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			if(file_length > -1){
				molfile_str = malloc(file_length + 1);
				if(molfile_str != NULL){
					size_t read_error_check = fread(molfile_str, 1, file_length, fp);
					if(read_error_check != file_length){
						DebugLog(D_WARN, "%s: error reading data from molfile", path);
						free(molfile_str);
						molfile_str = NULL;
					}
				}
			}else{
				DebugLog(D_WARN, "%s: error reading length of molfile", path);
			}
			fclose(fp);
			fp = NULL;
		}else{
			DebugLog(D_WARN, "%s: error opening molfile", path);
		}


		char *line = NULL;

		// Name
		int newline_prev = 0;
		int newline_index = FindChar(molfile_str, '\n') + 1;
		if(newline_index != -1){
			name = malloc(newline_index - newline_prev + 1);
			
			memcpy(name, molfile_str + newline_prev, newline_index - newline_prev);
			name[newline_index] = 0;
			printf("name: %s\n", name);

		}

		// Header
		newline_prev = newline_index;
		newline_index = FindChar(molfile_str + newline_prev, '\n') + newline_prev + 1;


		// Comment
		newline_prev = newline_index;
		newline_index = FindChar(molfile_str + newline_prev, '\n') + newline_prev + 1;


		// Counts
		unsigned int tmp_num_atoms = 0;
		unsigned int tmp_num_bonds = 0;

		newline_prev = newline_index;
		newline_index = FindChar(molfile_str + newline_prev, '\n') + newline_prev + 1;
		if(newline_index != -1){
			line = malloc(newline_index - newline_prev + 1);

			memcpy(line, molfile_str + newline_prev, newline_index - newline_prev);
			line[newline_index] = 0;
			printf("counts: %s\n", line);

			tmp_num_atoms = ToInt(line);
			tmp_num_bonds = ToInt(line + 4);

			free(line);
			line = NULL;
		}

		// Atoms
		for(int i = 0; i < tmp_num_atoms; i++){
			newline_prev = newline_index;
			newline_index = FindChar(molfile_str + newline_prev, '\n') + newline_prev + 1;
			if(newline_index != -1){
				line = malloc(newline_index - newline_prev + 1);

				memcpy(line, molfile_str + newline_prev, newline_index - newline_prev);
				line[newline_index] = 0;
				printf("atom line: %s\n", line);

				float x, y, z;
				x = atof(line);
				y = atof(line + 10);
				z = atof(line + 20);
				// NewAtom((Vector3){x, y, z}, (Vector3){1, 1, 1}, 0.5, 1, );

				// if(line[31] == 'H'){
				// 	atoms[num_atoms - 1].radius = 0.1;
				// }

				// van der Waals
				switch(line[31]){
					case 'H':
						NewAtom_real((Vector3){x, y, z}, ATOM_HYDROGEN);
						break;
					case 'C':
						NewAtom_real((Vector3){x, y, z}, ATOM_CARBON);
						break;
					case 'O':
						NewAtom_real((Vector3){x, y, z}, ATOM_OXYGEN);
						break;
					case 'N':
						NewAtom_real((Vector3){x, y, z}, ATOM_NITROGEN);
						break;
					default:
						NewAtom_real((Vector3){x, y, z}, ATOM_);
						break;
				}

				// Emprical
				// switch(line[31]){
				// 	case 'H':
				// 		NewAtom((Vector3){x, y, z}, (Vector3){1, 1, 1}, 0.25, 0.1);
				// 		break;
				// 	case 'C':
				// 		NewAtom((Vector3){x, y, z}, (Vector3){0.4, 0.4, 0.4}, 0.7, 1.2);
				// 		break;
				// 	case 'O':
				// 		NewAtom((Vector3){x, y, z}, (Vector3){0.8, 0.1, 0.1}, 0.6, 1.5999);
				// 		break;
				// 	default:
				// 		NewAtom((Vector3){x, y, z}, (Vector3){0.5, 0.5, 0.5}, 0.5, 1);
				// 		break;
				// }

				free(line);
				line = NULL;
			}
		}

		for(int i = 0; i < tmp_num_bonds; i++){
			newline_prev = newline_index;
			newline_index = FindChar(molfile_str + newline_prev, '\n') + newline_prev + 1;
			if(newline_index != -1){
				line = malloc(newline_index - newline_prev + 1);

				memcpy(line, molfile_str + newline_prev, newline_index - newline_prev);
				line[newline_index] = 0;

				float bond_length = atoms[ToInt(line) - 1].radius + atoms[ToInt(line + 3) - 1].radius;
				NewBond(&atoms[ToInt(line) - 1], &atoms[ToInt(line + 3) - 1], bond_length, 10);

				free(line);
				line = NULL;
			}
		}


		free(molfile_str);
		molfile_str = NULL;

	}
}

void EngineSetup(){
    UI_WINDOW_HEIGHT = SCREEN_HEIGHT;
    UI_WINDOW_WIDTH = SCREEN_WIDTH;


    bundle = BundleOpen(NULL);
    sphere = BundleGLTFOpen(&bundle, "models/sphere.gltf");
    cylinder = BundleGLTFOpen(&bundle, "models/cylinder.gltf");
    plane = BundleGLTFOpen(&bundle, "models/platform.gltf");
    mat = BundleMaterialOpen(&bundle, "materials/default.mat");
    // model = ModelNew(NULL, &sphere->meshes[0], mat);
    model = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/sphere.gltf", true)->meshes[0], mat);
	cylinder_model = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/cylinder.gltf", true)->meshes[0], mat);
    model2 = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/platform.gltf", true)->meshes[0], mat);

    MaterialUniformSetVec3(mat, "light_pos", (vec3){1, 1, 1});


    BindEvent(EV_POLL_ACCURATE, SDL_KEYDOWN, Grav);





	InitUIRender();
    UIParse(&state, "../assets/ui/new.ui");

    // // Create a slider
    // UISliderNew(UIFindElement(&state, "b1"), 0, 100, 5, 0.1);
    // UIFindElement(&state, "b1")->slider.modify_width = true;

    // Play / pause button
    UIFindElement(&state, "pause")->event_func = Pause;

	// Menu button
    UIFindElement(&state, "menu-toggle")->event_func = ToggleMenu;



	bound_max = (Vector3){bound_size, bound_size, bound_size};
	bound_min = (Vector3){-bound_size, -bound_size, -bound_size};

	// CO2
	// NewAtom((Vector3){0, 0, 0}, (Vector3){1, 0.1, 0.1}, 1.70, 12.011);
	// NewAtom((Vector3){0, 0, -1}, (Vector3){1, 1, 1}, 1.52, 15.999);
	// NewAtom((Vector3){0, 0, 1}, (Vector3){1, 1, 1}, 1.52, 15.999);
	// NewBond(&atoms[0], &atoms[1], 1.163, 10);
	// NewBond(&atoms[0], &atoms[2], 1.163, 10);
	// NewBond(&atoms[2], &atoms[1], 2*1.163, 10);
	// H2O
	// NewAtom((Vector3){0, 1, 0}, (Vector3){1, 1, 1}, 1.52, 15.999);
	// NewAtom((Vector3){0, 0, -1}, (Vector3){1, 0.1, 0.1}, 1.1, 1.008);
	// NewAtom((Vector3){0, 0, 1}, (Vector3){1, 1, 1}, 1.1, 1.008);
	// NewBond(&atoms[0], &atoms[1], 0.957, 1);
	// NewBond(&atoms[0], &atoms[2], 0.957, 1);
	// NewBond(&atoms[2], &atoms[1], 1.510, 1);


	// for(int i = 0; i < 20; i++){
	// 	NewAtom(
	// 		(Vector3){
	// 			((float)rand() / (float)RAND_MAX - 0.5) * 2 * bound_size, 
	// 			((float)rand() / (float)RAND_MAX - 0.5) * 2 * bound_size, 
	// 			((float)rand() / (float)RAND_MAX - 0.5) * 2 * bound_size
	// 		},
	// 		// (Vector3){
	// 		// 	(float)rand() / (float)RAND_MAX, 
	// 		// 	(float)rand() / (float)RAND_MAX, 
	// 		// 	(float)rand() / (float)RAND_MAX
	// 		// },
	// 		(Vector3){0.3, 0.3, 0.3},
	// 		0.5 + (float)rand() / (float)RAND_MAX * 0.25,
	// 		0.5 + (float)rand() / (float)RAND_MAX * 0.25
	// 	);
	// }

	ParseMolfile("chemicals/octane.mol");

}

void EngineExit(){
	BundleFree(&bundle);
}

void ApplyBounds(Atom *atom){
	if((atom->position.x - atom->radius_waals) < bound_min.x){
		float tmp = atom->position.x - atom->prev_position.x;
		atom->position.x = bound_min.x + atom->radius_waals;
		atom->prev_position.x = atom->position.x + tmp * wall_damping;
	}
	if((atom->position.x + atom->radius_waals) > bound_max.x){
		float tmp = atom->position.x - atom->prev_position.x;
		atom->position.x = bound_max.x - atom->radius_waals;
		atom->prev_position.x = atom->position.x + tmp * wall_damping;
	}

	if((atom->position.y - atom->radius_waals) < bound_min.y){
		float tmp = atom->position.y - atom->prev_position.y;
		atom->position.y = bound_min.y + atom->radius_waals;
		atom->prev_position.y = atom->position.y + tmp * wall_damping;
	}
	if((atom->position.y + atom->radius_waals) > bound_max.y){
		float tmp = atom->position.y - atom->prev_position.y;
		atom->position.y = bound_max.y - atom->radius_waals;
		atom->prev_position.y = atom->position.y + tmp * wall_damping;
	}

	if((atom->position.z - atom->radius_waals) < bound_min.z){
		float tmp = atom->position.z - atom->prev_position.z;
		atom->position.z = bound_min.z + atom->radius_waals;
		atom->prev_position.z = atom->position.z + tmp * wall_damping;
	}
	if((atom->position.z + atom->radius_waals) > bound_max.z){
		float tmp = atom->position.z - atom->prev_position.z;
		atom->position.z = bound_max.z - atom->radius_waals;
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

	float force = bond.stiffness * (bond.distance - distance);
	// float d = 1;
	// float c = sqrt(1 / d);
	// float force = d * (exp(-2 * (distance - 1) * c) - 2 * exp(-(distance - 1) * c)) + d;

	Vector3 n;
	glm_vec3_copy(normal.v, n.v);
	glm_normalize(n.v);
	// glm_vec3_scale(n.v, force / (b->mass / total_mass), n.v);
	glm_vec3_scale(n.v, force / (total_mass), n.v);

	glm_vec3_add(a->acceleration.v, n.v, a->acceleration.v);


	// glm_vec3_scale(n.v, -b->mass / a->mass, n.v);
	glm_vec3_scale(n.v, -1, n.v);
	glm_vec3_add(b->acceleration.v, n.v, b->acceleration.v);
}

bool CheckBonded(Atom *a, Atom *b){
	bool ret = false;
	for(int i = 0; i < MAX_BONDS; i++){
		if(a->bonds[i] == NULL){
			continue;
		}
		if((a->bonds[i]->a == b) || (a->bonds[i]->b == b)){
			ret = true;
			break;
		}
	}
	return ret;
}

void ApplyCollision(Atom *a, Atom *b, bool preserve, int id, int id2){
	if(!CheckBonded(a, b)){
		// {

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
		if(distance < (a->radius_waals + b->radius_waals)){


			float delta = a->radius_waals + b->radius_waals - distance;
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
				total_energy += (fabs(glm_vec3_norm(v.v)) * fabs(glm_vec3_norm(v.v)) / (timestep * timestep) * atoms[i].mass / 2) + (atoms[i].mass * -gravity * (atoms[i].position.y + bound_size - atoms[i].radius));

			}
			// total_energy /= (float)num_atoms;
			// printf("%f\n", total_energy);
		}
		// atoms[2].position = (Vector3){0, 13, 0};
	}
	RenderText(&default_font, 1.001, 10, 60, TEXT_ALIGN_LEFT, "Total energy: %f", total_energy);

	for(int i = 0; i < num_atoms; i++){
		memcpy(&data[0], atoms[i].position.v, sizeof(Vector3));
		memcpy(&data[3], atoms[i].color.v, sizeof(Vector3));
		// data[6] = atoms[i].radius;
		data[6] = atoms[i].radius_waals;

    	AppendInstance(model.attr, data, *model.mesh, model.material->shader, 1, texture_array);
	}

	// Render floor surface
	data[0] = 0;
	data[1] = -bound_size;
	data[2] = 0;
	data[3] = 1;
	data[4] = 1;
	data[5] = 1;
	data[6] = bound_size;
	AppendInstance(model2.attr, data, *model2.mesh, model2.material->shader, 1, texture_array);

	

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
