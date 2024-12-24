#include "flip_social_feed.h"

bool flip_social_get_feed(bool alloc_http, int series_index)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return false;
    }
    if (alloc_http && !flipper_http_init(flipper_http_rx_callback, app_instance))
    {
        FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
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
    if (strstr(flip_social_feed_type[flip_social_feed_type_index], "Global"))
    {
        snprintf(command, 96, "https://www.flipsocial.net/api/feed/%d/%s/%d/max/series/", MAX_FEED_ITEMS, app_instance->login_username_logged_out, series_index);
    }
    else
    {
        snprintf(command, 96, "https://www.flipsocial.net/api/feed/%d/%s/%d/max/friends/series/", MAX_FEED_ITEMS, app_instance->login_username_logged_out, series_index);
    }
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
        flipper_http_deinit();
        return NULL;
    }
    flipper_http_deinit();

    FlipSocialFeedMini *feed_info = (FlipSocialFeedMini *)malloc(sizeof(FlipSocialFeedMini));
    if (!feed_info)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for feed_info");
        return NULL;
    }

    int feed_count = 0;

    // Iterate through the feed array
    for (int i = 0; i < MAX_FEED_ITEMS; i++)
    {
        // Parse each item in the array
        FuriString *item = get_json_array_value_furi("feed", i, feed_data);
        if (item == NULL)
        {
            break;
        }

        // Extract individual fields from the JSON object
        FuriString *id = get_json_value_furi("id", item);
        if (id == NULL)
        {
            FURI_LOG_E(TAG, "Failed to parse item fields.");
            furi_string_free(item);
            furi_string_free(id);
            continue;
        }
        if (!flip_social_save_post(furi_string_get_cstr(id), furi_string_get_cstr(item)))
        {
            FURI_LOG_E(TAG, "Failed to save post.");
            furi_string_free(item);
            furi_string_free(id);
            continue;
        }
        feed_count++;
        feed_info->ids[i] = atoi(furi_string_get_cstr(id));

        // Furi_string_free allocated memory
        furi_string_free(item);
        furi_string_free(id);
    }

    // Store the number of feed items
    feed_info->count = feed_count;
    feed_info->index = 0;

    furi_string_free(feed_data);
    return feed_info;
}

bool flip_social_load_feed_post(int post_id)
{
    char file_path[128];
    snprintf(file_path, sizeof(file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/feed/feed_post_%d.json", post_id);

    // load the received data from the saved file
    FuriString *feed_data = flipper_http_load_from_file(file_path);
    if (feed_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    // Parse the feed post
    if (flip_feed_item)
    {
        free(flip_feed_item);
    }
    else
    {
        // first time
        save_char("series_index", "1");
    }

    flip_feed_item = (FlipSocialFeedItem *)malloc(sizeof(FlipSocialFeedItem));
    if (flip_feed_item == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for feed post.");
        furi_string_free(feed_data);
        return false;
    }

    // Extract individual fields from the JSON object
    FuriString *username = get_json_value_furi("username", feed_data);
    FuriString *message = get_json_value_furi("message", feed_data);
    FuriString *flipped = get_json_value_furi("flipped", feed_data);
    FuriString *flips = get_json_value_furi("flip_count", feed_data);
    FuriString *date_created = get_json_value_furi("date_created", feed_data);
    if (username == NULL || message == NULL || flipped == NULL || flips == NULL || date_created == NULL)
    {
        FURI_LOG_E(TAG, "Failed to parse item fields.");
        furi_string_free(username);
        furi_string_free(message);
        furi_string_free(flipped);
        furi_string_free(flips);
        furi_string_free(date_created);
        furi_string_free(feed_data);
        return false;
    }

    // Safely copy strings with bounds checking
    snprintf(flip_feed_item->username, MAX_USER_LENGTH, "%s", furi_string_get_cstr(username));
    snprintf(flip_feed_item->message, MAX_MESSAGE_LENGTH, "%s", furi_string_get_cstr(message));
    snprintf(flip_feed_item->date_created, MAX_LINE_LENGTH, "%s", furi_string_get_cstr(date_created));

    // Store boolean and integer values
    flip_feed_item->is_flipped = strstr(furi_string_get_cstr(flipped), "true") != NULL;
    flip_feed_item->id = post_id;
    flip_feed_item->flips = atoi(furi_string_get_cstr(flips));

    // Free allocated memory
    furi_string_free(username);
    furi_string_free(message);
    furi_string_free(flipped);
    furi_string_free(flips);
    furi_string_free(date_created);
    furi_string_free(feed_data);
    return true;
}

bool flip_social_load_initial_feed(bool alloc_http, int series_index)
{
    if (fhttp.state == INACTIVE)
    {
        FURI_LOG_E(TAG, "HTTP state is INACTIVE");
        return false;
    }
    Loading *loading;
    int32_t loading_view_id = 987654321; // Random ID
    loading = loading_alloc();
    if (!loading)
    {
        FURI_LOG_E(TAG, "Failed to set loading screen");
        return false;
    }
    view_dispatcher_add_view(app_instance->view_dispatcher, loading_view_id, loading_get_view(loading));
    view_dispatcher_switch_to_view(app_instance->view_dispatcher, loading_view_id);
    if (flip_social_get_feed(alloc_http, series_index)) // start the async request
    {
        furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        fhttp.state = RECEIVING;
    }
    else
    {
        FURI_LOG_E(HTTP_TAG, "Failed to send request");
        fhttp.state = ISSUE;
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        view_dispatcher_remove_view(app_instance->view_dispatcher, loading_view_id);
        loading_free(loading);
        return false;
    }
    while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
    {
        // Wait for the request to be received
        furi_delay_ms(100);
    }
    furi_timer_stop(fhttp.get_timeout_timer);

    // load feed info
    flip_feed_info = flip_social_parse_json_feed();
    if (!flip_feed_info || flip_feed_info->count < 1)
    {
        FURI_LOG_E(TAG, "Failed to parse JSON feed");
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        view_dispatcher_remove_view(app_instance->view_dispatcher, loading_view_id);
        loading_free(loading);
        return false;
    }

    // load the current feed post
    if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
    {
        FURI_LOG_E(TAG, "Failed to load feed post");
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        view_dispatcher_remove_view(app_instance->view_dispatcher, loading_view_id);
        loading_free(loading);
        return false;
    }
    if (!feed_view_alloc())
    {
        FURI_LOG_E(TAG, "Failed to allocate feed dialog");
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        view_dispatcher_remove_view(app_instance->view_dispatcher, loading_view_id);
        loading_free(loading);
        return false;
    }
    view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
    view_dispatcher_remove_view(app_instance->view_dispatcher, loading_view_id);
    loading_free(loading);

    return true;
}