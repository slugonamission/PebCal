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
    
// Which is the start day of the week?
// This offset is added to dow below to shift
// it slightly.
#define WEEK_START_SUN 0
#define WEEK_START_MON 1
#define WEEK_START_OFFSET WEEK_START_MON
    
typedef struct
{
    Layer* graphicsLayer;
    time_t curTime;
} RootData;


void calendar_graphics_draw(Layer* layer, GContext* context); // Root graphics context drawing routine
void calendar_single_draw(Layer* layer, GContext* context, int16_t offset); // Draw a single calendar
void calendar_click_config_provider(void* context);
int days_in_month(int monthNo, int yearNo);  // Monthno should be 1-12

// Click handlers
// Merge them all for single, since it's quite simple
void calendar_click_single(ClickRecognizerRef recogniser, void* context);

// -----------------------------------------------------------------------

int days_in_month(int monthNo, int yearNo)
{
    // Borrowed from http://www.codecodex.com/wiki/Calculate_the_number_of_days_in_a_month
    if(monthNo < 1 || monthNo > 12)
        return -1;
    
    if(monthNo == 4 || monthNo == 6 || monthNo == 9 || monthNo == 11)
        return 30;
    if(monthNo == 2)
    {
        bool isLeapYear = (yearNo % 4 == 0 && yearNo % 100 != 0) || (yearNo % 400 == 0);
        return isLeapYear ? 29 : 28;
    }
    return 31;
}

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
    data->curTime = time(NULL);
    window_set_user_data(wnd, data);
    
    // Set the update callback
    layer_set_update_proc(graphicsLayer, calendar_graphics_draw);
    
    layer_add_child(windowLayer, graphicsLayer);
    
    window_set_click_config_provider(wnd, calendar_click_config_provider);
}

void calendar_unload(Window* wnd)
{
    RootData* data = window_get_user_data(wnd);
    if(data->graphicsLayer)
        layer_destroy(data->graphicsLayer);
    free(data);
}

void calendar_click_config_provider(void* context)
{
    window_single_click_subscribe(BUTTON_ID_UP, calendar_click_single);
    window_single_click_subscribe(BUTTON_ID_DOWN, calendar_click_single);
    window_single_click_subscribe(BUTTON_ID_SELECT, calendar_click_single);
}

// Context in this case will be the window pointer.
void calendar_click_single(ClickRecognizerRef recogniser, void* context)
{
    ButtonId button = click_recognizer_get_button_id(recogniser);
    Window* wnd = (Window*)context;
    RootData* data = window_get_user_data(wnd);
    
    switch(button)
    {
    case BUTTON_ID_UP:
        data->curTime -= 7 * 86400;
        break;
    case BUTTON_ID_DOWN:
        data->curTime += 7 * 86400;
        break;
    case BUTTON_ID_SELECT:
        data->curTime += 86400;
        break;
    default:
        return; // Don't need to mark as dirty.
    }
    
    layer_mark_dirty(data->graphicsLayer);
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
    RootData* data = window_get_user_data(layer_get_window(layer));
    time_t t = data->curTime;
    struct tm* localTime = localtime(&t);
    char month[20];
    strftime(month, 20, "%B %Y", localTime);
    
    // Get the first day of the month (Mon-Sun)
    // Get the current day and day of week
    int curDay = localTime->tm_mday;
    int dow = localTime->tm_wday;
    
    // What's the offset into dow?
    // Ignoring weeks, this will give us the number of days
    // since the first of the month
    int curDowOffset = (curDay - 1 + WEEK_START_OFFSET) % 7;
    
    // We then subtract that from the current day of the week
    // which will yield the dow of the first day of the month.
    dow -= curDowOffset;
    if(dow < 0)
        dow += 7;
    
    // Get the number of days to draw
    int days_month = days_in_month(localTime->tm_mon + 1, localTime->tm_year + 1900);
    
    // How many weeks do we need to show?
    // How many days are there, including the leadin?
    int totalSlots = (days_month + dow);
    int lines = totalSlots / 7;
    
    // I'm not resorting to the FPU. Check if it had a fractional part
    if(lines * 7 != totalSlots)
        lines++;
    
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
    
    // And now the horizontal subdivisions.
    int horizSubDiv = calHeight / lines;
    for(int i = 1; i < lines; i++)
    {
        graphics_draw_line(context,
                           (GPoint){CALENDARPANE_X_MARGIN, CALENDARPANE_TOP + offset + horizSubDiv*i},
                           (GPoint){CALENDARPANE_X_MARGIN + calWidth, CALENDARPANE_TOP + offset + horizSubDiv*i});
    }
    
    // Now draw the days
    for(int day = 1; day <= days_month; day++)
    {
        int x = (day-1+dow) % 7;
        int y = (day-1+dow) / 7;
        char dayStr[3];
        snprintf(dayStr, 3, "%d", day);
        
        // Is this the current day?
        if(day == localTime->tm_mday)
        {
            // Paint it white
            GRect dayRectFill;
            dayRectFill.origin.x = (x * vertSubDiv) + CALENDARPANE_X_MARGIN;
            dayRectFill.origin.y = (y * horizSubDiv) + CALENDARPANE_TOP + offset;
            dayRectFill.size.h = horizSubDiv;
            dayRectFill.size.w = vertSubDiv;
            graphics_context_set_fill_color(context, GColorWhite);
            graphics_context_set_text_color(context, GColorBlack);
            graphics_fill_rect(context, dayRectFill, 0, GCornerNone);
        }
        
        GRect dayRect;
        dayRect.origin.x = (x * vertSubDiv) + CALENDARPANE_X_MARGIN + CALENDARPANE_NUMBER_X_PAD;
        dayRect.origin.y = (y * horizSubDiv) + CALENDARPANE_TOP + offset;
        dayRect.size.h = horizSubDiv;
        dayRect.size.w = vertSubDiv;
        
        graphics_draw_text(context, dayStr, fonts_get_system_font(FONT_KEY_GOTHIC_14), dayRect, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        
        if(day == localTime->tm_mday)
        {
            graphics_context_set_fill_color(context, GColorBlack);
            graphics_context_set_text_color(context, GColorWhite);
        }
    }
}