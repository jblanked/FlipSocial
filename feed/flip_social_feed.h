#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H
#include "flip_social.h"
#include <callback/flip_social_callback.h>
#include <flip_storage/flip_social_storage.h>

bool flip_social_get_feed(FlipperHTTP *fhttp, int series_index);
FlipSocialFeedMini *flip_social_parse_json_feed(FlipperHTTP *fhttp);
bool flip_social_load_feed_post(int post_id);
bool flip_social_load_initial_feed(FlipperHTTP *fhttp, int series_index);
#endif
