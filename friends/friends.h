#ifndef FLIP_SOCIAL_FRIENDS_H
#define FLIP_SOCIAL_FRIENDS_H
#include "flip_social.h"
bool friends_fetch(FlipperHTTP *fhttp);
bool friends_update();
bool friends_parse_json(FlipperHTTP *fhttp);
#endif