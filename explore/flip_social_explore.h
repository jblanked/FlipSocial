#ifndef FLIP_SOCIAL_EXPLORE_H
#define FLIP_SOCIAL_EXPLORE_H
#include "flip_social.h"
bool flip_social_get_explore(FlipperHTTP *fhttp);
bool flip_social_get_explore_2(FlipperHTTP *fhttp);
bool flip_social_parse_json_explore(FlipperHTTP *fhttp);
#endif