#ifndef MASK_H_
#define MASK_H_

void InitMaskedRender();

void MaskedRender(TilesheetObject top_sheet, unsigned int top_index, TilesheetObject bottom_sheet, unsigned int bottom_index, unsigned int mask_index, SDL_Rect dest, int zpos, SDL_Color color);

#endif