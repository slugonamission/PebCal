#ifndef __DEVINTERFACE_H__
#define __DEVINTERFACE_H__

#include <stdint.h>
    
// We currently only support a single transaction
#define DEV_STAT_OK 1
#define DEV_STAT_BUSY -1            // There's currently an in-flight transaction
#define DEV_STAT_SEND_FAILED -2     // Failed to send the request to the phone.
#define DEV_STAT_MESSAGE_DROPPED -3 // The system dropped the response message
#define DEV_STAT_NO_OUTBOX -4       // There was not a dictionary to write to (app_message_outbox_begin failed)
#define DEV_STAT_MESSAGE_FULL -5    // There was too much data to write.
#define DEV_STAT_OTHER_ERROR -100   // An unknown error.
    
// The communication between the watch and the phone need only be simple.
// We need two communication channels.
// 1) A way to get the used dates. This is simply to speed up the interface and allow us
// to download the full agenda in the background
// 2) A way to get a full agenda for a month
// These codes are also used by the phone for returned data
#define CMD_NONE 0
#define CMD_GET_DAYS_USED 1
#define CMD_GET_AGENDA 2
    
// The key value pair keys
#define KEY_COMMAND 1
#define KEY_DAYS_USED 2
#define KEY_YEAR 3
#define KEY_MONTH 4
#define KEY_DAY 5
    
typedef void (*DaysUsedValue)(int status, uint32_t daysUsed, unsigned int month, unsigned int year, void* context);

int devinterface_init();
int devinterface_destroy();
    
// This isn't great, but it's small!
// If a bit is set in daysUsed, that day is in use.
// Bit 0 = day 1, and so on until bit 30 = day 31.
// Return value is error code. 1 = success, negative is error.
int devinterface_get_days_used(int month, int year, DaysUsedValue daysUsed, void* context);
    
#endif