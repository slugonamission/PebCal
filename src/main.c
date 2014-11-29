#include <pebble.h>
#include "views.h"
    
int main()
{
    if(!views_create())
    {
        views_destroy(); 
        return 1;
    }
    
    window_stack_push(views[VIEW_CALENDAR].window, true);
    app_event_loop();
    
    views_destroy();
    
    return 0;
}