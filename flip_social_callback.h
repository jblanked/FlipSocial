// flip_social_callback.h
#ifndef FLIP_SOCIAL_CALLBACK_H
#define FLIP_SOCIAL_CALLBACK_H
static FlipSocialApp *app_instance = NULL;
bool flip_social_sent_login_request = false;
bool flip_social_sent_register_request = false;
bool flip_social_login_success = false;
bool flip_social_register_success = false;
bool flip_social_dialog_shown = false;
bool flip_social_dialog_stop = false;
uint32_t flip_social_pre_saved_message_clicked_index = 0;
static void flip_social_logged_in_compose_pre_save_updated(void *context);
static void flip_social_callback_submenu_choices(void *context, uint32_t index);
// include strndup
char *strndup(const char *s, size_t n)
{
    char *result;
    size_t len = strlen(s);

    if (n < len)
        len = n;

    result = (char *)malloc(len + 1);
    if (!result)
        return NULL;

    result[len] = '\0';
    return (char *)memcpy(result, s, len);
}

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

#define MAX_TOKENS 128 // Adjust based on expected JSON size

// Helper function to compare JSON keys
int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0)
    {
        return 0;
    }
    return -1;
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

static void on_input(const void *event, void *ctx)
{
    UNUSED(ctx);

    InputKey key = ((InputEvent *)event)->key;
    InputType type = ((InputEvent *)event)->type;

    if (type != InputTypeRelease)
    {
        return;
    }

    switch (key)
    {
    case InputKeyOk:
        action = ActionFlip;
        break;
    case InputKeyBack:
        action = ActionBack;
        break;
    case InputKeyRight:
        action = ActionNext;
        break;
    case InputKeyLeft:
        action = ActionPrev;
        break;
    case InputKeyUp:
        action = ActionPrev;
        break;
    case InputKeyDown:
        action = ActionNext;
        break;
    default:
        action = ActionNone;
        break;
    }
}

// Function to draw the message on the canvas with word wrapping
void draw_user_message(Canvas *canvas, const char *user_message, int x)
{
    if (user_message == NULL)
    {
        FURI_LOG_E(TAG, "User message is NULL.");
        return;
    }

    size_t msg_length = strlen(user_message);
    size_t start = 0;
    int line_num = 0;
    char line[MAX_LINE_LENGTH + 1]; // Buffer for the current line (+1 for null terminator)

    while (start < msg_length && line_num < MAX_FEED_ITEMS)
    {
        size_t remaining = msg_length - start;
        size_t len = (remaining > MAX_LINE_LENGTH) ? MAX_LINE_LENGTH : remaining;

        if (remaining > MAX_LINE_LENGTH)
        {
            // Find the last space within the first 'len' characters
            size_t last_space = len;
            while (last_space > 0 && user_message[start + last_space - 1] != ' ')
            {
                last_space--;
            }

            if (last_space > 0)
            {
                len = last_space; // Adjust len to the position of the last space
            }
            // If no space is found, len remains MAX_LINE_LENGTH to force split
        }

        // Copy the substring to 'line' and null-terminate it
        memcpy(line, user_message + start, len);
        line[len] = '\0'; // Ensure the string is null-terminated

        // Debug Logging: Print the current line being drawn
        FURI_LOG_D(TAG, "Drawing line %d: \"%s\"", line_num + 1, line);

        // Draw the string on the canvas
        // Adjust the y-coordinate based on the line number
        canvas_draw_str_aligned(canvas, 0, x + line_num * 10, AlignLeft, AlignTop, line);

        // Update the start position for the next line
        start += len;

        // Skip any spaces to avoid leading spaces on the next line
        while (start < msg_length && user_message[start] == ' ')
        {
            start++;
        }

        // Increment the line number
        line_num++;
    }

    // Handle any remaining text that wasn't processed due to exceeding MAX_FEED_ITEMS
    if (start < msg_length)
    {
        FURI_LOG_E(TAG, "Message exceeds maximum number of lines (%d).", MAX_FEED_ITEMS);
    }
}

