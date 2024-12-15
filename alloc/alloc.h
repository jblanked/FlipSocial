#pragma once
#include <flip_social.h>
#include <callback/flip_social_callback.h>
#include <alloc/free.h>
void auth_headers_alloc(void);
FlipSocialFeedMini *flip_feed_info_alloc(void);
bool messages_dialog_alloc(bool free_first);
bool feed_dialog_alloc();
bool alloc_text_input(uint32_t view_id);
bool about_widget_alloc(bool is_logged_in);
bool pre_saved_messages_alloc(void);
bool alloc_variable_item_list(uint32_t view_id);