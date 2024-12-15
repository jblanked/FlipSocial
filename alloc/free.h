#pragma once
#include <flip_social.h>
#include <callback/flip_social_callback.h>
void free_all();
bool alloc_text_input(uint32_t view_id);
void free_text_input();
void flip_social_free_explore_dialog();
void flip_social_free_friends_dialog();
void flip_social_free_messages_dialog();
void flip_social_free_compose_dialog();
void flip_social_free_feed_dialog();
bool about_widget_alloc(bool is_logged_in);
void free_about_widget(bool is_logged_in);
bool pre_saved_messages_alloc(void);
void free_pre_saved_messages(void);
void flip_social_free_friends(void);