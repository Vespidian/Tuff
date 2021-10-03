#ifndef EVENT_H_
#define EVENT_H_

typedef enum EventPollType_e{EV_POLL_QUICK = 0, EV_POLL_ACCURATE = 1}EventPollType_et;

typedef struct EventData{
	SDL_Event *e;
	const Uint8 *keyStates;
	const Uint32 *mouseState;
}EventData;

typedef void (*EV_Function)(EventData event);
typedef struct InputEvent{
	EventPollType_et pollType;
	Uint32 eventType;
	EV_Function function;
	bool isKeyPress;
	char keyCode;
	Uint8 scanCode;
}InputEvent;

void BindEvent(EventPollType_et pollType, Uint32 eventType, EV_Function function);
void BindQuickKeyEvent(EV_Function function, Uint8 scanCode);
void BindKeyEvent(EV_Function function, char key, Uint32 keyPressType);

extern SDL_Event e;
extern Vector2_i mouse_pos;
extern Vector2_i mouse_pos_previous;
extern bool enable_input;
extern bool mouse_held;
extern bool mouse_clicked;
extern bool mouse_lifted;
extern int scroll_value;

void InitEvents();
void FastEvents();
void EventListener();

#endif