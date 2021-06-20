#ifndef BUTTON_H_
#define BUTTON_H_

typedef void (*ButtonFunction)();

/**
 *  Renders a button with specified text
 *  \return Boolean specifying whether or not button is pressed
 */
bool Button(SDL_Rect rect, char *text);

/**
 *  Same as Button() but with a formatted text input
 */
bool Button_format(SDL_Rect rect, const char *text, ...);

/**
 *  Renders a button with specified text. If button is pressed 'function' is called
 */
void Button_function(SDL_Rect rect, char *text, ButtonFunction function);

#endif