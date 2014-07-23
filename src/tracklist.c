// 2013 Sébastiaan Versteeg

#include <pebble.h>
#include "pebble-assist.h"
#include "errorwindow.h"
#include "tracklist.h"
#include "common.h"

#define MAX_ITEMS 10
#define MENU_CELL_HEIGHT 50

static Departure items[MAX_ITEMS];
static Station current_station;

static int num_items;
static char error[128];

static void clean_list();
static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

static Window *window;
static MenuLayer *menu_layer;

void tracklist_init(void) {
	window = window_create();
	
	menu_layer = menu_layer_create_fullscreen(window);
	menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
		.get_num_sections = menu_get_num_sections_callback,
		.get_num_rows = menu_get_num_rows_callback,
		.get_header_height = menu_get_header_height_callback,
		.get_cell_height = menu_get_cell_height_callback,
		.draw_header = menu_draw_header_callback,
		.draw_row = menu_draw_row_callback
	});
	menu_layer_set_click_config_onto_window(menu_layer, window);
	menu_layer_add_to_window(menu_layer, window);
	
	num_items = 0;
}

void tracklist_show() {
	clean_list();
	window_stack_push(window, true);
}

void tracklist_remove_stack() {
	window_stack_remove(window, true);
}

void tracklist_destroy(void) {
	clean_list();
	layer_remove_from_parent(menu_layer_get_layer(menu_layer));
	menu_layer_destroy_safe(menu_layer);
	window_destroy_safe(window);
}

static void clean_list() {
	memset(items, 0x0, sizeof(items));
	num_items = 0;
	error[0] = '\0';
	menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
	menu_layer_reload_data_and_mark_dirty(menu_layer);
}

bool tracklist_is_on_top() {
	return window == window_stack_get_top_window();
}

void tracklist_in_received_handler(DictionaryIterator *iter) {
	Tuple *tuple_refresh = dict_find(iter, REFRESH);
	Tuple *tuple_code = dict_find(iter, CODE);
	Tuple *tuple_short_name = dict_find(iter, SHORT_NAME);
	Tuple *tuple_full_name = dict_find(iter, FULL_NAME);
	Tuple *tuple_time = dict_find(iter, TIME);
	Tuple *tuple_delay = dict_find(iter, DELAY_TEXT);
	Tuple *tuple_dest = dict_find(iter, DESTINATION);
	Tuple *tuple_type = dict_find(iter, TRAIN_TYPE);
	Tuple *tuple_route = dict_find(iter, ROUTE);
	Tuple *tuple_transp = dict_find(iter, TRANSPORTER);
	Tuple *tuple_track = dict_find(iter, TRACK);
	Tuple *tuple_track_changed = dict_find(iter, TRACK_CHANGED);
	Tuple *tuple_tip = dict_find(iter, TIP);
	
	if(tuple_refresh) {
		if(tuple_refresh->value->int16 == 1) {
			clean_list();
		}
	}
	
	if(tuple_time) {
		Departure item;
		if(num_items < 10) {
			item.index = num_items;
			strncpy(item.dep_time, tuple_time->value->cstring, sizeof(item.dep_time));
			strncpy(item.delay, tuple_delay->value->cstring, sizeof(item.delay));
			strncpy(item.destination, tuple_dest->value->cstring, sizeof(item.destination));
			strncpy(item.train_type, tuple_type->value->cstring, sizeof(item.train_type));
			strncpy(item.route, tuple_route->value->cstring, sizeof(item.route));
			strncpy(item.transporter, tuple_transp->value->cstring, sizeof(item.transporter));
			strncpy(item.track, tuple_track->value->cstring, sizeof(item.track));
			item.track_changed = (tuple_track_changed->value->int16 == 1 ? true : false);
			strncpy(item.tip, tuple_tip->value->cstring, sizeof(item.tip));
			items[item.index] = item;
			num_items++;
			menu_layer_reload_data_and_mark_dirty(menu_layer);
		}
		
	} else if(tuple_code) {
		strncpy(current_station.code, tuple_code->value->cstring, sizeof(current_station.code));
		strncpy(current_station.short_name, tuple_short_name->value->cstring, sizeof(current_station.short_name));
		strncpy(current_station.full_name, tuple_full_name->value->cstring, sizeof(current_station.full_name));
	}
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
	return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return (num_items) ? num_items : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	return MENU_CELL_HEIGHT;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
	menu_cell_basic_header_draw(ctx, cell_layer, current_station.full_name);        
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
	if (num_items == 0) {
		menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
	} else {
		
		char retVal[strlen(items[cell_index->row].dep_time)+strlen(items[cell_index->row].track)+5];
		if(items[cell_index->row].track_changed) {
			strcpy(retVal, "*");
			strcat(retVal, items[cell_index->row].track);
		} else {
			strcpy(retVal, items[cell_index->row].track);
		}                
		strcat(retVal, " - ");
		strcat(retVal, items[cell_index->row].dep_time);
		
		menu_cell_basic_draw(ctx, cell_layer, items[cell_index->row].destination, retVal, NULL);
	}
}