static void flip_social_callback_draw_compose(Canvas *canvas, void *model)
{
    UNUSED(model);
    if (!canvas)
    {
        FURI_LOG_E(TAG, "Canvas is NULL");
        return;
    }
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    char *message = app_instance->pre_saved_messages.messages[flip_social_pre_saved_message_clicked_index];

    if (!flip_social_dialog_shown)
    {
        flip_social_dialog_shown = true;
        app_instance->input_event_queue = furi_record_open(RECORD_INPUT_EVENTS);
        app_instance->input_event = furi_pubsub_subscribe(app_instance->input_event_queue, on_input, NULL);
    }

    draw_user_message(canvas, message, 2);

    canvas_draw_icon(canvas, 0, 53, &I_ButtonLeft_4x7);
    canvas_draw_str_aligned(canvas, 7, 54, AlignLeft, AlignTop, "Delete");
    canvas_draw_icon(canvas, 52, 53, &I_ButtonBACK_10x8);
    canvas_draw_str_aligned(canvas, 64, 54, AlignLeft, AlignTop, "Back");
    canvas_draw_icon(canvas, 100, 53, &I_ButtonRight_4x7);
    canvas_draw_str_aligned(canvas, 107, 54, AlignLeft, AlignTop, "Post");

    // handle action
    switch (action)
    {
    case ActionNone:
        break;
    case ActionBack:
        flip_social_dialog_stop = true;
        break;
    case ActionNext:
        // send message
        if (message && app_instance->login_username_logged_in)
        {
            // Send the message
            char command[256];
            snprintf(command, sizeof(command), "{\"username\":\"%s\",\"content\":\"%s\"}",
                     app_instance->login_username_logged_in, message);

            bool success = flipper_http_post_request_with_headers(
                "https://www.flipsocial.net/api/feed/post/",
                "{\"Content-Type\":\"application/json\"}",
                command);

            if (!success)
            {
                FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
                furi_check(success); // Log the error with furi_check
                return;              // Exit early to avoid further errors
            }

            fhttp.state = RECEIVING;
            furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        }
        else
        {
            FURI_LOG_E(TAG, "Message or username is NULL");
            furi_check(false); // Log as an error and return
            return;
        }

        int i = 0;
        while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
        {
            // Wait for the feed to be received
            furi_delay_ms(100);

            char dots_str[256] = "Receiving";

            // Append dots to the string based on the value of i
            int dot_count = i % 4;
            int len = strlen(dots_str);
            snprintf(dots_str + len, sizeof(dots_str) - len, "%.*s", dot_count, "....");

            // Draw the resulting string on the canvas
            canvas_draw_str(canvas, 0, 30, dots_str);

            i++;
        }
        flip_social_dialog_stop = true;
        furi_timer_stop(fhttp.get_timeout_timer);
        break;
    case ActionPrev:
        // delete message
        // remove the message from app_instance->pre_saved_messages
        app_instance->pre_saved_messages.messages[flip_social_pre_saved_message_clicked_index] = NULL;

        for (uint32_t i = flip_social_pre_saved_message_clicked_index; i < app_instance->pre_saved_messages.count - 1; i++)
        {
            app_instance->pre_saved_messages.messages[i] = app_instance->pre_saved_messages.messages[i + 1];
        }
        app_instance->pre_saved_messages.count--;

        // add the item to the submenu
        submenu_reset(app_instance->submenu_compose);

        submenu_add_item(app_instance->submenu_compose, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, flip_social_callback_submenu_choices, app_instance);

        for (uint32_t i = 0; i < app_instance->pre_saved_messages.count; i++)
        {
            submenu_add_item(app_instance->submenu_compose, app_instance->pre_saved_messages.messages[i], FlipSocialSubemnuComposeIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
        }

        // save playlist
        save_playlist(&app_instance->pre_saved_messages);

        flip_social_dialog_stop = true;
        break;
    default:
        action = ActionNone;
        break;
    }

    if (flip_social_dialog_stop)
    {
        furi_pubsub_unsubscribe(app_instance->input_event_queue, app_instance->input_event);
        flip_social_dialog_shown = false;
        flip_social_dialog_stop = false;
        if (action == ActionBack || action == ActionNext)
        {
            action = ActionNone;
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        }
        else
        {
            action = ActionNone;
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInCompose);
        }
    }
}
// function to draw the dialog canvas
static void flip_social_canvas_draw_message(Canvas *canvas, char *user_username, char *user_message, bool is_flipped, bool show_prev, bool show_next)
{
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, user_username);
    canvas_set_font(canvas, FontSecondary);

    draw_user_message(canvas, user_message, 12);

    canvas_set_font(canvas, FontSecondary);
    if (show_prev)
    {
        canvas_draw_icon(canvas, 0, 53, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 9, 54, AlignLeft, AlignTop, "Prev");
    }
    if (!is_flipped)
    {
        canvas_draw_icon(canvas, 52, 53, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 61, 54, AlignLeft, AlignTop, "Flip");
    }
    else
    {
        canvas_draw_icon(canvas, 47, 53, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 56, 54, AlignLeft, AlignTop, "UnFlip");
    }
    if (show_next)
    {
        canvas_draw_icon(canvas, 98, 53, &I_ButtonRight_4x7);
        canvas_draw_str_aligned(canvas, 107, 54, AlignLeft, AlignTop, "Next");
    }
}
// Callback function to handle the feed dialog
static void flip_social_callback_draw_feed(Canvas *canvas, void *model)
{
    UNUSED(model);
    if (!canvas)
    {
        FURI_LOG_E(TAG, "Canvas is NULL");
        return;
    }
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    if (!flip_social_dialog_shown)
    {
        flip_social_dialog_shown = true;
        app_instance->input_event_queue = furi_record_open(RECORD_INPUT_EVENTS);
        app_instance->input_event = furi_pubsub_subscribe(app_instance->input_event_queue, on_input, NULL);
    }

    // handle action
    switch (action)
    {
    case ActionNone:
        flip_social_canvas_draw_message(canvas, flip_social_feed.usernames[flip_social_feed.index], flip_social_feed.messages[flip_social_feed.index], flip_social_feed.is_flipped[flip_social_feed.index], flip_social_feed.index > 0, flip_social_feed.index < flip_social_feed.count - 1);
        break;
    case ActionNext:
        canvas_clear(canvas);
        if (flip_social_feed.index < flip_social_feed.count - 1)
        {
            flip_social_feed.index++;
        }
        flip_social_canvas_draw_message(canvas, flip_social_feed.usernames[flip_social_feed.index], flip_social_feed.messages[flip_social_feed.index], flip_social_feed.is_flipped[flip_social_feed.index], flip_social_feed.index > 0, flip_social_feed.index < flip_social_feed.count - 1);
        action = ActionNone;
        break;
    case ActionPrev:
        canvas_clear(canvas);
        if (flip_social_feed.index > 0)
        {
            flip_social_feed.index--;
        }
        flip_social_canvas_draw_message(canvas, flip_social_feed.usernames[flip_social_feed.index], flip_social_feed.messages[flip_social_feed.index], flip_social_feed.is_flipped[flip_social_feed.index], flip_social_feed.index > 0, flip_social_feed.index < flip_social_feed.count - 1);
        action = ActionNone;
        break;
    case ActionFlip:
        canvas_clear(canvas);
        flip_social_feed.is_flipped[flip_social_feed.index] = !flip_social_feed.is_flipped[flip_social_feed.index];
        flip_social_canvas_draw_message(canvas, flip_social_feed.usernames[flip_social_feed.index], flip_social_feed.messages[flip_social_feed.index], flip_social_feed.is_flipped[flip_social_feed.index], flip_social_feed.index > 0, flip_social_feed.index < flip_social_feed.count - 1);
        action = ActionNone;
        // send post request to flip the message
        if (app_instance->login_username_logged_in == NULL)
        {
            FURI_LOG_E(TAG, "Username is NULL");
            return;
        }
        char payload[256];
        snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"post_id\":\"%lu\"}", app_instance->login_username_logged_in, flip_social_feed.ids[flip_social_feed.index]);
        flipper_http_post_request_with_headers("https://www.flipsocial.net/api/feed/flip/", "{\"Content-Type\":\"application/json\"}", payload);
        break;
    case ActionBack:
        canvas_clear(canvas);
        flip_social_dialog_stop = true;
        flip_social_feed.index = 0;
        action = ActionNone;
        break;
    default:
        break;
    }

    if (flip_social_dialog_stop)
    {
        furi_pubsub_unsubscribe(app_instance->input_event_queue, app_instance->input_event);
        flip_social_dialog_shown = false;
        flip_social_dialog_stop = false;
        action = ActionNone;
    }
}
/**
 * @brief Navigation callback for asynchonously handling the login process.
 * @param canvas The canvas to draw on.
 * @param model The model - unused
 * @return void
 */
