#include <explore/explore.h>
#include <alloc/alloc.h>

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the explore view
bool explore_fetch(FlipperHTTP *fhttp)
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

    // create the explore directory
    if (!flip_social_subfolder_mkdir("explore"))
    {
        FURI_LOG_E(TAG, "Failed to create explore directory");
        return false;
    }

    char *keyword = !app_instance->explore_logged_in || strlen(app_instance->explore_logged_in) == 0 ? "a" : app_instance->explore_logged_in;
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/explore/%s.json",
        keyword);

    fhttp->save_received_data = true;
    alloc_headers();
    char url[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/user/explore/%s/%d/", keyword, MAX_EXPLORE_USERS);
    if (!flipper_http_request(fhttp, GET, url, auth_headers, NULL))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for explore");
        fhttp->state = ISSUE;
        return false;
    }
    fhttp->state = RECEIVING;
    return true;
}
bool explore_fetch_2(FlipperHTTP *fhttp)
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
    // create the explore directory
    if (!flip_social_subfolder_mkdir("explore"))
    {
        FURI_LOG_E(TAG, "Failed to create explore directory");
        return false;
    }

    char *keyword = !app_instance->message_users_logged_in || strlen(app_instance->message_users_logged_in) == 0 ? "a" : app_instance->message_users_logged_in;
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/explore/%s.json",
        keyword);

    fhttp->save_received_data = true;
    alloc_headers();
    char url[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/user/explore/%s/%d/", keyword, MAX_EXPLORE_USERS);
    return flipper_http_request(fhttp, GET, url, auth_headers, NULL);
}

bool explore_parse_json(FlipperHTTP *fhttp)
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
    // load the received data from the saved file
    FuriString *user_data = flipper_http_load_from_file(fhttp->file_path);
    if (user_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    // Allocate memory for each username only if not already allocated
    flip_social_explore = alloc_explore();
    if (flip_social_explore == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore usernames.");
        furi_string_free(user_data);
        return false;
    }

    // Initialize explore count
    flip_social_explore->count = 0;

    // set submenu
    submenu_reset(app_instance->submenu);
    submenu_set_header(app_instance->submenu, "Explore");
    // Parse the JSON array of usernames
    for (size_t i = 0; i < MAX_EXPLORE_USERS; i++)
    {
        FuriString *username = get_json_array_value_furi("users", i, user_data); // currently its 53 tokens (with max explore users at 50)
        if (username == NULL)
        {
            break;
        }
        snprintf(flip_social_explore->usernames[i], MAX_USER_LENGTH, "%s", furi_string_get_cstr(username));
        submenu_add_item(app_instance->submenu, flip_social_explore->usernames[i], FlipSocialSubmenuExploreIndexStartIndex + i, callback_submenu_choices, app_instance);
        flip_social_explore->count++;
        furi_string_free(username);
    }
    furi_string_free(user_data);
    return flip_social_explore->count > 0;
}
