#pragma once

void errorwindow_init(void);
void errorwindow_show();
void errorwindow_remove_stack();
void errorwindow_destroy(void);
void errorwindow_in_received_handler(DictionaryIterator *iter);
bool errorwindow_is_on_top();
void errorwindow_refresh();
void errorwindow_set_text(char *text);