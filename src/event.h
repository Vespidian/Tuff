#ifndef EVENT_H_
#define EVENT_H_

#include <SDL2/SDL.h>

typedef enum EventPollType_e{EV_POLL_QUICK = 0, EV_POLL_ACCURATE = 1}EventPollType_et;

typedef struct EventData{
	SDL_Event *e;
	const uint8_t *keyStates;
	const uint32_t *mouseState;
}EventData;

typedef void (*EV_Function)(EventData event);
typedef struct InputEvent{
	EventPollType_et pollType;
	Uint32 eventType;
	EV_Function function;
	bool isKeyPress;
	char keyCode;
	uint8_t scanCode;
}InputEvent;

void BindEvent(EventPollType_et pollType, uint32_t eventType, EV_Function function);
void BindQuickKeyEvent(EV_Function function, uint8_t scanCode);
void BindKeyEvent(EV_Function function, char key, uint32_t keyPressType);

extern SDL_Event e;
extern iVector2 mouse_pos;
extern iVector2 mouse_pos_previous;
extern bool enable_input;
extern bool mouse_held;
extern bool mouse_clicked;
extern bool mouse_released;
extern int scroll_value;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

extern SDL_Window *window;

void InitEvents();
void QuitEvents();
void FastEvents();
void EventListener();

#endif