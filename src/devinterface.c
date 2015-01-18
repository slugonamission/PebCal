#include <pebble.h>
#include "devinterface.h"

int transaction_active = CMD_NONE; // If (any) transaction is active
DaysUsedValue days_used_callback = NULL;
void* days_used_context = NULL;
    
static void devinterface_incoming_dropped(AppMessageResult reason, void* context);
static void devinterface_incoming_message(DictionaryIterator* iter, void* context);
static void devinterface_outgoing_failed(DictionaryIterator *iterator, AppMessageResult reason, void* context);
static void devinterface_outgoing_complete(DictionaryIterator* iterator, void* context);
static void send_days_used_result(int status, uint32_t daysused, unsigned int month, unsigned int year);
    
int devinterface_init()
{
    app_message_register_outbox_sent(devinterface_outgoing_complete);
    app_message_register_outbox_failed(devinterface_outgoing_failed);
    app_message_register_inbox_received(devinterface_incoming_message);
    app_message_register_inbox_dropped(devinterface_incoming_dropped);
    
    // TODO: Optimise this call...
    AppMessageResult rv = app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
    
    if(rv != APP_MSG_OK)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Could not open app message, aborting");
        devinterface_destroy();
        return 0;
    }
    
    return 1;
}

int devinterface_destroy()
{
    app_message_register_outbox_sent(NULL);
    app_message_register_outbox_failed(NULL);
    app_message_register_inbox_received(NULL);
    app_message_register_inbox_dropped(NULL);
    return 1;
}
    
void send_days_used_result(int status, uint32_t daysused, unsigned int month, unsigned int year)
{
    if(days_used_callback != NULL)
    {
        days_used_callback(status, daysused, month, year, days_used_context);
        days_used_callback = NULL;
        days_used_context = NULL;
    }
}

int devinterface_get_days_used(int month, int year, DaysUsedValue daysUsed, void* context)
{
    if(transaction_active)
        return DEV_STAT_BUSY;
    
    // Fill in the request!
    DictionaryIterator* message;
    AppMessageResult rv = app_message_outbox_begin(&message);
    
    if(rv != APP_MSG_OK)
    {
        if(rv == APP_MSG_BUSY)
            APP_LOG(APP_LOG_LEVEL_WARNING, "app_message_outbox_begin returned busy but no transaction in flight");
        return DEV_STAT_NO_OUTBOX;
    }
    
    // The actual message is pretty simple...
    DictionaryResult dr = dict_write_uint8(message, KEY_COMMAND, CMD_GET_DAYS_USED);
    if(dr != DICT_OK)
    {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Couldn't enqueue command using dict_write_uint8");
        return DEV_STAT_MESSAGE_FULL;
    }
    
    // Now set the flags and send!
    transaction_active = CMD_GET_DAYS_USED;
    days_used_callback = daysUsed;
    days_used_context = context;
    rv = app_message_outbox_send();
    
    if(rv != APP_MSG_OK)
    {
        if(rv == APP_MSG_BUSY)
            APP_LOG(APP_LOG_LEVEL_WARNING, "app_message_outbox_begin returned busy but no transaction in flight");
        transaction_active = CMD_NONE;
        return DEV_STAT_SEND_FAILED;
    }
    
    return DEV_STAT_OK;
}

void devinterface_incoming_dropped(AppMessageResult reason, void* context)
{
    APP_LOG(APP_LOG_LEVEL_WARNING, "Receiving message failed with result %d", reason);
    
    switch(transaction_active)
    {
    case CMD_GET_DAYS_USED:
        send_days_used_result(DEV_STAT_MESSAGE_DROPPED, 0, 0, 0);
        break;
    }
    
    transaction_active = CMD_NONE;
}

void devinterface_incoming_message(DictionaryIterator* iter, void* context)
{
    switch(transaction_active)
    {
    case CMD_GET_DAYS_USED:
        {
            Tuple* data = dict_find(iter, KEY_DAYS_USED);
            Tuple* year = dict_find(iter, KEY_YEAR);
            Tuple* month = dict_find(iter, KEY_MONTH);
            
            if(data == NULL || year == NULL || month == NULL)
            {
                APP_LOG(APP_LOG_LEVEL_WARNING, "Could not find KEY_DAYS_USED in a days used transaction");
                send_days_used_result(DEV_STAT_OTHER_ERROR, 0, 0, 0);
            }
            else if(data->length != 4 || data->type != TUPLE_UINT ||
                    year->length != 4 || year->type != TUPLE_UINT ||
                    month->length != 4 || month->type != TUPLE_UINT)
            {
                APP_LOG(APP_LOG_LEVEL_WARNING, "DAYS_USED transaction didn't return uint32_t.");
                send_days_used_result(DEV_STAT_OTHER_ERROR, 0, 0, 0);
            }
            else
            {
                uint32_t days_used = data->value[0].uint32;
                send_days_used_result(DEV_STAT_OK, days_used, month->value[0].uint32, year->value[0].uint32);
            }
        }
    }
    
    transaction_active = CMD_NONE;
}

void devinterface_outgoing_failed(DictionaryIterator *iterator, AppMessageResult reason, void* context)
{
    // Send failed, update the flags
    APP_LOG(APP_LOG_LEVEL_WARNING, "Sending message failed with result %d", reason);
    
    switch(transaction_active)
    {
    case CMD_GET_DAYS_USED:
        send_days_used_result(DEV_STAT_SEND_FAILED, 0, 0, 0);
        break;
    }
    
    transaction_active = CMD_NONE;
}

void devinterface_outgoing_complete(DictionaryIterator* iterator, void* context)
{
    // Message has been sent, wait for a response!
}