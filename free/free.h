#pragma once
#include <flip_social.h>
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
void free_message_users();
void free_messages();
void free_explore();
void free_flipper_http();
void free_flip_social_app(FlipSocialApp *app);