#include <pebble.h>
#include "calendar.h"

#define BOX_MIN 10
#define BOX_MAX 130
#define CURVE_STEP 10
    
typedef struct
{
    Layer* graphicsLayer;
} RootData;

void calendar_graphics_draw(Layer* layer, GContext* context);
void calendar_graphics_tick(void* data);
    
void calendar_load(Window* wnd)
{
    // Create a graphics context
    Layer* windowLayer = window_get_root_layer(wnd);
    Layer* graphicsLayer = layer_create(layer_get_frame(windowLayer));
    
    RootData* data = malloc(sizeof(RootData));
    if(data == NULL)
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to allocate storage for data");
    data->graphicsLayer = graphicsLayer;
    window_set_user_data(wnd, data);
    
    // Set the update callback
    layer_set_update_proc(graphicsLayer, calendar_graphics_draw);
    
    layer_add_child(windowLayer, graphicsLayer);
}

void calendar_unload(Window* wnd)
{
    RootData* data = window_get_user_data(wnd);
    if(data->graphicsLayer)
        layer_destroy(data->graphicsLayer);
    free(data);
}

void calendar_graphics_tick(void* data)
{
    layer_mark_dirty((Layer*)data);
}

void calendar_graphics_draw(Layer* layer, GContext* context)
{
    int i = 0;
    static int iterCount = 12;
    static int currentIteration = 0;
    
    graphics_context_set_stroke_color(context, GColorBlack);
    
    // Draw a big box :)
    graphics_draw_line(context, (GPoint){BOX_MIN, BOX_MIN}, (GPoint){BOX_MIN, BOX_MAX});
    graphics_draw_line(context, (GPoint){BOX_MIN, BOX_MIN}, (GPoint){BOX_MAX, BOX_MIN});
    graphics_draw_line(context, (GPoint){BOX_MAX, BOX_MIN}, (GPoint){BOX_MAX, BOX_MAX});
    graphics_draw_line(context, (GPoint){BOX_MIN, BOX_MAX}, (GPoint){BOX_MAX, BOX_MAX});
    
    // Now draw the "curve"
    for(i = BOX_MIN; i < BOX_MIN + (CURVE_STEP * currentIteration); i += CURVE_STEP)
    {
        graphics_draw_line(context, (GPoint){i, BOX_MIN}, (GPoint){BOX_MAX, i + BOX_MIN}); // Top -> right
        graphics_draw_line(context, (GPoint){BOX_MIN, i}, (GPoint){i + BOX_MIN, BOX_MAX}); // Left -> bottom
        graphics_draw_line(context, (GPoint){i, BOX_MAX}, (GPoint){BOX_MAX, BOX_MAX - i}); // Bottom -> right
        graphics_draw_line(context, (GPoint){BOX_MIN, BOX_MAX - i}, (GPoint){i + BOX_MIN, BOX_MIN}); // Left -> Top
    }
    
    if(currentIteration++ < iterCount)
        app_timer_register(100, calendar_graphics_tick, (void*)layer);
}