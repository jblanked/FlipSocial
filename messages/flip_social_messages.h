#ifndef FLIP_SOCIAL_MESSAGES_H
#define FLIP_SOCIAL_MESSAGES_H

#include "flip_social.h"
#include <callback/flip_social_callback.h>
#include <explore/flip_social_explore.h>

FlipSocialModel2 *flip_social_messages_alloc();
FlipSocialMessage *flip_social_user_messages_alloc();
void flip_social_free_message_users();
void flip_social_free_messages();
bool flip_social_update_messages_submenu();
bool flip_social_update_submenu_user_choices();
// Get all the users that have sent messages to the logged in user
bool flip_social_get_message_users(FlipperHTTP *fhttp);
// Get all the messages between the logged in user and the selected user
bool flip_social_get_messages_with_user(FlipperHTTP *fhttp);
// Parse the users that have sent messages to the logged-in user
bool flip_social_parse_json_message_users(FlipperHTTP *fhttp);

// Parse the users that the logged in user can message
bool flip_social_parse_json_message_user_choices(FlipperHTTP *fhttp);

// parse messages between the logged in user and the selected user
bool flip_social_parse_json_messages(FlipperHTTP *fhttp);

#endif