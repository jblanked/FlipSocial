#ifndef FLIP_SOCIAL_FRIENDS_H
#define FLIP_SOCIAL_FRIENDS_H
#include "flip_social.h"
#include <callback/callback.h>

bool flip_social_get_friends(FlipperHTTP *fhttp);
bool flip_social_update_friends();
bool flip_social_parse_json_friends(FlipperHTTP *fhttp);
#endif