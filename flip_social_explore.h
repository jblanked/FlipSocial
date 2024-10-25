#ifndef FLIP_SOCIAL_EXPLORE_H
#define FLIP_SOCIAL_EXPLORE_H

// for now we're just listing the current users
// as the feed is upgraded, then we can port more to the explore view
bool flip_social_get_explore()
{
    // will return true unless the devboard is not connected
    bool success = flipper_http_get_request_with_headers("https://www.flipsocial.net/api/user/users/", "{\"Content-Type\":\"application/json\"}");
    if (!success)
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for explore");
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}

bool flip_social_parse_json_explore()
{
    if (fhttp.received_data == NULL)
    {
        FURI_LOG_E(TAG, "No data received.");
        return false;
    }

    // Remove newlines
    char *pos = fhttp.received_data;
    while ((pos = strchr(pos, '\n')) != NULL)
    {
        *pos = ' ';
    }

    // Initialize explore count
    flip_social_explore.count = 0;

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
    while ((end = strchr(start, ',')) != NULL)
    {
        *end = '\0'; // Null-terminate the current token

        // Remove the quotes
        if (*start == '"')
        {
            start++;
        }
        if (*(end - 1) == '"')
        {
            *(end - 1) = '\0';
        }

        // Allocate memory for the username
        size_t length = strlen(start) + 1;
        flip_social_explore.usernames[flip_social_explore.count] = malloc(length);
        if (flip_social_explore.usernames[flip_social_explore.count] == NULL)
        {
            FURI_LOG_E(TAG, "Memory allocation failed.");
            return false;
        }

        // Copy the username to the allocated memory
        strncpy(flip_social_explore.usernames[flip_social_explore.count], start, length);
        flip_social_explore.count++;
        start = end + 1; // Move to the next token
    }

    // Handle the last token
    if (*start != '\0')
    {
        // Remove the quotes
        if (*start == '"')
        {
            start++;
        }
        // Skip the closing bracket
        if (*(start + strlen(start) - 1) == ']')
        {
            *(start + strlen(start) - 1) = '\0';
        }
        // Remove the quotes
        if (*(start + strlen(start) - 1) == '"')
        {
            *(start + strlen(start) - 1) = '\0';
        }

        // Allocate memory for the username
        size_t length = strlen(start) + 1;
        flip_social_explore.usernames[flip_social_explore.count] = malloc(length);
        if (flip_social_explore.usernames[flip_social_explore.count] == NULL)
        {
            FURI_LOG_E(TAG, "Memory allocation failed.");
            return false;
        }

        // Copy the username to the allocated memory
        strncpy(flip_social_explore.usernames[flip_social_explore.count], start, length);
        flip_social_explore.count++;
    }

    // Add submenu items for the users
    submenu_reset(app_instance->submenu_explore);
    submenu_set_header(app_instance->submenu_explore, "Explore");
    for (uint32_t i = 0; i < flip_social_explore.count; i++)
    {
        submenu_add_item(app_instance->submenu_explore, flip_social_explore.usernames[i], FlipSocialSubmenuExploreIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
    }

    // Free the json_users
    free(json_users);

    return true;
}

#endif // FLIP_SOCIAL_EXPLORE_H