#ifndef __VIEWS_H__
#define __VIEWS_H__

#include <pebble.h>

// Indices into the views array.
#define VIEW_CALENDAR 0
    
typedef struct
{
    Window* window;
    WindowHandlers handlers;
} ViewRegistration;

extern ViewRegistration views[];

int views_create();
void views_destroy();
    
#endif