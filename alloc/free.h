#pragma once
#include <flip_social.h>
#include <callback/callback.h>
void free_all(bool should_free_variable_item_list, bool should_free_submenu, void *context);
void free_text_input();
void free_explore_dialog();
void free_friends_dialog();
void free_messages_dialog();
void free_compose_dialog();
void free_feed_view();
void free_about_widget(bool is_logged_in);
void free_friends(void);
void flip_feed_info_free(void);
void free_variable_item_list(void);
void free_submenu(void);
