#ifndef FLIP_SOCIAL_FRIENDS
#define FLIP_SOCIAL_FRIENDS

static bool flip_social_friends_alloc()
{
    // Allocate memory for each username only if not already allocated
    for (size_t i = 0; i < MAX_FRIENDS; i++)
    {
        if (app_instance->flip_social_friends.usernames[i] == NULL)
        {
            app_instance->flip_social_friends.usernames[i] = malloc(MAX_USER_LENGTH);
            if (app_instance->flip_social_friends.usernames[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for username %zu", i);
                return false; // Return false on memory allocation failure
            }
        }
    }
    return true;
}

static void flip_social_free_friends()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return;
    }
    for (int i = 0; i < app_instance->flip_social_friends.count; i++)
    {
        free(app_instance->flip_social_friends.usernames[i]);
    }
}

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the friends view
static bool flip_social_get_friends()
{
    // will return true unless the devboard is not connected
    char url[100];
    snprintf(url, 100, "https://www.flipsocial.net/api/user/friends/%s/", app_instance->login_username_logged_in);
    bool success = flipper_http_get_request_with_headers(url, jsmn("Content-Type", "application/json"));
    if (!success)
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for friends");
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}

static bool flip_social_update_friends()
{
    if (!app_instance->submenu_friends)
    {
        FURI_LOG_E(TAG, "Friends submenu is NULL");
        return false;
    }
    // Add submenu items for the users
    submenu_reset(app_instance->submenu_friends);
    submenu_set_header(app_instance->submenu_friends, "Friends");
    for (int i = 0; i < app_instance->flip_social_friends.count; i++)
    {
        submenu_add_item(app_instance->submenu_friends, app_instance->flip_social_friends.usernames[i], FlipSocialSubmenuLoggedInIndexFriendsStart + i, flip_social_callback_submenu_choices, app_instance);
    }
    return true;
}

static bool flip_social_parse_json_friends()
{
    if (fhttp.received_data == NULL)
    {
        FURI_LOG_E(TAG, "No data received.");
        return false;
    }

    // Allocate memory for each username only if not already allocated
    if (!flip_social_friends_alloc())
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for friends usernames.");
        return false;
    }

    // Remove newlines
    char *pos = fhttp.received_data;
    while ((pos = strchr(pos, '\n')) != NULL)
    {
        *pos = ' ';
    }

    // Initialize friends count
    app_instance->flip_social_friends.count = 0;

    // Extract the users array from the JSON
    char *json_users = get_json_value("friends", fhttp.received_data, MAX_TOKENS);
    if (json_users == NULL)
    {
        FURI_LOG_E(TAG, "Failed to parse friends array.");
        return false;
    }

    // Manual tokenization for comma-separated values
    char *start = json_users + 1; // Skip the opening bracket
    char *end;
    while ((end = strchr(start, ',')) != NULL && app_instance->flip_social_friends.count < MAX_FRIENDS)
    {
        *end = '\0'; // Null-terminate the current token

        // Remove quotes
        if (*start == '"')
            start++;
        if (*(end - 1) == '"')
            *(end - 1) = '\0';

        // Copy username to pre-allocated memory
        strncpy(app_instance->flip_social_friends.usernames[app_instance->flip_social_friends.count], start, MAX_USER_LENGTH - 1);
        app_instance->flip_social_friends.usernames[app_instance->flip_social_friends.count][MAX_USER_LENGTH - 1] = '\0'; // Ensure null termination
        app_instance->flip_social_friends.count++;
        start = end + 1;
    }

    // Handle the last token
    if (*start != '\0' && app_instance->flip_social_friends.count < MAX_FRIENDS)
    {
        if (*start == '"')
            start++;
        if (*(start + strlen(start) - 1) == ']')
            *(start + strlen(start) - 1) = '\0';
        if (*(start + strlen(start) - 1) == '"')
            *(start + strlen(start) - 1) = '\0';

        strncpy(app_instance->flip_social_friends.usernames[app_instance->flip_social_friends.count], start, MAX_USER_LENGTH - 1);
        app_instance->flip_social_friends.usernames[app_instance->flip_social_friends.count][MAX_USER_LENGTH - 1] = '\0'; // Ensure null termination
        app_instance->flip_social_friends.count++;
    }

    // Add submenu items for the friends
    if (!flip_social_update_friends())
    {
        FURI_LOG_E(TAG, "Failed to update friends submenu");
        return false;
    }

    // Free the json_users
    free(json_users);

    return true;
}
#endif // FLIP_SOCIAL_FRIENDS