static void flip_social_callback_draw_login(Canvas *canvas, void *model)
{
    UNUSED(model);
    if (!canvas)
    {
        FURI_LOG_E(TAG, "Canvas is NULL");
        return;
    }

    canvas_set_font(canvas, FontSecondary);

    if (fhttp.state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your Dev Board with the");
        canvas_draw_str(canvas, 0, 62, "FlipperHTTP firmware.");
        return;
    }

    canvas_draw_str(canvas, 0, 7, "Logging in...");

    // Perform login request
    if (!flip_social_sent_login_request)
    {

        if (!app_instance->login_username_logged_out || !app_instance->login_password_logged_out || strlen(app_instance->login_username_logged_out) == 0 || strlen(app_instance->login_password_logged_out) == 0)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Please enter your credentials.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            return;
        }

        flip_social_sent_login_request = true;

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->login_username_logged_out, app_instance->login_password_logged_out);
        flip_social_login_success = flipper_http_post_request_with_headers("https://www.flipsocial.net/api/login/", "{\"Content-Type\":\"application/json\"}", buffer);
        fhttp.state = RECEIVING;
    }
    // handle response
    if (flip_social_sent_login_request && flip_social_login_success)
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 0, 17, "Request Sent!");
        canvas_draw_str(canvas, 0, 32, "Awaiting reponse...");

        if (fhttp.state == IDLE && fhttp.received_data != NULL)
        {
            // read response
            if (strstr(fhttp.received_data, "[SUCCESS]") != NULL || strstr(fhttp.received_data, "User found") != NULL)
            {
                canvas_draw_str(canvas, 0, 42, "Login successful!");
                canvas_draw_str(canvas, 0, 62, "Welcome back!");

                app_instance->is_logged_in = "true";

                save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

                // send user to the logged in submenu
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
            }
            else if (strstr(fhttp.received_data, "User not found") != NULL)
            {
                canvas_clear(canvas);
                canvas_draw_str(canvas, 0, 10, "Account not found...");
                canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            }
            else if (strstr(fhttp.received_data, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
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
                canvas_draw_str(canvas, 0, 42, "Login failed...");
                canvas_draw_str(canvas, 0, 52, "Update your credentials.");
                canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
            }
        }
        else if ((fhttp.state == ISSUE || fhttp.state == INACTIVE) && fhttp.received_data != NULL)
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
                FURI_LOG_E(TAG, "Received an error: %s", fhttp.received_data);
                canvas_draw_str(canvas, 0, 42, "Login failed...");
                canvas_draw_str(canvas, 0, 52, "Update your credentials.");
                canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
            }
        }
    }
    else if (flip_social_sent_login_request && !flip_social_login_success)
    {
        canvas_clear(canvas);
        canvas_draw_str(canvas, 0, 10, "Failed sending request.");
        canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
        canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
    }
}

