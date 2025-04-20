#ifndef FLIP_SOCIAL_EXPLORE_H
#define FLIP_SOCIAL_EXPLORE_H
#include "flip_social.h"
bool explore_fetch(FlipperHTTP *fhttp);
bool explore_fetch_2(FlipperHTTP *fhttp);
bool explore_parse_json(FlipperHTTP *fhttp);
#endif