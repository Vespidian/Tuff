
// #include <GL/glew.h>
#include <cglm/cglm.h>

#include "vectorlib.h"
#include "ui.h"
#include "event.h"
#include "engine.h"


static iVector2 hold_pos;
static iVector2 diff_pos;
static Vector3 direction = {0, 0, 0};
static Vector3 view_position = {0, 0, 0};
static float view_distance = 10;
static float yaw, pitch = 0.25;
static float sensitivity = 0.005f;
static float move_sensitivity = 0.0025f;
static void MouseEvent(EventData event){
	if(!ui_focused){
		if(mouse_clicked){
			printf("CLICKED\n");
			hold_pos = mouse_pos;
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		if(mouse_released){
			printf("RELEASED\n");
			SDL_SetRelativeMouseMode(SDL_FALSE);
		}

		if(mouse_held){

			diff_pos.x = 0;
			diff_pos.y = 0;
			if((mouse_pos.x != hold_pos.x) || (mouse_pos.y != hold_pos.y)){
				diff_pos.x = mouse_pos.x - hold_pos.x;
				diff_pos.y = mouse_pos.y - hold_pos.y;

				SDL_WarpMouseInWindow(window, hold_pos.x, hold_pos.y);

			}

			if(event.keyStates[SDL_SCANCODE_LSHIFT]){
				Vector3 tmp = {-diff_pos.x * move_sensitivity * view_distance, -diff_pos.y * move_sensitivity * view_distance, 0};
				glm_vec3_rotate(tmp.v, -pitch * 2, (vec3){1, 0, 0});
				glm_vec3_rotate(tmp.v, yaw, (vec3){0, 1, 0});
				view_position.x += -tmp.x;
				view_position.y += tmp.y;
				view_position.z += tmp.z;

			}else{

				yaw += diff_pos.x * sensitivity;
				pitch += diff_pos.y * sensitivity;

				if(pitch > M_PI * 2){
					pitch -= M_PI * 2;
				}
				if(pitch < 0){
					pitch += M_PI * 2;
				}

				if(yaw > M_PI * 2){
					yaw -= M_PI * 2;
				}
				if(yaw < 0){
					yaw += M_PI * 2;
				}

				direction.x = yaw;
				direction.y = pitch;
			}
		}
	}
}

float zoom_speed = 0.075f;
static void Zoom(EventData event){
	if(event.e->wheel.y > 0){
		view_distance -= zoom_speed * view_distance;
	}else if(event.e->wheel.y < 0){
		view_distance += zoom_speed * view_distance;
	}
}

void InitCamera(){
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEWHEEL, Zoom);
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEMOTION, MouseEvent);
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEBUTTONDOWN, MouseEvent);
    BindEvent(EV_POLL_ACCURATE, SDL_MOUSEBUTTONUP, MouseEvent);
}

void UpdateCamera(){
    Vector3 tmp = {0, 0, view_distance};
    glm_vec3_rotate(tmp.v, -pitch * 2, (vec3){1, 0, 0});
    glm_vec3_rotate(tmp.v, -yaw, (vec3){0, 1, 0});
    glm_vec3_add(view_position.v, tmp.v, tmp.v);

    glm_mat4_identity(view_matrix);
    glm_translate(view_matrix, (vec3){0, 0, -view_distance});
    glm_rotate(view_matrix, pitch * 2, (vec3){1, 0, 0});
    glm_rotate(view_matrix, yaw, (vec3){0, 1, 0});
    glm_translate(view_matrix, view_position.v);
}