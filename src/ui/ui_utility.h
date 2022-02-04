#ifndef UI_UTILITY_H_
#define UI_UTILITY_H_

void SetElementText(UIElement *element, char *format, ...);
UIClass *FindClass(UIDomain *domain, char *name);
UIElement *FindElement(UIDomain *domain, char *name);

#endif