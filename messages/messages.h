#ifndef FLIP_SOCIAL_MESSAGES_H
#define FLIP_SOCIAL_MESSAGES_H

#include "flip_social.h"

bool messages_submenu_update();
bool messages_update_submenu_user_choices();
// Get all the users that have sent messages to the logged in user
bool messages_get_message_users(FlipperHTTP *fhttp);
// Get all the messages between the logged in user and the selected user
bool messages_get_messages_with_user(FlipperHTTP *fhttp);
// Parse the users that have sent messages to the logged-in user
bool messages_parse_json_message_users(FlipperHTTP *fhttp);

// Parse the users that the logged in user can message
bool messages_parse_json_message_user_choices(FlipperHTTP *fhttp);

// parse messages between the logged in user and the selected user
bool messages_parse_json_messages(FlipperHTTP *fhttp);

#endif