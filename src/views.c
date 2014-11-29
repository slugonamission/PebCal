#include <pebble.h>
#include "views.h"

// Include all views
#include "calendar.h"
    
#define REG(name) { NULL, { .load = name ## _load, .unload = name ## _unload, .appear = NULL, .disappear = NULL } }
#define REG_APPEAR(name) { NULL, { .load = name ## _load, .unload = name ## _unload, .appear = name ## _appear, .disappear = name ## _disappear } }

ViewRegistration views[] = 
{
    REG(calendar),
    { NULL, { NULL, NULL, NULL, NULL } }
};

// Create all listed views. Should be called first.
// Returns 1 on success and 0 on failure.
// On failure, the app should call destroy_views() and
// exit.
int views_create()
{
    ViewRegistration* reg = &views[0];
    
    // An all-NULL element signifies the end.
    // Maybe a sentinel on the Window* would be better. *shrug*. This works.
    while(reg->handlers.load != NULL)
    {
        reg->window = window_create();
        if(reg->window == NULL)
            return 0;
        
        // Wait...this is passed by value?!
        window_set_window_handlers(reg->window, reg->handlers);
        reg++;
    }
    
    return 1;
}

// Destroy all created views.
void views_destroy()
{
    ViewRegistration* reg = &views[0];
    while(reg->handlers.load != NULL)
    {
        if(reg->window != NULL)
        {
            window_destroy(reg->window);
            reg->window = NULL;
        }
        reg++;
    }
}