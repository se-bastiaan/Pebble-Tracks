// 2013 Sébastiaan Versteeg

#pragma once

void tracklist_init(void);
void tracklist_show();
void tracklist_remove_stack();
void tracklist_destroy(void);
void tracklist_in_received_handler(DictionaryIterator *iter);
bool tracklist_is_on_top();
void tracklist_refresh();