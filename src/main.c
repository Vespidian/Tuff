#include <GL/glew.h>
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




#include <libserialport.h>

#define PORT_TIMEOUT 50
struct sp_port **ports = NULL;
uint8_t port_id = 0;


uint8_t buffer[8000];
int buffer_length = 1024;
enum sp_return serial_error_code = SP_OK;

bool serial_initialized = false;




// OSCOPE CONFIG VARIABLES
typedef struct __attribute__((packed)){
	bool trigger_threshold_enable :		1;
	bool trigger_edge_enable :			1;
	bool single_trigger	:				1;
	uint8_t padding :					5;
	uint16_t trigger_threshold;
	int16_t trigger_edge_slope;
}Config;

Config oscope_config;





Bundle bundle;
GLTF *sphere;
Material *mat;
Model model;
Shader fftshader;

Texture texture;
UIState state;

Texture tex_button;
Texture tex_button_pressed;
bool button_pressed = false;

// OSCOPE CONFIG VARIABLES
bool trigger_threshold_enable = true;
bool trigger_edge_enable = false;
bool single_trigger = false;

uint16_t trigger_threshold = 2048;
int16_t trigger_edge_slope = 200;





void serial_refresh(){
		// Delete listing element texts
		UIElement *port_listing = UIFindElement(&state, "serial-list");
		for(int i = 0; i < port_listing->num_children; i++){
			free(port_listing->children[i]->text);
			port_listing->children[i]->text = NULL;
		}
		
		sp_list_ports(&ports);

		for(int i = 0; (ports[i] != 0) && (i < port_listing->num_children); i++){
			port_listing->children[i]->text = sp_get_port_name(ports[i]);
			printf("%s\n", port_listing->children[i]->text);
		}
}

void serial_connect(uint8_t port_index){
	if(ports != NULL){
		if((serial_error_code = sp_open(ports[port_index], SP_MODE_READ_WRITE)) == SP_OK){
			port_id = port_index;
			sp_set_baudrate(ports[port_index], 460800);
			sp_set_bits(ports[port_index], 8);
			sp_set_parity(ports[port_index], SP_PARITY_NONE);
			sp_set_stopbits(ports[port_index], 1);
			sp_set_flowcontrol(ports[port_index], SP_FLOWCONTROL_NONE);
			sp_flush(ports[port_index], SP_BUF_BOTH);
			serial_initialized = true;	
		}else{
			DebugLog(D_ERR, "error: couldn't open serial port '%s'. error code %d\n", sp_get_port_name(ports[port_index]), serial_error_code);
			printf("error opening serial port\n");
			return;
		}
	}
}



extern void CheckGLErrors(const char *file, int line);
#define GLCall CheckGLErrors(__FILE__, __LINE__);

void button(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
	if(events & UI_MOUSE_CLICK){
		button_pressed = !button_pressed;
		if(button_pressed){
			element->texture = tex_button;
		}else{
			element->texture = tex_button_pressed;
		}
	}
}

void button_trigger_threshold(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
	if(events & UI_MOUSE_CLICK){
		if(serial_initialized){
			sp_blocking_write(ports[port_id], "thresh_togg\n", 12, 2);
		}
	}
}

void button_trigger_edge(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
	if(events & UI_MOUSE_CLICK){
		trigger_edge_enable = !trigger_edge_enable;
		if(serial_initialized){
			sp_blocking_write(ports[port_id], "edge_togg\n", 10, 2);
		}
		// printf("Edge: ");
		// if(trigger_edge_enable){
		// 	printf("on\n");
		// 	element->texture = tex_button_pressed;
		// 	// Send command to ENABLE edge trigger
		// }else{
		// 	printf("off\n");
		// 	element->texture = tex_button;
		// 	// Send command to DISABLE edge trigger
		// }
	}
}

void button_serial_refresh(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
	if(events & UI_MOUSE_CLICK){
		serial_refresh();
	}
}

void button_serial_port_select(UIState *state, UIElement *element, UI_MOUSE_EVENT events){
	if(events & UI_MOUSE_CLICK){
		if(!serial_initialized && element->name != NULL && element->text != NULL){
			serial_connect(element->name[0] - '0');
		}
		// printf("%c\n", element->name[0]);
	}
}

static void AssignUIFunctions(){
	// ASSIGNING UI ELEMENT FUNCTIONS
	UIFindElement(&state, "button-trigger-threshold")->event_func = button_trigger_threshold;
	UIFindElement(&state, "button-trigger-edge")->event_func = button_trigger_edge;
	UIFindElement(&state, "button-serial-refresh")->event_func = button_serial_refresh;
	char tmp_name[16] = "0-serial-list";
	for(int i = 0; i < UIFindElement(&state, "serial-list")->num_children; i++){
		tmp_name[0] = i + '0';
		UIFindElement(&state, tmp_name)->event_func = button_serial_port_select;
	}
	// UIFindElement(&state, "0-serial-list")->event_func = button_serial_port_select;
	// UIFindElement(&state, "1-serial-list")->event_func = button_serial_port_select;
	// UIFindElement(&state, "2-serial-list")->event_func = button_serial_port_select;
	// UIFindElement(&state, "3-serial-list")->event_func = button_serial_port_select;
	UIFindElement(&state, "channel1")->event_func = button;
}

static void ReloadUI(EventData event){
	if(event.keyStates[SDL_SCANCODE_Z]){
		printf("UI Reloaded\n");
		UIFreeState(&state);
		UIParse(&state, "../assets/ui/oscope.ui");
		AssignUIFunctions();
	}
}

// SETUP
unsigned int graph_vao;
unsigned int graph_vbo;
// float graph_array[1024];
Vector2 graph_array[512];


