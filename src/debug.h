#ifndef DEBUG_H_
#define DEBUG_H_

//Ordered by level of complexity (least to greatest)
enum DEBUG{D_ACT, D_WARN, D_ERR, D_SCRIPT_ERR, D_SCRIPT_ACT, D_VERBOSE_ACT, D_VERBOSE_WARN, D_VERBOSE_ERR};

//Each complexity level corresponds to the last debug type of that level in the 'DEBUG' enum
enum DEBUG_LEVEL{DL_BRIEF = 4, DL_VERBOSE = 7};

void InitDebug();
void DebugLog(int type, const char *format, ...);
void QuitDebug();

#endif