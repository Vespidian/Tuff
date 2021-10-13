#ifndef UI_UTILITY_H_
#define UI_UTILITY_H_

void SetElementText(UIElement *element, char *format, ...);
UIClass *FindClass(UIScene *scene, char *name);
UIElement *FindElement(UIScene *scene, char *name);

#endif