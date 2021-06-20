#ifndef UTILITY_H_
#define UTILITY_H_

/**
 *  Returns a rect representing the window size
**/
SDL_Rect *GetWindowRect(SDL_Window *window);

/**
 *  Sets dst to a ':' separated string representing the contents of a directory
 * \return Length of string
**/
int ListDirectory(const char *path, char *dst);

/**
 *  Retrieve the length of a filestream
**/
int GetLineLength(FILE *file);

/**
 *  Shift a string left starting at position 'startIndex' shifting by 'shiftBy'
**/
int strshft_l(char *stringShift, int startIndex, int shiftBy);

/**
 *  Compare 2 Vector2_i variables, returns true if they are equal, false otherwise
**/
bool CompareVector2(Vector2_i v1, Vector2_i v2);

int Rand(int min, int max);

int WhiteNoise(int x, int y);

#endif