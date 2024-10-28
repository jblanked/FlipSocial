#ifndef FLIP_SOCIAL_EXPLORE_H
#define FLIP_SOCIAL_EXPLORE_H

static bool flip_social_explore_alloc()
{
    // Allocate memory for each username only if not already allocated
    for (size_t i = 0; i < MAX_EXPLORE_USERS; i++)
    {
        if (app_instance->flip_social_explore.usernames[i] == NULL)
        {
            app_instance->flip_social_explore.usernames[i] = malloc(MAX_USER_LENGTH);
            if (app_instance->flip_social_explore.usernames[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for username %zu", i);
                return false; // Return false on memory allocation failure
            }
        }
    }
    return true;
}

static void flip_social_free_explore()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return;
    }
    for (int i = 0; i < app_instance->flip_social_explore.count; i++)
    {
        free(app_instance->flip_social_explore.usernames[i]);
    }
}

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the explore view
static bool flip_social_get_explore()
{
    // will return true unless the devboard is not connected
    bool success = flipper_http_get_request_with_headers("https://www.flipsocial.net/api/user/users/", jsmn("Content-Type", "application/json"));
    if (!success)
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for explore");
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}

static bool flip_social_parse_json_explore()
{
    if (fhttp.received_data == NULL)
    {
        FURI_LOG_E(TAG, "No data received.");
        return false;
    }

    // Allocate memory for each username only if not already allocated
    if (!flip_social_explore_alloc())
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore usernames.");
        return false;
    }

    // Remove newlines
    char *pos = fhttp.received_data;
    while ((pos = strchr(pos, '\n')) != NULL)
    {
        *pos = ' ';
    }

    // Initialize explore count
    app_instance->flip_social_explore.count = 0;

    // Extract the users array from the JSON
    char *json_users = get_json_value("users", fhttp.received_data, MAX_TOKENS);
    if (json_users == NULL)
    {
        FURI_LOG_E(TAG, "Failed to parse users array.");
        return false;
    }

    // Manual tokenization for comma-separated values
    char *start = json_users + 1; // Skip the opening bracket
    char *end;
    while ((end = strchr(start, ',')) != NULL && app_instance->flip_social_explore.count < MAX_EXPLORE_USERS)
    {
        *end = '\0'; // Null-terminate the current token

        // Remove quotes
        if (*start == '"')
            start++;
        if (*(end - 1) == '"')
            *(end - 1) = '\0';

        // Copy username to pre-allocated memory
        strncpy(app_instance->flip_social_explore.usernames[app_instance->flip_social_explore.count], start, MAX_USER_LENGTH - 1);
        app_instance->flip_social_explore.usernames[app_instance->flip_social_explore.count][MAX_USER_LENGTH - 1] = '\0'; // Ensure null termination
        app_instance->flip_social_explore.count++;
        start = end + 1;
    }

    // Handle the last token
    if (*start != '\0' && app_instance->flip_social_explore.count < MAX_EXPLORE_USERS)
    {
        if (*start == '"')
            start++;
        if (*(start + strlen(start) - 1) == ']')
            *(start + strlen(start) - 1) = '\0';
        if (*(start + strlen(start) - 1) == '"')
            *(start + strlen(start) - 1) = '\0';

        strncpy(app_instance->flip_social_explore.usernames[app_instance->flip_social_explore.count], start, MAX_USER_LENGTH - 1);
        app_instance->flip_social_explore.usernames[app_instance->flip_social_explore.count][MAX_USER_LENGTH - 1] = '\0'; // Ensure null termination
        app_instance->flip_social_explore.count++;
    }

    // Add submenu items for the users
    submenu_reset(app_instance->submenu_explore);
    submenu_set_header(app_instance->submenu_explore, "Explore");
    for (int i = 0; i < app_instance->flip_social_explore.count; i++)
    {
        submenu_add_item(app_instance->submenu_explore, app_instance->flip_social_explore.usernames[i], FlipSocialSubmenuExploreIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
    }

    // Free the json_users
    free(json_users);

    return true;
}

#endif // FLIP_SOCIAL_EXPLORE_H