#include "flip_social_friends.h"

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the friends view
bool flip_social_get_friends(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/friends");
    // will return true unless the devboard is not connected
    char url[100];
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/friends.json");

    fhttp->save_received_data = true;
    auth_headers_alloc();
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/user/friends/%s/", app_instance->login_username_logged_in);
    if (!flipper_http_request(fhttp, GET, url, auth_headers, NULL))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for friends");
        fhttp->state = ISSUE;
        return false;
    }
    fhttp->state = RECEIVING;
    return true;
}

bool flip_social_update_friends()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (!app_instance->submenu)
    {
        FURI_LOG_E(TAG, "Friends submenu is NULL");
        return false;
    }
    if (!flip_social_friends)
    {
        FURI_LOG_E(TAG, "Friends model is NULL");
        return false;
    }
    // Add submenu items for the users
    submenu_reset(app_instance->submenu);
    submenu_set_header(app_instance->submenu, "Friends");
    for (int i = 0; i < flip_social_friends->count; i++)
    {
        submenu_add_item(app_instance->submenu, flip_social_friends->usernames[i], FlipSocialSubmenuLoggedInIndexFriendsStart + i, flip_social_callback_submenu_choices, app_instance);
    }
    return true;
}

bool flip_social_parse_json_friends(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    // load the received data from the saved file
    FuriString *friend_data = flipper_http_load_from_file(fhttp->file_path);
    if (friend_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    //  Allocate memory for each username only if not already allocated
    flip_social_friends = alloc_friends_model();
    if (flip_social_friends == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for friends usernames.");
        furi_string_free(friend_data);
        return false;
    }

    // Initialize friends count
    flip_social_friends->count = 0;

    // Reset the friends submenu
    submenu_reset(app_instance->submenu);
    submenu_set_header(app_instance->submenu, "Friends");

    // Extract the users array from the JSON
    for (int i = 0; i < MAX_FRIENDS; i++)
    {
        FuriString *friend = get_json_array_value_furi("friends", i, friend_data);
        if (friend == NULL)
        {
            FURI_LOG_E(TAG, "Failed to parse friend %d.", i);
            furi_string_free(friend_data);
            break;
        }
        snprintf(flip_social_friends->usernames[i], MAX_USER_LENGTH, "%s", furi_string_get_cstr(friend));
        submenu_add_item(app_instance->submenu, flip_social_friends->usernames[i], FlipSocialSubmenuLoggedInIndexFriendsStart + i, flip_social_callback_submenu_choices, app_instance);
        flip_social_friends->count++;
        furi_string_free(friend);
    }
    furi_string_free(friend_data);

    return true;
}
