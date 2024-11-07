#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H
#include "flip_social.h"
#include <callback/flip_social_callback.h>

bool flip_social_temp_feed();
FlipSocialFeed *flip_social_feed_alloc();
void flip_social_free_feed();
bool flip_social_get_feed();
bool flip_social_parse_json_feed();
#endif