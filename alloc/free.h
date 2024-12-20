#pragma once
#include <flip_social.h>
#include <callback/flip_social_callback.h>
void free_all(bool should_free_variable_item_list, bool should_free_submenu);
void free_text_input();
void flip_social_free_explore_dialog();
void flip_social_free_friends_dialog();
void flip_social_free_messages_dialog();
void flip_social_free_compose_dialog();
void flip_social_free_feed_view();
void free_about_widget(bool is_logged_in);
void flip_social_free_friends(void);
void flip_feed_info_free(void);
void free_variable_item_list(void);
void free_submenu(void);
