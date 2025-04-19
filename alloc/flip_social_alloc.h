#ifndef FLIP_SOCIAL_ALLOC_H
#define FLIP_SOCIAL_ALLOC_H
#include <flip_social.h>
#include <callback/callback.h>
#include <flip_storage/flip_social_storage.h>

/**
 * @brief Function to allocate resources for the FlipSocialApp.
 * @details Initializes all components and views of the application.
 * @return Pointer to the initialized FlipSocialApp, or NULL on failure.
 */
FlipSocialApp *flip_social_app_alloc();
#endif