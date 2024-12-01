#ifndef FLIP_SOCIAL_STORAGE_H
#define FLIP_SOCIAL_STORAGE_H

#include "flip_social.h"

// Function to save the playlist
void save_playlist(const PreSavedPlaylist *playlist);

// Function to load the playlist
bool load_playlist(PreSavedPlaylist *playlist);

void save_settings(
    const char *ssid,
    const char *password,
    const char *login_username_logged_out,
    const char *login_username_logged_in,
    const char *login_password_logged_out,
    const char *change_password_logged_in,
    const char *is_logged_in);

bool load_settings(
    char *ssid,
    size_t ssid_size,
    char *password,
    size_t password_size,
    char *login_username_logged_out,
    size_t username_out_size,
    char *login_username_logged_in,
    size_t username_in_size,
    char *login_password_logged_out,
    size_t password_out_size,
    char *change_password_logged_in,
    size_t change_password_size,
    char *is_logged_in,
    size_t is_logged_in_size);

bool flip_social_save_post(char *post_id, char *json_feed_data);
#endif