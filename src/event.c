#include "global.h"
#include "debug.h"
#include "renderer/renderer.h"
#include "text_event.h"

#include "event.h"

SDL_Event e;
Vector2_i mouse_pos = {0, 0};
Vector2_i mouse_pos_previous = {0, 0};

InputEvent *events;
int num_events = 0;

bool enable_input = true;

bool mouse_held = false;
bool mouse_clicked = false;
bool mouse_lifted = false;
int scroll_value = 0;
Vector2 mouse = {0, 0};

//Event predefinitions
static void MouseClicked();
static void MouseUp();
static void Scroll(EventData event);
void KeyEvents_quick();
void WindowResize();

//Event management
void InitEvents(){
	events = malloc(sizeof(InputEvent));

	#ifdef DEBUG_BUILD
		BindKeyEvent(Quit, 0x1B, SDL_KEYDOWN);//escape
	#endif
	BindEvent(EV_POLL_ACCURATE, SDL_MOUSEBUTTONDOWN, MouseClicked);
	BindEvent(EV_POLL_ACCURATE, SDL_MOUSEBUTTONUP, MouseUp);
	BindEvent(EV_POLL_ACCURATE, SDL_MOUSEWHEEL, Scroll);
	BindEvent(EV_POLL_ACCURATE, SDL_WINDOWEVENT, WindowResize);
	BindEvent(EV_POLL_ACCURATE, SDL_QUIT, Quit);

	InitTextEvent();
	DebugLog(D_ACT, "Initialized event subsystem");
}

void BindEvent(EventPollType_et pollType, Uint32 eventType, EV_Function function){
	events = realloc(events, sizeof(InputEvent) * (num_events + 1));
	events[num_events] = (InputEvent){pollType, eventType, function, false, 0x00};
	num_events++;
}

void PollEvents(){
	const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
	mouse_pos_previous = mouse_pos;
	const Uint32 mouseState = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
	if(mouseState){
		mouse_held = true;
	}
	while(SDL_PollEvent(&e)){
		if(text_input){
			PollText(&e);
		}
		for(int i = 0; i < num_events; i++){
			if(events[i].pollType == EV_POLL_ACCURATE){
				if(events[i].eventType == e.type){
					if(events[i].isKeyPress){
						if(e.key.keysym.sym == events[i].keyCode){
							events[i].function((EventData){&e, keyStates, &mouseState});
						}
					}else{
						events[i].function((EventData){&e, keyStates, &mouseState});
					}
				}
			}
		}
	}
	for(int i = 0; i < num_events; i++){
		if(events[i].pollType == EV_POLL_QUICK){
			if(events[i].isKeyPress){
				if(keyStates[events[i].scanCode]){
					events[i].function((EventData){NULL, keyStates, &mouseState});
				}
			}else{
				events[i].function((EventData){NULL, keyStates, &mouseState});
			}
		}
	}
}

void EventListener(){
	mouse_clicked = false;
	mouse_held = false;
	mouse_lifted = false;
	scroll_value = 0;

	if(enable_input){
		PollEvents();
	}
}

void BindQuickKeyEvent(EV_Function function, Uint8 scanCode){
	BindEvent(EV_POLL_QUICK, SDL_KEYDOWN, function);
	events[num_events - 1] = (InputEvent){EV_POLL_QUICK, SDL_KEYDOWN, function, true, 0x00, scanCode};
}

void BindKeyEvent(EV_Function function, char keyCode, Uint32 keyPressType){
	//Make sure keypresstype is keydown or keyup
	if(keyPressType != SDL_KEYDOWN && keyPressType != SDL_KEYUP){
		keyPressType = SDL_KEYDOWN;
	}
	BindEvent(EV_POLL_ACCURATE, keyPressType, function);
	events[num_events - 1] = (InputEvent){EV_POLL_ACCURATE, keyPressType, function, true, keyCode};
}

//Events
static void MouseClicked(EventData event){
	if(event.e->key.state == SDL_RELEASED){
		mouse_clicked = true;
	}
}
static void MouseUp(EventData event){
	if(event.e->key.state == SDL_RELEASED){
		mouse_lifted = true;
	}
}
static void Scroll(EventData event){
	if(event.e->wheel.y > 0){
		scroll_value = 1;
	}else if(event.e->wheel.y < 0){
		scroll_value = -1;
	}
}

void WindowResize(EventData event){
	if(event.e->window.event == SDL_WINDOWEVENT_RESIZED){
		SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
		DebugLog(D_ACT, "Window resolution set to %dx%d", SCREEN_WIDTH, SCREEN_HEIGHT);
	}
}

void KeyEvents_quick(EventData event){
	if(event.keyStates[SDL_SCANCODE_ESCAPE]){
		running = false;
	}
}