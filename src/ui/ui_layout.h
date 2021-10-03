#ifndef UI_LAYOUT_H_
#define UI_LAYOUT_H_

void ApplyClass(UIElement *element, UIClass *class);
void RecursiveApplyStaticClasses(UIElement *element);
void RecursiveApplyElementClasses(UIElement *element);

#endif