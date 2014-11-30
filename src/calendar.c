#include <pebble.h>
#include "calendar.h"
    
#define CALENDAR_FOREGROUND GColorWhite
#define CALENDAR_BACKGROUND GColorBlack
    
#define MONTHPANE_HEIGHT 18
#define MONTHPANE_Y_OFFSET 0
#define MONTHPANE_Y_MARGIN 10

#define CALENDARPANE_TOP (MONTHPANE_Y_OFFSET + MONTHPANE_HEIGHT + MONTHPANE_Y_MARGIN)
#define CALENDARPANE_X_MARGIN 5
#define CALENDARPANE_Y_MARGIN 5 // Space at the bottom
#define CALENDARPANE_NUMBER_X_PAD 2
    
typedef struct
{
    Layer* graphicsLayer;
} RootData;

// Root graphics context drawing routine
void calendar_graphics_draw(Layer* layer, GContext* context);
    
// Draw a single calendar
void calendar_single_draw(Layer* layer, GContext* context, int16_t offset);

void calendar_load(Window* wnd)
{
    // Set the background first
    window_set_background_color(wnd, CALENDAR_BACKGROUND);
    
    // Create a graphics context
    Layer* windowLayer = window_get_root_layer(wnd);
    
    // Get the frame of the parent layer
    APP_LOG(APP_LOG_LEVEL_INFO, "Getting frame");
    GRect calendarFrame = layer_get_frame(windowLayer);
    GRect calendarBounds = calendarFrame;
    
    // Get the height and triple it
    // This allows us to have a region at the top and bottom
    // for next/previous months.
    // Then set the frame to be dead in the middle of it.
    calendarBounds.origin.y = -calendarFrame.size.h;
    calendarBounds.size.h *= 3;
    
    Layer* graphicsLayer = layer_create(calendarFrame);
    layer_set_bounds(graphicsLayer, calendarBounds);
    
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

void calendar_graphics_draw(Layer* layer, GContext* context)
{
    // Set the colour
    graphics_context_set_text_color(context, CALENDAR_FOREGROUND);
    graphics_context_set_fill_color(context, CALENDAR_FOREGROUND);
    graphics_context_set_stroke_color(context, CALENDAR_FOREGROUND);
    
    // Get the frame to figure out the offset
    GRect frame = layer_get_frame(layer);
    calendar_single_draw(layer, context, frame.size.h);
}

void calendar_single_draw(Layer* layer, GContext* context, int16_t offset)
{
    // Get the current date
    time_t t = time(NULL);
    struct tm* localTime = localtime(&t);
    char month[20];
    strftime(month, 20, "%B %Y", localTime);
    
    // Create the bounding box for the period
    GRect contextBound = layer_get_frame(layer);
    GRect textBounding = contextBound;
    textBounding.origin.y += offset + MONTHPANE_Y_OFFSET;
    textBounding.size.h = MONTHPANE_HEIGHT;
    
    // Put up the current period
    graphics_draw_text(context, month, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), textBounding, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    
    // Compute the height of the calendar box
    int calHeight = contextBound.size.h - CALENDARPANE_TOP - CALENDARPANE_Y_MARGIN;
    int calWidth = contextBound.size.w - (CALENDARPANE_X_MARGIN*2);
    
    // Draw the calendar box
    // Top line
    graphics_draw_line(context, 
                       (GPoint){CALENDARPANE_X_MARGIN, CALENDARPANE_TOP + offset}, 
                       (GPoint){CALENDARPANE_X_MARGIN + calWidth, CALENDARPANE_TOP + offset});
    
    // Bottom line
    graphics_draw_line(context, 
                       (GPoint){CALENDARPANE_X_MARGIN, CALENDARPANE_TOP + calHeight + offset}, 
                       (GPoint){CALENDARPANE_X_MARGIN + calWidth, CALENDARPANE_TOP + calHeight + offset});
    
    // Left line
    graphics_draw_line(context,
                      (GPoint){CALENDARPANE_X_MARGIN, CALENDARPANE_TOP + offset},
                      (GPoint){CALENDARPANE_X_MARGIN, CALENDARPANE_TOP + offset + calHeight});
    
    // Right line
    graphics_draw_line(context,
                      (GPoint){CALENDARPANE_X_MARGIN + calWidth, CALENDARPANE_TOP + offset},
                      (GPoint){CALENDARPANE_X_MARGIN + calWidth, CALENDARPANE_TOP + offset + calHeight});
    
    // And now the vertical subdivisions. There needs to be 7.
    int vertSubDiv = calWidth / 7;
    for(int i = 1; i < 7; i++)
    {
        graphics_draw_line(context,
                          (GPoint){CALENDARPANE_X_MARGIN + vertSubDiv*i, CALENDARPANE_TOP + offset},
                          (GPoint){CALENDARPANE_X_MARGIN + vertSubDiv*i, CALENDARPANE_TOP + offset + calHeight});
    }
    
    // And now the horizontal subdivisions. There needs to be 5
    int horizSubDiv = calHeight / 5;
    for(int i = 1; i < 5; i++)
    {
        graphics_draw_line(context,
                           (GPoint){CALENDARPANE_X_MARGIN, CALENDARPANE_TOP + offset + horizSubDiv*i},
                           (GPoint){CALENDARPANE_X_MARGIN + calWidth, CALENDARPANE_TOP + offset + horizSubDiv*i});
    }
    
    // Now draw the days
    for(int day = 1; day <= 31; day++)
    {
        int x = (day-1) % 7;
        int y = (day-1) / 7;
        char dayStr[3];
        snprintf(dayStr, 3, "%d", day);
        
        GRect dayRect;
        dayRect.origin.x = (x * vertSubDiv) + CALENDARPANE_X_MARGIN + CALENDARPANE_NUMBER_X_PAD;
        dayRect.origin.y = (y * horizSubDiv) + CALENDARPANE_TOP + offset;
        dayRect.size.h = horizSubDiv;
        dayRect.size.w = vertSubDiv;
        
        graphics_draw_text(context, dayStr, fonts_get_system_font(FONT_KEY_GOTHIC_14), dayRect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
}