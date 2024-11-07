#ifndef FLIP_SOCIAL_EXPLORE_H
#define FLIP_SOCIAL_EXPLORE_H
#include "flip_social.h"
#include <callback/flip_social_callback.h>
FlipSocialModel *flip_social_explore_alloc();
void flip_social_free_explore();
bool flip_social_get_explore();
bool flip_social_parse_json_explore();
#endif