/**
 * @brief Navigation callback for asynchonously handling the register process.
 * @param canvas The canvas to draw on.
 * @param model The model - unused
 * @return void
 */
static void flip_social_callback_draw_register(Canvas *canvas, void *model)
{
    UNUSED(model);
    if (!canvas)
    {
        FURI_LOG_E(TAG, "Canvas is NULL");
        return;
    }

    canvas_set_font(canvas, FontSecondary);

    if (fhttp.state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If you board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your Dev Board with the");
        canvas_draw_str(canvas, 0, 62, "FlipperHTTP firmware.");
        return;
    }

    canvas_draw_str(canvas, 0, 7, "Registering...");

    // Perform login request
    if (!flip_social_sent_register_request)
    {
        // check if the username and password are valid
        if (!app_instance->register_username_logged_out || !app_instance->register_password_logged_out || strlen(app_instance->register_username_logged_out) == 0 || strlen(app_instance->register_password_logged_out) == 0)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Please enter your credentials.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            return;
        }

        // check if both passwords match
        if (strcmp(app_instance->register_password_logged_out, app_instance->register_password_2_logged_out) != 0)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Passwords do not match.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            return;
        }

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->register_username_logged_out, app_instance->register_password_logged_out);
        flip_social_register_success = flipper_http_post_request_with_headers("https://www.flipsocial.net/api/register/", "{\"Content-Type\":\"application/json\"}", buffer);

        flip_social_sent_register_request = true;
        // Set the state to RECEIVING to ensure we continue to see the receiving message
        fhttp.state = RECEIVING;
    }
    // handle response
    if (flip_social_sent_register_request && flip_social_register_success)
    {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 0, 17, "Request Sent!");
        canvas_draw_str(canvas, 0, 32, "Awaiting reponse...");

        if (fhttp.state == IDLE)
        {
            // read response
            if (fhttp.received_data != NULL && (strstr(fhttp.received_data, "[SUCCESS]") != NULL || strstr(fhttp.received_data, "User created") != NULL))
            {
                canvas_draw_str(canvas, 0, 42, "Registeration successful!");
                canvas_draw_str(canvas, 0, 62, "Welcome to FlipSocial!");

                // set the login credentials
                if (app_instance->login_username_logged_out)
                {
                    app_instance->login_username_logged_out = app_instance->register_username_logged_out;
                }
                if (app_instance->login_password_logged_out)
                {
                    app_instance->login_password_logged_out = app_instance->register_password_logged_out;
                    app_instance->change_password_logged_in = app_instance->register_password_logged_out;
                }

                app_instance->is_logged_in = "true";

                // save the credentials
                save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

                // send user to the logged in submenu
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
            }
            else if (strstr(fhttp.received_data, "Username or password not provided") != NULL)
            {
                canvas_clear(canvas);
                canvas_draw_str(canvas, 0, 10, "Please enter your credentials.");
                canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
            }
            else if (strstr(fhttp.received_data, "User already exists") != NULL || strstr(fhttp.received_data, "Multiple users found") != NULL)
            {
                canvas_draw_str(canvas, 0, 42, "Registration failed...");
                canvas_draw_str(canvas, 0, 52, "Username already exists.");
                canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
            }
            else
            {
                canvas_draw_str(canvas, 0, 42, "Registration failed...");
                canvas_draw_str(canvas, 0, 52, "Update your credentials.");
                canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
            }
        }
        else if (fhttp.state == ISSUE || fhttp.state == INACTIVE)
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
                canvas_draw_str(canvas, 0, 42, "Login failed...");
                canvas_draw_str(canvas, 0, 52, "Update your credentials.");
                canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
            }
        }
    }
    else if (flip_social_sent_register_request && !flip_social_register_success)
    {
        canvas_clear(canvas);
        canvas_draw_str(canvas, 0, 10, "Failed sending request.");
        canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
        canvas_draw_str(canvas, 0, 62, "Press BACK to return.");
    }
}