float data[64] = {0};
void EngineSetup(){
	UI_WINDOW_HEIGHT = 800;
	UI_WINDOW_WIDTH = 800;

	bundle = BundleOpen(NULL);
	sphere = BundleGLTFOpen(&bundle, "models/sphere.gltf");
	mat = BundleMaterialOpen(&bundle, "materials/default.mat");
	model = ModelNew(NULL, &BundleGLTFFind(&bundle, "models/sphere.gltf", true)->meshes[0], mat);

	MaterialUniformSetVec3(mat, "light_pos", (vec3){1, 1, 1});
	ShaderUniformSetVec3(mat->shader, "light_pos", (vec3){1, 1, 1});
	fftshader = ShaderOpen("shaders/graph.shader");

	texture = TextureOpen("../assets/textures/pause.png", TEXTURE_FILTERING_NEAREST);

	InitUIRender();
	// UIParse(&state, "../assets/ui/new.ui");
	UIParse(&state, "../assets/ui/oscope.ui");
	
	// Create a slider
	UISliderNew(UIFindElement(&state, "slider-num-samples"), 0, 3.3, 0, 0.01);
	// UIFindElement(&state, "b1")->slider.modify_width = true;
	BindEvent(EV_POLL_ACCURATE, SDL_KEYDOWN, ReloadUI);
	// Implement a show/hide button
	tex_button = TextureOpen("../assets/textures/button.png", TEXTURE_FILTERING_NEAREST);
	tex_button_pressed = TextureOpen("../assets/textures/button_pressed.png", TEXTURE_FILTERING_NEAREST);

	AssignUIFunctions();



	glGenVertexArrays(1, &graph_vao);
	glBindVertexArray(graph_vao);

	glGenBuffers(1, &graph_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, graph_vbo);

	for(int i = 0; i < 512; i++){
		graph_array[i].x = i / 512.0 * 800 + 200;
		graph_array[i].y = sin((float)i / 512.0 * M_PI * 2) * 400;
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof(graph_array), graph_array, GL_DYNAMIC_DRAW);

	serial_refresh();

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


	// // RenderTextEx(&default_font, 1.001, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 30, (Vector4){1, 1, 1, 1}, TEXT_ALIGN_LEFT, 0, -1, "testing");


	// UIUpdate(&state);

	// UIInteract(&state);

	// UIRender(&state);

	UIPush(&state);


	// // UPDATE + RENDER
	glBindVertexArray(graph_vao);
	ShaderSet(&fftshader);
	glBindBuffer(GL_ARRAY_BUFFER, graph_vbo);

	if(serial_initialized){
		unsigned int val = 0;
		// for(int k = 0; k < 2; k++){
			// printf("0\n");
			uint32_t t = SDL_GetTicks();
			while(val != 0xffff){
				buffer[1] = buffer[0];
				sp_blocking_read_next(ports[port_id], &buffer, 1, PORT_TIMEOUT);
				val = buffer[0] + (buffer[0 + 1] << 8);
				if((SDL_GetTicks() - t) > PORT_TIMEOUT){
					printf("disconnected\n");
					serial_initialized = false;
					sp_close(ports[port_id]);
					goto skip_serial_frame;
				}
			}
			// printf("1\n");
			sp_blocking_read_next(ports[port_id], &buffer, 2, PORT_TIMEOUT);
			// printf("loop_counter = %d\n", (unsigned int)(buffer[0] + (buffer[0 + 1] << 8)));
			// uint8_t flags = 0;
			// sp_blocking_read_next(ports[port_id], &flags, 1, PORT_TIMEOUT);
			// printf("%d\n", flags);


			uint8_t conf[5] = {0};
			sp_blocking_read_next(ports[port_id], &conf, 5, PORT_TIMEOUT);

			*(uint8_t *)&oscope_config = conf[0];
			oscope_config.trigger_threshold = conf[1] | (conf[2] << 8);
			oscope_config.trigger_edge_slope = conf[3] | (conf[4] << 8);

			// printf("%d\n", d[0] | (d[1] << 8));


			// printf("START\n");
			// printf("2\n");
			sp_blocking_read(ports[port_id], &buffer[0], buffer_length, PORT_TIMEOUT);
			for(int i = 0; i < buffer_length; i += 2){
				val = (unsigned int)(buffer[i] + (buffer[i + 1] << 8));
				val = 4096 - val;
				graph_array[i / 2].y = val / 4096.0 * 750;
				// printf("%d\n", val);
				if((val > 4096) && (val != 0xffff)){
					i++;
				}
				if(val == 0xffff){
					// printf("WOAHAOHAOH\n");
				}
			}
		// }

		UIElement *button_trig = UIFindElement(&state, "button-trigger-threshold");
		if(oscope_config.trigger_threshold_enable){
			button_trig->texture = tex_button_pressed;
		}else{
			button_trig->texture = tex_button;
		}
		button_trig = UIFindElement(&state, "button-trigger-edge");
		if(oscope_config.trigger_edge_enable){
			button_trig->texture = tex_button_pressed;
		}else{
			button_trig->texture = tex_button;
		}


		skip_serial_frame:
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(graph_array), graph_array);
	glEnableVertexAttribArray(0); // Now we config the first attribute
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
	GLCall(glDrawArrays(GL_LINE_STRIP, 0, 512));

	PushRender();
}

/** WHEN COME BACK :
 * - Convert 'bundle' namespace into 'tuff' namespace
 * - make resource arrays statically allocated
 * - integrate physics sim
 * - set up UI for physics sim parameters and pause / play, etc..
*/


/** WHEN COME BACK SERIAL :
 *  - look at 'send_receive.c' example file in libserialport source
 *  	- can look at the prototype main.c file too
 *  - implement error checking properly
 *  - 
 * 
 */