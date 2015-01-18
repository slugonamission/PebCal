#include <pebble.h>
#include "agenda.h"

typedef struct {
    SimpleMenuLayer* menu;
    
    // Log the menu sections for cleanup
    SimpleMenuSection* sections;
    int numSections;
} AgendaData;
    
void agenda_load(Window* window)
{
    // Build a dummy menu for pissing around...
    Layer* rootLayer = window_get_root_layer(window);
    AgendaData* data = malloc(sizeof(AgendaData));
    memset(data, 0, sizeof(AgendaData));
    window_set_user_data(window, data);
    
    SimpleMenuItem* item = malloc(sizeof(SimpleMenuItem));
    item->icon = NULL;
    item->callback = NULL;
    item->title = "Hello World!";
    item->subtitle = "This is a test...";
    
    SimpleMenuSection* sect = malloc(sizeof(SimpleMenuSection));
    sect->title = "Test section";
    sect->items = item;
    sect->num_items = 1;
    
    data->sections = sect;
    data->numSections = 1;
    
    SimpleMenuLayer* menu = simple_menu_layer_create(layer_get_bounds(rootLayer), window, sect, 1, NULL);
    data->menu = menu;
    
    Layer* menuLayer = simple_menu_layer_get_layer(menu);
    layer_add_child(rootLayer, menuLayer);
}

void agenda_unload(Window* window)
{
    // Get the data...
    AgendaData* data = window_get_user_data(window);
    
    // Delete the menu
    if(data->menu)
        simple_menu_layer_destroy(data->menu);
    
    // Delete the sections...
    if(data->sections && data->numSections)
    {
        int i = 0;
        for(i = 0; i < data->numSections; i++)
        {
            unsigned int j = 0;
            for(j = 0; j < data->sections[i].num_items; j++)
            {
                SimpleMenuItem* item = (SimpleMenuItem*)&data->sections[i].items[j];
                free(item);
            }
            free(&data->sections[i]);
        }
    }
    
    free(data);
}

void agenda_appear(Window* window)
{
    
}

void agenda_disappear(Window* window)
{
    
}
