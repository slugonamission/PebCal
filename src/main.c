#include <pebble.h>
#include "views.h"
#include "devinterface.h"
    
int main()
{
    if(!views_create())
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create views");
        views_destroy(); 
        return 1;
    }
    
    if(!devinterface_init())
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to init dev interface");
        views_destroy(); // devinterface cleans itself up
        return 1;
    }
    
    window_stack_push(view_get(VIEW_CALENDAR), true);
    app_event_loop();
    
    devinterface_destroy();
    views_destroy();
    
    return 0;
}