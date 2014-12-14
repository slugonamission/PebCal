#include <pebble.h>
#include "views.h"

// Include all views
#include "calendar.h"
#include "agenda.h"
    
#define REG(name) { NULL, { .load = name ## _load, .unload = name ## _unload, .appear = NULL, .disappear = NULL } }
#define REG_APPEAR(name) { NULL, { .load = name ## _load, .unload = name ## _unload, .appear = name ## _appear, .disappear = name ## _disappear } }
#define REG_NULL { NULL, { NULL, NULL, NULL, NULL } }

ViewRegistration myapp_views[] = 
{
    REG(calendar),
    REG_APPEAR(agenda),
    REG_NULL
};

// Create all listed views. Should be called first.
// Returns 1 on success and 0 on failure.
// On failure, the app should call destroy_views() and
// exit.
int views_create()
{
    unsigned int* foo = (unsigned int*)&myapp_views[0];
    APP_LOG(APP_LOG_LEVEL_INFO, "Views: 0x%x", (unsigned int)foo);
    ViewRegistration* reg = myapp_views;
    APP_LOG(APP_LOG_LEVEL_INFO, "First element: 0x%x", (unsigned int)reg);
    // An all-NULL element signifies the end.
    // Maybe a sentinel on the Window* would be better. *shrug*. This works.
    while(reg->handlers.load != NULL)
    {
        APP_LOG(APP_LOG_LEVEL_INFO, "Creating View");
        reg->window = window_create();
        if(reg->window == NULL)
            return 0;
        
        APP_LOG(APP_LOG_LEVEL_INFO, "Got view 0x%x", (unsigned int)reg->window);
        
        // Wait...this is passed by value?!
        window_set_window_handlers(reg->window, reg->handlers);
        reg++;
        APP_LOG(APP_LOG_LEVEL_INFO, "LOL");
    }
    
    APP_LOG(APP_LOG_LEVEL_INFO, "Done");
    
    foo = (unsigned int*)&myapp_views[0];
    APP_LOG(APP_LOG_LEVEL_INFO, "Views: 0x%x", (unsigned int)foo);
    
    return 1;
}

// Destroy all created views.
void views_destroy()
{
    ViewRegistration* reg = &myapp_views[0];
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

// To work around a compiler bug
Window* view_get(int index)
{
    return myapp_views[index].window;
}