#ifndef __AGENDA_H__
#define __AGENDA_H__

#include <pebble.h>
    
void agenda_load(Window* window);
void agenda_unload(Window* window);
void agenda_appear(Window* window);
void agenda_disappear(Window* window);

#endif