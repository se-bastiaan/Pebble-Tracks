// 2013 Sébastiaan Versteeg @se_bastiaan

#include <pebble.h>
#include "pebble-assist.h"
#include "errorwindow.h"
#include "tracklist.h"
#include "common.h"
        
#define MAX_ITEMS 10

static Window *window;
static TextLayer *text_layer;

void out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
}

void tap_handler(AccelAxisType axis, int32_t direction) {
    vibes_double_pulse();
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
            return;
    }

    Tuplet tuple = TupletInteger(REFRESH, 1);
    dict_write_tuplet(iter, &tuple);
   
    dict_write_end(iter);
    app_message_outbox_send();

    window_stack_pop_all(true);
    errorwindow_show();
    errorwindow_set_text("Refreshing!");
}

void enableRefresh() {
    accel_tap_service_subscribe(tap_handler);
}

void in_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *tuple_code = dict_find(iter, CODE);
    Tuple *tuple_time = dict_find(iter, TIME);
    Tuple *tuple_error = dict_find(iter, ERROR);

    if(tuple_time && !tuple_error) {
        if(!tracklist_is_on_top()) {
            window_stack_pop_all(true);
            tracklist_show();
        }

        enableRefresh();
        if (tracklist_is_on_top()) {
            tracklist_in_received_handler(iter);
        } else {
            app_message_outbox_send();
        }
    } else if(tuple_code && !tuple_error) {
        tracklist_in_received_handler(iter);        
    } else {
        if(!errorwindow_is_on_top()) {
            window_stack_pop_all(true);
            errorwindow_show();
        }
        errorwindow_in_received_handler(iter);       
    }
}

void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void init(void) {
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);

    const uint32_t inbound_size = 2048;
    const uint32_t outbound_size = 64;
    app_message_open(inbound_size, outbound_size);
    
    tracklist_init();
    errorwindow_init();
}

int main(void) {
    init();
    window = window_create();
    window_stack_push(window, true);

    Layer *window_layer = window_get_root_layer(window);

    GRect bounds = layer_get_frame(window_layer);

    text_layer = text_layer_create(GRect(0,25, bounds.size.w - 5, bounds.size.h - 5));
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text(text_layer, "Welcome to Tracks!");
    layer_add_child(window_layer, text_layer_get_layer(text_layer));

    app_event_loop();

	accel_tap_service_unsubscribe();
    tracklist_destroy();
    errorwindow_destroy();
    text_layer_destroy(text_layer);
    window_destroy(window);
}