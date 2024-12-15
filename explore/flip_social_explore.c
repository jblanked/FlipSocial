#include "flip_social_explore.h"

FlipSocialModel *flip_social_explore_alloc(void)
{
    if (!app_instance)
    {
        return NULL;
    }

    if (!app_instance->submenu_explore)
    {
        if (!easy_flipper_set_submenu(&app_instance->submenu_explore, FlipSocialViewLoggedInExploreSubmenu, "Explore", flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
        {
            return NULL;
        }
    }

    // Allocate memory for each username only if not already allocated
    FlipSocialModel *explore = malloc(sizeof(FlipSocialModel));
    if (explore == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore model.");
        return NULL;
    }
    for (size_t i = 0; i < MAX_EXPLORE_USERS; i++)
    {
        if (explore->usernames[i] == NULL)
        {
            explore->usernames[i] = malloc(MAX_USER_LENGTH);
            if (explore->usernames[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for username %zu", i);
                return NULL; // Return false on memory allocation failure
            }
        }
    }
    return explore;
}

void flip_social_free_explore(void)
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->submenu_explore)
    {
        submenu_free(app_instance->submenu_explore);
        app_instance->submenu_explore = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedInExploreSubmenu);
    }
    if (!flip_social_explore)
    {
        return;
    }
    free(flip_social_explore);
    flip_social_explore = NULL;
}

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the explore view
bool flip_social_get_explore(void)
{
    if (!flipper_http_init(flipper_http_rx_callback, app_instance))
    {
        FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
        return false;
    }
    char *keyword = !app_instance->explore_logged_in || strlen(app_instance->explore_logged_in) == 0 ? "a" : app_instance->explore_logged_in;
    snprintf(
        fhttp.file_path,
        sizeof(fhttp.file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/%s.json",
        keyword);

    fhttp.save_received_data = true;
    auth_headers_alloc();
    char url[256];
    snprintf(url, sizeof(url), "https://www.flipsocial.net/api/user/explore/%s/%d/", keyword, MAX_EXPLORE_USERS);
    if (!flipper_http_get_request_with_headers(url, auth_headers))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for explore");
        fhttp.state = ISSUE;
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}
bool flip_social_get_explore_2(void)
{
    if (!flipper_http_init(flipper_http_rx_callback, app_instance))
    {
        FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
        return false;
    }
    char *keyword = !app_instance->message_users_logged_in || strlen(app_instance->message_users_logged_in) == 0 ? "a" : app_instance->message_users_logged_in;
    snprintf(
        fhttp.file_path,
        sizeof(fhttp.file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/%s.json",
        keyword);

    fhttp.save_received_data = true;
    auth_headers_alloc();
    char url[256];
    snprintf(url, sizeof(url), "https://www.flipsocial.net/api/user/explore/%s/%d/", keyword, MAX_EXPLORE_USERS);
    if (!flipper_http_get_request_with_headers(url, auth_headers))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for explore");
        fhttp.state = ISSUE;
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}

bool flip_social_parse_json_explore()
{
    // load the received data from the saved file
    FuriString *user_data = flipper_http_load_from_file(fhttp.file_path);
    if (user_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    flipper_http_deinit();

    char *data_cstr = (char *)furi_string_get_cstr(user_data);
    if (data_cstr == NULL)
    {
        FURI_LOG_E(TAG, "Failed to get C-string from FuriString.");
        furi_string_free(user_data);
        return false;
    }

    // Allocate memory for each username only if not already allocated
    flip_social_explore = flip_social_explore_alloc();
    if (flip_social_explore == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore usernames.");
        furi_string_free(user_data);
        free(data_cstr);
        return false;
    }

    // Initialize explore count
    flip_social_explore->count = 0;

    // set submenu
    submenu_reset(app_instance->submenu_explore);
    submenu_set_header(app_instance->submenu_explore, "Explore");
    // Parse the JSON array of usernames
    for (size_t i = 0; i < MAX_EXPLORE_USERS; i++)
    {
        char *username = get_json_array_value("users", i, data_cstr, 64); // currently its 53 tokens (with max explore users at 50)
        if (username == NULL)
        {
            break;
        }
        snprintf(flip_social_explore->usernames[i], MAX_USER_LENGTH, "%s", username);
        submenu_add_item(app_instance->submenu_explore, flip_social_explore->usernames[i], FlipSocialSubmenuExploreIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
        flip_social_explore->count++;
        free(username);
    }
    free(data_cstr);
    furi_string_free(user_data);
    return flip_social_explore->count > 0;
}
