#include "flip_social_feed.h"

bool flip_social_get_feed()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return false;
    }
    if (fhttp.state == INACTIVE)
    {
        FURI_LOG_E(TAG, "HTTP state is INACTIVE");
        return false;
    }
    // Get the feed from the server
    if (app_instance->login_username_logged_out == NULL)
    {
        FURI_LOG_E(TAG, "Username is NULL");
        return false;
    }
    snprintf(
        fhttp.file_path,
        sizeof(fhttp.file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/feed.json");

    fhttp.save_received_data = true;
    auth_headers_alloc();
    char command[96];
    snprintf(command, 96, "https://www.flipsocial.net/api/feed/50/%s/extended/", app_instance->login_username_logged_out);
    if (!flipper_http_get_request_with_headers(command, auth_headers))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
        fhttp.state = ISSUE;
        return false;
    }
    fhttp.state = RECEIVING;
    return true;
}

FlipSocialFeedMini *flip_social_parse_json_feed()
{
    // load the received data from the saved file
    FuriString *feed_data = flipper_http_load_from_file(fhttp.file_path);
    if (feed_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return NULL;
    }
    char *data_cstr = (char *)furi_string_get_cstr(feed_data);
    if (data_cstr == NULL)
    {
        FURI_LOG_E(TAG, "Failed to get C-string from FuriString.");
        furi_string_free(feed_data);
        return NULL;
    }

    FlipSocialFeedMini *feed_info = (FlipSocialFeedMini *)malloc(sizeof(FlipSocialFeedMini));
    if (!feed_info)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for feed_info");
        return NULL;
    }

    // Remove newlines
    char *pos = data_cstr;
    while ((pos = strchr(pos, '\n')) != NULL)
    {
        *pos = ' ';
    }

    int feed_count = 0;

    // Iterate through the feed array
    for (int i = 0; i < MAX_FEED_ITEMS; i++)
    {
        // Parse each item in the array
        char *item = get_json_array_value("feed", i, data_cstr, MAX_TOKENS);
        if (item == NULL)
        {
            break;
        }

        // Extract individual fields from the JSON object
        char *username = get_json_value("username", item, 64);
        char *message = get_json_value("message", item, 64);
        char *flipped = get_json_value("flipped", item, 64);
        char *flips = get_json_value("flip_count", item, 64);
        char *id = get_json_value("id", item, 64);

        if (username == NULL || message == NULL || flipped == NULL || id == NULL)
        {
            FURI_LOG_E(TAG, "Failed to parse item fields.");
            free(item);
            free(username);
            free(message);
            free(flipped);
            free(flips);
            free(id);
            continue;
        }

        if (!flip_social_save_post(id, item))
        {
            FURI_LOG_E(TAG, "Failed to save post.");
            free(item);
            free(username);
            free(message);
            free(flipped);
            free(flips);
            free(id);
            continue;
        }
        feed_count++;
        feed_info->ids[i] = atoi(id);

        // Free allocated memory
        free(item);
        free(username);
        free(message);
        free(flipped);
        free(flips);
        free(id);
    }

    // Store the number of feed items
    feed_info->count = feed_count;
    feed_info->index = 0;

    furi_string_free(feed_data);
    free(data_cstr);
    return feed_info;
}

bool flip_social_load_feed_post(int post_id)
{
    char file_path[128];
    snprintf(file_path, sizeof(file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/feed_post_%d.json", post_id);

    // load the received data from the saved file
    FuriString *feed_data = flipper_http_load_from_file(file_path);
    if (feed_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }
    char *data_cstr = (char *)furi_string_get_cstr(feed_data);
    if (data_cstr == NULL)
    {
        FURI_LOG_E(TAG, "Failed to get C-string from FuriString.");
        furi_string_free(feed_data);
        return false;
    }

    // Parse the feed post
    if (!flip_feed_item)
    {
        flip_feed_item = (FlipSocialFeedItem *)malloc(sizeof(FlipSocialFeedItem));
        if (flip_feed_item == NULL)
        {
            FURI_LOG_E(TAG, "Failed to allocate memory for feed post.");
            furi_string_free(feed_data);
            free(data_cstr);
            return false;
        }
        flip_feed_item->username = malloc(MAX_USER_LENGTH);
        flip_feed_item->message = malloc(MAX_MESSAGE_LENGTH);
    }

    // Extract individual fields from the JSON object
    char *username = get_json_value("username", data_cstr, 64);
    char *message = get_json_value("message", data_cstr, 64);
    char *flipped = get_json_value("flipped", data_cstr, 64);
    char *flips = get_json_value("flip_count", data_cstr, 64);
    char *id = get_json_value("id", data_cstr, 64);

    if (username == NULL || message == NULL || flipped == NULL || id == NULL)
    {
        FURI_LOG_E(TAG, "Failed to parse item fields.");
        free(username);
        free(message);
        free(flipped);
        free(flips);
        free(id);
        free(data_cstr);
        furi_string_free(feed_data);
        return false;
    }

    // Safely copy strings with bounds checking
    snprintf(flip_feed_item->username, MAX_USER_LENGTH, "%s", username);
    snprintf(flip_feed_item->message, MAX_MESSAGE_LENGTH, "%s", message);

    // Store boolean and integer values
    flip_feed_item->is_flipped = strstr(flipped, "true") != NULL;
    flip_feed_item->id = atoi(id);
    flip_feed_item->flips = atoi(flips);

    // Free allocated memory
    free(username);
    free(message);
    free(flipped);
    free(flips);
    free(id);

    furi_string_free(feed_data);
    free(data_cstr);

    return true;
}