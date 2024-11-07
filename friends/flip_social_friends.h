#ifndef FLIP_SOCIAL_FRIENDS_H
#define FLIP_SOCIAL_FRIENDS_H
#include "flip_social.h"
#include <callback/flip_social_callback.h>

FlipSocialModel *flip_social_friends_alloc();
void flip_social_free_friends();
bool flip_social_get_friends();
bool flip_social_update_friends();
bool flip_social_parse_json_friends();
#endif