/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
static uint32_t flip_social_callback_exit_app(void *context)
{
    // Exit the application
    if (!context)
    {
        FURI_LOG_E(TAG, "Context is NULL");
        return VIEW_NONE;
    }
    UNUSED(context);
    return VIEW_NONE; // Return VIEW_NONE to exit the app
}

/**
 * @brief Handle ALL submenu item selections.
 * @param context The context - FlipSocialApp object.
 * @param index The FlipSocialSubmenuIndex item that was clicked.
 * @return void
 */
static void flip_social_callback_submenu_choices(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case FlipSocialSubmenuLoggedOutIndexLogin:
        flip_social_sent_login_request = false;
        flip_social_login_success = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLogin);
        break;
    case FlipSocialSubmenuLoggedOutIndexRegister:
        flip_social_sent_register_request = false;
        flip_social_register_success = false;
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
        break;
    case FlipSocialSubmenuLoggedOutIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutAbout);
        break;
    case FlipSocialSubmenuLoggedOutIndexWifiSettings:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettings);
        break;
    case FlipSocialSubmenuLoggedInIndexProfile:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInProfile);
        break;
    case FlipSocialSubmenuLoggedInIndexFeed:
        if (flip_social_get_feed()) // start the async feed request
        {
            furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        }
        while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
        {
            // Wait for the feed to be received
            furi_delay_ms(100);
        }
        furi_timer_stop(fhttp.get_timeout_timer);

        if (!flip_social_parse_json_feed()) // parse the JSON before switching to the feed (synchonous)
        {
            FURI_LOG_E(TAG, "Failed to parse the JSON feed. Using the temporary feed.");
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInFeed);
        break;
    case FlipSocialSubmenuLoggedInIndexCompose:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInCompose);
        break;
    case FlipSocialSubmenuLoggedInIndexSettings:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettings);
        break;
    case FlipSocialSubmenuLoggedInSignOutButton:
        app->is_logged_in = "false";

        save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        break;
    case FlipSocialSubmenuComposeIndexAddPreSave:
        uart_text_input_set_header_text(app->text_input_logged_in_compose_pre_save_input, "Enter your message:");
        uart_text_input_set_result_callback(app->text_input_logged_in_compose_pre_save_input, flip_social_logged_in_compose_pre_save_updated, app, app->compose_pre_save_logged_in_temp_buffer, app->compose_pre_save_logged_in_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInComposeAddPreSaveInput);
        break;
    default:
        // handle FlipSocialSubemnuComposeIndexStartIndex + i
        // set temp variable to hold the index
        flip_social_pre_saved_message_clicked_index = index - FlipSocialSubemnuComposeIndexStartIndex;
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInProcessCompose);
        break;
    }
}

