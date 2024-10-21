#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H

static FlipSocialApp *app_instance = NULL;

#define MAX_TOKENS 512 // Adjust based on expected JSON tokens

typedef struct
{
    char *usernames[128];
    char *messages[128];
    bool is_flipped[128];
    uint32_t ids[128];
    size_t count;
    size_t index;
} FlipSocialFeed;

#define MAX_FEED_ITEMS 128
#define MAX_LINE_LENGTH 30

// temporary FlipSocialFeed object
static FlipSocialFeed flip_social_feed = {
    .usernames = {"JBlanked", "FlipperKing", "FlipperQueen"},
    .messages = {"Welcome. This is a temp message. Either the feed didn't load or there was a server error.", "I am the Chosen Flipper.", "No one can flip like me."},
    .is_flipped = {false, false, true},
    .ids = {0, 1, 2},
    .count = 3,
    .index = 0};

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

bool flip_social_board_is_active(Canvas *canvas)
{
    if (fhttp.state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return false;
    }
    return true;
}

void flip_social_handle_error(Canvas *canvas)
{
    if (fhttp.received_data != NULL)
    {
        if (strstr(fhttp.received_data, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(fhttp.received_data, "[ERROR] Failed to connect to Wifi.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else
        {
            canvas_draw_str(canvas, 0, 42, "Failed...");
            canvas_draw_str(canvas, 0, 52, "Update your credentials.");
            canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
        }
    }
    else
    {
        canvas_draw_str(canvas, 0, 42, "Failed...");
        canvas_draw_str(canvas, 0, 52, "Update your credentials.");
        canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
    }
}

#endif // FLIP_SOCIAL_FEED_H