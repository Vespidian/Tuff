#include "global.h"
#include "event.h"

#include "text_event.h"

bool text_input = false;
char text_buffer;
TextObject text_object;

void Backspace(EventData event);

void InitTextEvent(){
    BindEvent(EV_POLL_ACCURATE, SDL_KEYDOWN, Backspace);
	// SDL_StartTextInput();
}

void TextObjectClear(TextObject *t){
	t->buffer_pointer = 0;
	t->buffer[0] = 0;
}

void TextObjectWrite(TextObject *t, char c){
	// Make sure the buffer is not full
	if(t->buffer_pointer < 31){

		// Increment the buffer pointer
		t->buffer_pointer++;
	}else{

		// If it is full, shift out the oldest character
		for(int i = 0; i < 31; i++){
			t->buffer[i] = t->buffer[i + 1];
		}

		// And set the pointer to the last character in the buffer
		t->buffer_pointer = 31;
	}
	t->buffer[t->buffer_pointer] = c;
}

char TextObjectRead(TextObject *t){
	char tmp = t->buffer[0];

	// Shift out the character that was just read
	for(int i = 0; i < t->buffer_pointer; i++){
		t->buffer[i] = t->buffer[i + 1];
	}

	// Decrement the buffer pointer (but only to 0)
	if(t->buffer_pointer != 0){
		t->buffer_pointer--;
	}else{
		t->buffer[0] = 0;
	}

	return tmp;
}

void PollText(SDL_Event *event){
	while(SDL_PollEvent(event)){
		if(event->type == SDL_TEXTINPUT){
			TextObjectWrite(&text_object, event->text.text[0]);
		}
	}
}

void Backspace(EventData event){
    if(text_input){
        if(event.e->key.keysym.sym == SDLK_BACKSPACE){
			TextObjectWrite(&text_object, 8);
        }
    }
}