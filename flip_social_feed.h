#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H

// Set failure FlipSocialFeed object
static bool flip_social_temp_feed()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    for (int i = 0; i < 3; i++)
    {
        if (app_instance->flip_social_feed.usernames[i] == NULL)
        {
            app_instance->flip_social_feed.usernames[i] = malloc(MAX_USER_LENGTH);
            if (app_instance->flip_social_feed.usernames[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for username %zu", i);
                return false;
            }
        }
        if (app_instance->flip_social_feed.messages[i] == NULL)
        {
            app_instance->flip_social_feed.messages[i] = malloc(MAX_MESSAGE_LENGTH);
            if (app_instance->flip_social_feed.messages[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for message %zu", i);
                return false;
            }
        }
    }
    app_instance->flip_social_feed.usernames[0] = "JBlanked";
    app_instance->flip_social_feed.usernames[1] = "FlipperKing";
    app_instance->flip_social_feed.usernames[2] = "FlipperQueen";
    //
    app_instance->flip_social_feed.messages[0] = "Welcome. This is a temp message. Either the feed didn't load or there was a server error.";
    app_instance->flip_social_feed.messages[1] = "I am the Chosen Flipper.";
    app_instance->flip_social_feed.messages[2] = "No one can flip like me.";
    //
    app_instance->flip_social_feed.is_flipped[0] = false;
    app_instance->flip_social_feed.is_flipped[1] = false;
    app_instance->flip_social_feed.is_flipped[2] = true;
    //
    app_instance->flip_social_feed.ids[0] = 0;
    app_instance->flip_social_feed.ids[1] = 1;
    app_instance->flip_social_feed.ids[2] = 2;
    //
    app_instance->flip_social_feed.count = 3;
    app_instance->flip_social_feed.index = 0;

    return true;
}

// Allocate memory for each feed item if not already allocated
static bool flip_social_feed_alloc()
{
    for (size_t i = 0; i < MAX_FEED_ITEMS; i++)
    {
        if (app_instance->flip_social_feed.usernames[i] == NULL)
        {
            app_instance->flip_social_feed.usernames[i] = malloc(MAX_USER_LENGTH);
            if (app_instance->flip_social_feed.usernames[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for username %zu", i);
                return false;
            }
        }
        if (app_instance->flip_social_feed.messages[i] == NULL)
        {
            app_instance->flip_social_feed.messages[i] = malloc(MAX_MESSAGE_LENGTH);
            if (app_instance->flip_social_feed.messages[i] == NULL)
            {
                FURI_LOG_E(TAG, "Failed to allocate memory for message %zu", i);
                return false;
            }
        }
    }
    return true;
}

static void flip_social_free_feed()
{
    for (uint32_t i = 0; i < app_instance->flip_social_feed.count; i++)
    {
        free(app_instance->flip_social_feed.usernames[i]);
    }
}

static bool flip_social_get_feed()
{
    // Get the feed from the server
    if (app_instance->login_username_logged_out == NULL)
    {
        FURI_LOG_E(TAG, "Username is NULL");
        return false;
    }
    char command[128];
    snprintf(command, 128, "https://www.flipsocial.net/api/feed/50/%s/", app_instance->login_username_logged_out);
    bool success = flipper_http_get_request_with_headers(command, jsmn("Content-Type", "application/json"));
    if (!success)
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}
static bool flip_social_parse_json_feed()
{
    if (fhttp.received_data == NULL)
    {
        FURI_LOG_E(TAG, "No data received.");
        return false;
    }

    // Allocate memory for each feed item if not already allocated
    if (!flip_social_feed_alloc())
    {
        return false;
    }

    // Remove newlines
    char *pos = fhttp.received_data;
    while ((pos = strchr(pos, '\n')) != NULL)
    {
        *pos = ' ';
    }

    // Initialize feed count
    app_instance->flip_social_feed.count = 0;

    // Iterate through the feed array
    for (int i = 0; i < MAX_FEED_ITEMS; i++)
    {
        // Parse each item in the array
        char *item = get_json_array_value("feed", i, fhttp.received_data, MAX_TOKENS);
        if (item == NULL)
        {
            break;
        }

        // Extract individual fields from the JSON object
        char *username = get_json_value("username", item, MAX_TOKENS);
        char *message = get_json_value("message", item, MAX_TOKENS);
        char *flipped = get_json_value("flipped", item, MAX_TOKENS);
        char *id = get_json_value("id", item, MAX_TOKENS);

        if (username == NULL || message == NULL || flipped == NULL || id == NULL)
        {
            FURI_LOG_E(TAG, "Failed to parse item fields.");
            free(item);
            free(username);
            free(message);
            free(flipped);
            free(id);
            continue;
        }

        // Copy parsed values into allocated memory
        app_instance->flip_social_feed.usernames[i] = username;
        app_instance->flip_social_feed.messages[i] = message;
        app_instance->flip_social_feed.is_flipped[i] = strstr(flipped, "true") != NULL;
        app_instance->flip_social_feed.ids[i] = atoi(id);
        app_instance->flip_social_feed.count++;

        // Free temporary JSON value
        free(item);
    }

    return true;
}

#endif // FLIP_SOCIAL_FEED_H