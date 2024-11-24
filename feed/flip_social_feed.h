#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H
#include "flip_social.h"
#include <callback/flip_social_callback.h>
#include <flip_storage/flip_social_storage.h>

bool flip_social_get_feed();
bool flip_social_load_feed_post(int post_id);
FlipSocialFeedMini *flip_social_parse_json_feed();
#endif
