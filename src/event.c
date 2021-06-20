#include "global.h"
#include "debug.h"
#include "renderer/renderer.h"
#include "text_event.h"

#include "event.h"

SDL_Event e;
SDL_Point mouse_pos = {0, 0};

InputEvent *events;
int num_events = 0;

bool enable_input = true;

bool mouse_held = false;
bool mouse_clicked = false;
Vector2 mouse = {0, 0};

//Event predefinitions
static void MouseClicked();
void KeyEvents_quick();
void WindowResize();

//Event management
void InitEvents(){
	events = malloc(sizeof(InputEvent));

	#ifdef DEBUG_BUILD
		BindKeyEvent(Quit, 0x1B, SDL_KEYDOWN);//escape
	#endif
	BindEvent(EV_ACCURATE, SDL_MOUSEBUTTONDOWN, MouseClicked);
	BindEvent(EV_ACCURATE, SDL_WINDOWEVENT, WindowResize);
	BindEvent(EV_ACCURATE, SDL_QUIT, Quit);

	InitTextEvent();
	DebugLog(D_ACT, "Initialized event subsystem");
}

void BindEvent(int pollType, Uint32 eventType, EV_Function function){
	events = realloc(events, sizeof(InputEvent) * (num_events + 1));
	events[num_events] = (InputEvent){pollType, eventType, function, false, 0x00};
	num_events++;
}

void PollEvents(){
	const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
	const Uint32 mouseState = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
	if(mouseState){
		mouse_held = true;
	}
	while(SDL_PollEvent(&e)){
		if(text_input){
			PollText(&e);
		}
		for(int i = 0; i < num_events; i++){
			if(events[i].pollType == EV_ACCURATE){
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
		if(events[i].pollType == EV_QUICK){
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

	if(enable_input){
		PollEvents();
	}
}

void BindQuickKeyEvent(EV_Function function, Uint8 scanCode){
	BindEvent(EV_QUICK, SDL_KEYDOWN, function);
	events[num_events - 1] = (InputEvent){EV_QUICK, SDL_KEYDOWN, function, true, 0x00, scanCode};
}

void BindKeyEvent(EV_Function function, char keyCode, Uint32 keyPressType){
	//Make sure keypresstype is keydown or keyup
	if(keyPressType != SDL_KEYDOWN && keyPressType != SDL_KEYUP){
		keyPressType = SDL_KEYDOWN;
	}
	BindEvent(EV_ACCURATE, keyPressType, function);
	events[num_events - 1] = (InputEvent){EV_ACCURATE, keyPressType, function, true, keyCode};
}

//Events
static void MouseClicked(EventData event){
	if(event.e->key.state == SDL_RELEASED){
		mouse_clicked = true;
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