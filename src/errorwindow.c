// 2013 Sébastiaan Versteeg

#include <pebble.h>
#include "pebble-assist.h"
#include "common.h"

static char error[128];

static Window *window;
static TextLayer *text_layer;

void errorwindow_init(void) {
    window = window_create();

	Layer *window_layer = window_get_root_layer(window);

	GRect bounds = layer_get_frame(window_layer);

	text_layer = text_layer_create(GRect(0,25, bounds.size.w - 5, bounds.size.h - 5));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text(text_layer, "");
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

void errorwindow_show() {
	window_stack_push(window, true);
}

void errorwindow_remove_stack() {
    window_stack_remove(window, true);
}

void errorwindow_destroy(void) {
	layer_remove_from_parent(text_layer_get_layer(text_layer));
	text_layer_destroy_safe(text_layer);
    window_destroy_safe(window);
}

bool errorwindow_is_on_top() {
    return window == window_stack_get_top_window();
}

void errorwindow_set_text(char *text) {
	text_layer_set_text(text_layer, text);
}

void errorwindow_in_received_handler(DictionaryIterator *iter) {
    Tuple *tuple_error = dict_find(iter, ERROR);
    text_layer_set_text(text_layer, tuple_error->value->cstring);
}