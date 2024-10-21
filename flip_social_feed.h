#ifndef FLIP_SOCIAL_FEED_H
#define FLIP_SOCIAL_FEED_H

static FlipSocialApp *app_instance = NULL;

#define MAX_TOKENS 128 // Adjust based on expected JSON tokens

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
    // Parse the JSON feed
    if (fhttp.received_data != NULL)
    {
        jsmn_parser parser;
        jsmn_init(&parser);

        // Allocate tokens array on the heap
        jsmntok_t *tokens = malloc(sizeof(jsmntok_t) * MAX_TOKENS);
        if (tokens == NULL)
        {
            FURI_LOG_E(TAG, "Failed to allocate memory for JSON tokens.");
            return false;
        }

        int ret = jsmn_parse(&parser, fhttp.received_data, strlen(fhttp.received_data), tokens, MAX_TOKENS);

        if (ret < 0)
        {
            // Handle parsing errors
            FURI_LOG_E(TAG, "Failed to parse JSON: %d", ret);
            free(tokens);
            return false;
        }

        // Ensure that the root element is an object
        if (ret < 1 || tokens[0].type != JSMN_OBJECT)
        {
            FURI_LOG_E(TAG, "Root element is not an object.");
            free(tokens);
            return false;
        }

        // Initialize feed count
        flip_social_feed.count = 0;

        // Loop over all keys in the root object
        int i = 0;
        for (i = 1; i < ret; i++)
        {
            if (jsoneq(fhttp.received_data, &tokens[i], "feed") == 0)
            {
                // Found "feed" key
                jsmntok_t *feed_array = &tokens[i + 1];

                if (feed_array->type != JSMN_ARRAY)
                {
                    FURI_LOG_E(TAG, "'feed' is not an array.");
                    break;
                }

                int j, k;
                int feed_index = 0;

                // Iterate over the feed array
                for (j = 0; j < feed_array->size; j++)
                {
                    int idx = i + 2; // Position of the first feed item
                    for (k = 0; k < j; k++)
                    {
                        // Skip tokens of previous feed items
                        idx += tokens[idx].size * 2 + 1;
                    }

                    if (idx >= ret)
                    {
                        FURI_LOG_E(TAG, "Index out of bounds while accessing feed items.");
                        break;
                    }

                    jsmntok_t *item = &tokens[idx];
                    if (item->type != JSMN_OBJECT)
                    {
                        FURI_LOG_E(TAG, "Feed item is not an object.");
                        continue;
                    }

                    // Variables to hold item data
                    char *username = NULL;
                    char *message = NULL;
                    int flipped = 0;
                    int id = 0;

                    // Iterate over keys in the feed item
                    int l;
                    int item_size = item->size;
                    int item_idx = idx + 1; // Position of the first key in the item

                    for (l = 0; l < item_size; l++)
                    {
                        if (item_idx + 1 >= ret)
                        {
                            FURI_LOG_E(TAG, "Index out of bounds while accessing item properties.");
                            break;
                        }

                        jsmntok_t *key = &tokens[item_idx];
                        jsmntok_t *val = &tokens[item_idx + 1];

                        if (jsoneq(fhttp.received_data, key, "username") == 0)
                        {
                            username = strndup(fhttp.received_data + val->start, val->end - val->start);
                        }
                        else if (jsoneq(fhttp.received_data, key, "message") == 0)
                        {
                            message = strndup(fhttp.received_data + val->start, val->end - val->start);
                        }
                        else if (jsoneq(fhttp.received_data, key, "flipped") == 0)
                        {
                            if (val->type == JSMN_PRIMITIVE)
                            {
                                if (strncmp(fhttp.received_data + val->start, "true", val->end - val->start) == 0)
                                    flipped = 1;
                                else
                                    flipped = 0;
                            }
                        }
                        else if (jsoneq(fhttp.received_data, key, "id") == 0)
                        {
                            if (val->type == JSMN_PRIMITIVE)
                            {
                                char id_str[16] = {0};
                                uint32_t id_len = val->end - val->start;
                                if (id_len >= sizeof(id_str))
                                    id_len = sizeof(id_str) - 1;
                                strncpy(id_str, fhttp.received_data + val->start, id_len);
                                id = atoi(id_str);
                            }
                        }

                        item_idx += 2; // Move to the next key-value pair
                    }

                    // Store the data in flip_social_feed
                    if (username && message && feed_index < MAX_FEED_ITEMS)
                    {
                        flip_social_feed.usernames[feed_index] = username;
                        flip_social_feed.messages[feed_index] = message;
                        flip_social_feed.is_flipped[feed_index] = flipped;
                        flip_social_feed.ids[feed_index] = id;
                        feed_index++;
                        flip_social_feed.count = feed_index;
                    }
                    else
                    {
                        // Free allocated memory if not stored
                        if (username)
                            free(username);
                        if (message)
                            free(message);
                    }
                }
                break; // Feed processed
            }
        }

        free(tokens); // Free the allocated tokens array
    }
    else
    {
        FURI_LOG_E(TAG, "No data received.");
        return false;
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