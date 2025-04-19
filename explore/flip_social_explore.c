#include "flip_social_explore.h"

FlipSocialModel *flip_social_explore_alloc(void)
{
    // Allocate memory for each username only if not already allocated
    FlipSocialModel *explore = malloc(sizeof(FlipSocialModel));
    if (explore == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore model.");
        return NULL;
    }
    return explore;
}

void free_explore(void)
{
    if (flip_social_explore)
    {
        free(flip_social_explore);
        flip_social_explore = NULL;
    }
}

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the explore view
bool flip_social_get_explore(FlipperHTTP *fhttp)
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
    char directory[128];
    snprintf(directory, sizeof(directory), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/explore");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory);
    char *keyword = !app_instance->explore_logged_in || strlen(app_instance->explore_logged_in) == 0 ? "a" : app_instance->explore_logged_in;
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/explore/%s.json",
        keyword);

    fhttp->save_received_data = true;
    auth_headers_alloc();
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
bool flip_social_get_explore_2(FlipperHTTP *fhttp)
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
    char directory[128];
    snprintf(directory, sizeof(directory), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/explore");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory);
    char *keyword = !app_instance->message_users_logged_in || strlen(app_instance->message_users_logged_in) == 0 ? "a" : app_instance->message_users_logged_in;
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/explore/%s.json",
        keyword);

    fhttp->save_received_data = true;
    auth_headers_alloc();
    char url[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/user/explore/%s/%d/", keyword, MAX_EXPLORE_USERS);
    return flipper_http_request(fhttp, GET, url, auth_headers, NULL);
}

bool flip_social_parse_json_explore(FlipperHTTP *fhttp)
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
    flip_social_explore = flip_social_explore_alloc();
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
        submenu_add_item(app_instance->submenu, flip_social_explore->usernames[i], FlipSocialSubmenuExploreIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
        flip_social_explore->count++;
        furi_string_free(username);
    }
    furi_string_free(user_data);
    return flip_social_explore->count > 0;
}
