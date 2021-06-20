#ifndef TEXT_EVENT_H_
#define TEXT_EVENT_H_


typedef struct TextObject{
	char buffer[32];
	unsigned int buffer_pointer;
}TextObject;

TextObject text_object;
void TextObjectClear(TextObject *t);
char TextObjectRead(TextObject *t);

extern bool text_input;
extern char text_buffer;

void InitTextEvent();
void PollText(SDL_Event *event);

#endif