/**
 * @brief Navigation callback to go back to the submenu Logged out.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutSubmenu)
 */
static uint32_t flip_social_callback_to_submenu_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedOutSubmenu;
}

/**
 * @brief Navigation callback to go back to the submenu Logged in.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSubmenu)
 */
static uint32_t flip_social_callback_to_submenu_logged_in(void *context)
{
    UNUSED(context);
    // flip_social_get_feed(); // start the feed request
    return FlipSocialViewLoggedInSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Login screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutLogin)
 */
static uint32_t flip_social_callback_to_login_logged_out(void *context)
{
    UNUSED(context);
    flip_social_sent_login_request = false;
    flip_social_login_success = false;
    return FlipSocialViewLoggedOutLogin;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Register screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutRegister)
 */
static uint32_t flip_social_callback_to_register_logged_out(void *context)
{
    UNUSED(context);
    flip_social_sent_register_request = false;
    flip_social_register_success = false;
    return FlipSocialViewLoggedOutRegister;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutWifiSettings)
 */
static uint32_t flip_social_callback_to_wifi_settings_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedOutWifiSettings;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
static uint32_t flip_social_callback_to_wifi_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInSettingsWifi;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
static uint32_t flip_social_callback_to_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInSettings;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Compose screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInCompose)
 */
static uint32_t flip_social_callback_to_compose_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInCompose;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Profile screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInProfile)
 */
static uint32_t flip_social_callback_to_profile_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInProfile;
}

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_out_wifi_settings_ssid_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->wifi_ssid_logged_out, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->wifi_ssid_logged_out[app->wifi_ssid_logged_out_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_out_wifi_settings_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_wifi_settings_ssid, app->wifi_ssid_logged_out);
    }

    // update the wifi settings
    if (!flipper_http_save_wifi(app->wifi_ssid_logged_out, app->wifi_password_logged_out))
    {
        FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettings);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_out_wifi_settings_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->wifi_password_logged_out, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->wifi_password_logged_out[app->wifi_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_wifi_settings_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_wifi_settings_password, app->wifi_password_logged_out);
    }

    // update the wifi settings
    if (!flipper_http_save_wifi(app->wifi_ssid_logged_out, app->wifi_password_logged_out))
    {
        FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettings);
}

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
static void flip_social_text_input_logged_out_wifi_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input SSID
        // Initialize temp_buffer with the current name
        if (app->wifi_ssid_logged_out && strlen(app->wifi_ssid_logged_out) > 0)
        {
            strncpy(app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out, app->wifi_ssid_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->wifi_ssid_logged_out_temp_buffer, "", app->wifi_ssid_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_wifi_settings_ssid, "Enter SSID");
        uart_text_input_set_result_callback(app->text_input_logged_out_wifi_settings_ssid, flip_social_logged_out_wifi_settings_ssid_updated, app, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsSSIDInput);
        break;
    case 1: // Input Password
        // Initialize temp_buffer with the current password
        if (app->wifi_password_logged_out && strlen(app->wifi_password_logged_out) > 0)
        {
            strncpy(app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out, app->wifi_password_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->wifi_password_logged_out_temp_buffer, "", app->wifi_password_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_wifi_settings_password, "Enter Password");
        uart_text_input_set_result_callback(app->text_input_logged_out_wifi_settings_password, flip_social_logged_out_wifi_settings_password_updated, app, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsPasswordInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their username on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_out_login_username_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->login_username_logged_out, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->login_username_logged_out[app->login_username_logged_out_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_out_login_username)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_login_username, app->login_username_logged_out);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLogin);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */

static void flip_social_logged_out_login_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->login_password_logged_out, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->login_password_logged_out[app->login_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_login_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_login_password, app->login_password_logged_out);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLogin);
}

