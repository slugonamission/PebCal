#include <pebble.h>
#include "views.h"
    
int main()
{
    if(!views_create())
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create views");
        views_destroy(); 
        return 1;
    }
    window_stack_push(view_get(VIEW_CALENDAR), true);
    app_event_loop();
    
    views_destroy();
    
    return 0;
}