#ifndef UI_H_
#define UI_H_

extern TilesheetObject ui_tilesheet;

void InitUI();

/**
 *  Returns the position of a rectangular element in a list of specified size and spacing
 */
SDL_Rect VerticalRectList(int num_items, int itemIndex, Vector2 size, Vector2 origin, int spacing);
Vector4_i VerticalRectList_vec(int num_items, int itemIndex, Vector2 size, Vector2 origin, int spacing);

/**
 *  Renders a background to a set of VerticalRectList() parameters
 */
void DrawVListBackground(int num_items, Vector2 size, Vector2 origin, int spacing, uint8_t opacity);


long IntegerLerp(long start, long end, uint16_t amount);

void LoopUI();

/**
 *  State variable which declares whether or not the mouse is hovering over a ui element
 */
extern bool ui_hovered;


extern bool ui_selected;


extern ShaderObject ui_shader;
void DrawUIElement();

#endif