/**
 * @brief Callback when the user selects a menu item in the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
static void flip_social_text_input_logged_out_login_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input Username
        // Initialize temp_buffer with the current name
        if (app->login_username_logged_out && strlen(app->login_username_logged_out) > 0)
        {
            strncpy(app->login_username_logged_out_temp_buffer, app->login_username_logged_out, app->login_username_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->login_username_logged_out_temp_buffer, "", app->login_username_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_login_username, "Enter Username");
        uart_text_input_set_result_callback(app->text_input_logged_out_login_username, flip_social_logged_out_login_username_updated, app, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginUsernameInput);
        break;
    case 1: // Input Password
        // Initialize temp_buffer with the current password
        if (app->login_password_logged_out && strlen(app->login_password_logged_out) > 0)
        {
            strncpy(app->login_password_logged_out_temp_buffer, app->login_password_logged_out, app->login_password_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->login_password_logged_out_temp_buffer, "", app->login_password_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_login_password, "Enter Password");
        uart_text_input_set_result_callback(app->text_input_logged_out_login_password, flip_social_logged_out_login_password_updated, app, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginPasswordInput);
        break;
    case 2: // Login Button
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutProcessLogin);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their username on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_out_register_username_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->register_username_logged_out, app->register_username_logged_out_temp_buffer, app->register_username_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->register_username_logged_out[app->register_username_logged_out_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_out_register_username)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_register_username, app->register_username_logged_out);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_out_register_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->register_password_logged_out, app->register_password_logged_out_temp_buffer, app->register_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->register_password_logged_out[app->register_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_register_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_register_password, app->register_password_logged_out);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
}

/**
 * @brief Text input callback for when the user finishes entering their password 2 on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_out_register_password_2_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->register_password_2_logged_out, app->register_password_2_logged_out_temp_buffer, app->register_password_2_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->register_password_2_logged_out[app->register_password_2_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_register_password_2)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_register_password_2, app->register_password_2_logged_out);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
}

/**
 * @brief Callback when the user selects a menu item in the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
static void flip_social_text_input_logged_out_register_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input Username
        // Initialize temp_buffer with the current name
        if (app->register_username_logged_out && strlen(app->register_username_logged_out) > 0)
        {
            strncpy(app->register_username_logged_out_temp_buffer, app->register_username_logged_out, app->register_username_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->register_username_logged_out_temp_buffer, "", app->register_username_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_register_username, "Enter Username");
        uart_text_input_set_result_callback(app->text_input_logged_out_register_username, flip_social_logged_out_register_username_updated, app, app->register_username_logged_out_temp_buffer, app->register_username_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterUsernameInput);
        break;
    case 1: // Input Password
        // Initialize temp_buffer with the current password
        if (app->register_password_logged_out && strlen(app->register_password_logged_out) > 0)
        {
            strncpy(app->register_password_logged_out_temp_buffer, app->register_password_logged_out, app->register_password_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->register_password_logged_out_temp_buffer, "", app->register_password_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_register_password, "Enter Password");
        uart_text_input_set_result_callback(app->text_input_logged_out_register_password, flip_social_logged_out_register_password_updated, app, app->register_password_logged_out_temp_buffer, app->register_password_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterPasswordInput);
        break;
    case 2: // Input Password 2
        // Initialize temp_buffer with the current password
        if (app->register_password_2_logged_out && strlen(app->register_password_2_logged_out) > 0)
        {
            strncpy(app->register_password_2_logged_out_temp_buffer, app->register_password_2_logged_out, app->register_password_2_logged_out_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->register_password_2_logged_out_temp_buffer, "", app->register_password_2_logged_out_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_out_register_password_2, "Enter Password Again");
        uart_text_input_set_result_callback(app->text_input_logged_out_register_password_2, flip_social_logged_out_register_password_2_updated, app, app->register_password_2_logged_out_temp_buffer, app->register_password_2_logged_out_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterPassword2Input);
        break;
    case 3: // Register button
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutProcessRegister);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_in_wifi_settings_ssid_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->wifi_ssid_logged_in[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_in_wifi_settings_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_ssid, app->wifi_ssid_logged_in);
    }

    // update the wifi settings
    if (!flipper_http_save_wifi(app->wifi_ssid_logged_in, app->wifi_password_logged_in))
    {
        FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsWifi);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_in_wifi_settings_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->wifi_password_logged_in[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_in_wifi_settings_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_password, app->wifi_password_logged_in);
    }

    // update the wifi settings
    if (!flipper_http_save_wifi(app->wifi_ssid_logged_in, app->wifi_password_logged_in))
    {
        FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsWifi);
}

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
static void flip_social_text_input_logged_in_wifi_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input SSID
        // Initialize temp_buffer with the current name
        if (app->wifi_ssid_logged_in && strlen(app->wifi_ssid_logged_in) > 0)
        {
            strncpy(app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->wifi_ssid_logged_in_temp_buffer, "", app->wifi_ssid_logged_in_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_in_wifi_settings_ssid, "Enter SSID");
        uart_text_input_set_result_callback(app->text_input_logged_in_wifi_settings_ssid, flip_social_logged_in_wifi_settings_ssid_updated, app, app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsSSIDInput);
        break;
    case 1: // Input Password
        // Initialize temp_buffer with the current password
        if (app->wifi_password_logged_in && strlen(app->wifi_password_logged_in) > 0)
        {
            strncpy(app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->wifi_password_logged_in_temp_buffer, "", app->wifi_password_logged_in_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_in_wifi_settings_password, "Enter Password");
        uart_text_input_set_result_callback(app->text_input_logged_in_wifi_settings_password, flip_social_logged_in_wifi_settings_password_updated, app, app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_in_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsPasswordInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their message on the compose (logged in) screen for Add Text
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_in_compose_pre_save_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty or if adding in the message would exceed the MAX_PRE_SAVED_MESSAGES
    if (app->compose_pre_save_logged_in_temp_buffer_size == 0 || app->pre_saved_messages.count >= MAX_PRE_SAVED_MESSAGES)
    {
        FURI_LOG_E(TAG, "Message is empty or would exceed the maximum number of pre-saved messages");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInCompose);
        return;
    }

    // Store the entered message
    strncpy(app->compose_pre_save_logged_in, app->compose_pre_save_logged_in_temp_buffer, app->compose_pre_save_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->compose_pre_save_logged_in[app->compose_pre_save_logged_in_temp_buffer_size - 1] = '\0';

    // add the item to the submenu
    submenu_reset(app->submenu_compose);

    // loop through the items and add them to the submenu
    app->pre_saved_messages.messages[app->pre_saved_messages.count] = app->compose_pre_save_logged_in;
    app->pre_saved_messages.count++;

    submenu_add_item(app->submenu_compose, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, flip_social_callback_submenu_choices, app);
    for (uint32_t i = 0; i < app->pre_saved_messages.count; i++)
    {
        submenu_add_item(app->submenu_compose, app->pre_saved_messages.messages[i], FlipSocialSubemnuComposeIndexStartIndex + i, flip_social_callback_submenu_choices, app);
    }

    // save playlist
    save_playlist(&app->pre_saved_messages);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInCompose);
}

/**
 * @brief Text input callback for when the user finishes entering their message on the profile (logged in) screen for change password
 * @param context The context - FlipSocialApp object.
 * @return void
 */
