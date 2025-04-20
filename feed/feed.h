#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H
#include "flip_social.h"

bool feed_fetch(FlipperHTTP *fhttp, int series_index);
FlipSocialFeedMini *feed_parse_json(FlipperHTTP *fhttp);
bool feed_load_post(int post_id);
bool feed_load_initial_feed(FlipperHTTP *fhttp, int series_index);
#endif
