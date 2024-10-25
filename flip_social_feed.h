#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H

bool flip_social_get_feed()
{
    // Get the feed from the server
    if (app_instance->login_username_logged_out == NULL)
    {
        FURI_LOG_E(TAG, "Username is NULL");
        return false;
    }
    char command[256];
    snprintf(command, 128, "https://www.flipsocial.net/api/feed/20/%s/", app_instance->login_username_logged_out);
    bool success = flipper_http_get_request_with_headers(command, "{\"Content-Type\":\"application/json\"}");
    if (!success)
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}
bool flip_social_parse_json_feed()
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

    // Initialize feed count
    flip_social_feed.count = 0;

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
            continue;
        }

        // Store parsed values
        flip_social_feed.usernames[i] = username;
        flip_social_feed.messages[i] = message;
        flip_social_feed.is_flipped[i] = strstr(flipped, "true") != NULL;
        flip_social_feed.ids[i] = atoi(id);
        flip_social_feed.count++;

        free(item);
    }

    return true;
}

#endif // FLIP_SOCIAL_FEED_H