static void flip_social_logged_in_profile_change_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Correct type: old_pass should be a pointer to a string (char *)
    const char *old_password = app->login_password_logged_out;

    // Store the entered message
    strncpy(app->change_password_logged_in, app->change_password_logged_in_temp_buffer, app->change_password_logged_in_temp_buffer_size);

    // store the entered password in the logged out password
    strncpy(app->login_password_logged_out, app->change_password_logged_in, app->login_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->change_password_logged_in[app->change_password_logged_in_temp_buffer_size - 1] = '\0';

    // Update the message item text
    if (app->variable_item_logged_in_profile_change_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_profile_change_password, app->change_password_logged_in);
    }

    // send post request to change password
    char payload[256];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"old_password\":\"%s\",\"new_password\":\"%s\"}", app->login_username_logged_out, old_password, app->change_password_logged_in);

    if (!flipper_http_post_request_with_headers("https://www.flipsocial.net/api/change-password/", "Content-Type: application/json", payload))
    {
        FURI_LOG_E(TAG, "Failed to send post request to change password");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        return;
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInProfile);
}

/**
 * @brief Callback when a user selects a menu item in the profile (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
static void flip_social_text_input_logged_in_profile_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Change Username
        // do nothing since username cannot be changed
        break;
    case 1: // Change Password
        // Initialize temp_buffer with the current password
        if (app->change_password_logged_in && strlen(app->change_password_logged_in) > 0)
        {
            strncpy(app->change_password_logged_in_temp_buffer, app->change_password_logged_in, app->change_password_logged_in_temp_buffer_size - 1);
        }
        else
        {
            strncpy(app->change_password_logged_in_temp_buffer, "", app->change_password_logged_in_temp_buffer_size - 1);
        }
        uart_text_input_set_header_text(app->text_input_logged_in_change_password, "Enter New Password");
        uart_text_input_set_result_callback(app->text_input_logged_in_change_password, flip_social_logged_in_profile_change_password_updated, app, app->change_password_logged_in_temp_buffer, app->change_password_logged_in_temp_buffer_size, false);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInChangePasswordInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Callback when a user selects a menu item in the settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
static void flip_social_text_input_logged_in_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // About
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsAbout);
        break;
    case 1: // Wifi
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsWifi);
        break;
    default:
        break;
    }
}

#endif // FLIP_SOCIAL_CALLBACK_H