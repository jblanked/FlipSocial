#pragma once
#include <flip_social.h>
#include <callback/callback.h>
#include <free/free.h>
FlipSocialApp *alloc_flip_social_app();
FlipSocialModel *alloc_friends_model();
FlipSocialModel2 *alloc_messages();
FlipSocialMessage *alloc_user_messages();
FlipSocialModel *alloc_explore();
void alloc_headers(void);
FlipSocialFeedMini *alloc_feed_info(void);
bool allow_messages_dialog(bool free_first);
bool alloc_feed_view();
char *alloc_format_message(const char *user_message);
bool alloc_text_input(uint32_t view_id);
bool alloc_about_widget(bool is_logged_in);
bool alloc_variable_item_list(uint32_t view_id);
bool alloc_submenu(uint32_t view_id);
bool alloc_flipper_http();