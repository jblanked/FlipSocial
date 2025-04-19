#pragma once
#include <flip_social.h>
#include <callback/callback.h>
#include <free/free.h>
FlipSocialApp *flip_social_app_alloc();
FlipSocialModel *alloc_friends_model();
FlipSocialModel2 *alloc_messages();
FlipSocialMessage *alloc_user_messages();
FlipSocialModel *alloc_explore();
void auth_headers_alloc(void);
FlipSocialFeedMini *flip_feed_info_alloc(void);
bool messages_dialog_alloc(bool free_first);
bool feed_view_alloc();
char *updated_user_message(const char *user_message);
bool alloc_text_input(uint32_t view_id);
bool about_widget_alloc(bool is_logged_in);
bool alloc_variable_item_list(uint32_t view_id);
bool alloc_submenu(uint32_t view_id);