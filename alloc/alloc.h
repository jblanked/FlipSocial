#pragma once
#include <flip_social.h>
#include <callback/flip_social_callback.h>
#include <alloc/free.h>
void auth_headers_alloc(void);
FlipSocialFeedMini *flip_feed_info_alloc(void);
bool messages_dialog_alloc(bool free_first);
bool feed_dialog_alloc();