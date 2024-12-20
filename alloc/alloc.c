#include <alloc/alloc.h>
bool went_to_friends = false;
void auth_headers_alloc(void)
{
    if (!app_instance)
    {
        snprintf(auth_headers, sizeof(auth_headers), "{\"Content-Type\":\"application/json\"}");
        return;
    }

    if (app_instance->login_username_logged_out && app_instance->login_password_logged_out && strlen(app_instance->login_username_logged_out) > 0 && strlen(app_instance->login_password_logged_out) > 0)
    {
        snprintf(auth_headers, sizeof(auth_headers), "{\"Content-Type\":\"application/json\",\"username\":\"%s\",\"password\":\"%s\"}", app_instance->login_username_logged_out, app_instance->login_password_logged_out);
    }
    else if (app_instance->login_username_logged_in && app_instance->change_password_logged_in && strlen(app_instance->login_username_logged_in) > 0 && strlen(app_instance->change_password_logged_in) > 0)
    {
        snprintf(auth_headers, sizeof(auth_headers), "{\"Content-Type\":\"application/json\",\"username\":\"%s\",\"password\":\"%s\"}", app_instance->login_username_logged_in, app_instance->change_password_logged_in);
    }
    else
    {
        snprintf(auth_headers, sizeof(auth_headers), "{\"Content-Type\":\"application/json\"}");
    }
}

FlipSocialFeedMini *flip_feed_info_alloc(void)
{
    FlipSocialFeedMini *feed_info = (FlipSocialFeedMini *)malloc(sizeof(FlipSocialFeedMini));
    if (!feed_info)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for feed_info");
        return NULL;
    }
    feed_info->count = 0;
    feed_info->index = 0;
    return feed_info;
}
bool messages_dialog_alloc(bool free_first)
{
    if (free_first)
    {
        flip_social_free_messages_dialog();
    }
    if (!app_instance->dialog_messages)
    {
        if (!easy_flipper_set_dialog_ex(
                &app_instance->dialog_messages,
                FlipSocialViewMessagesDialog,
                flip_social_messages->usernames[flip_social_messages->index],
                0,
                0,
                updated_user_message(flip_social_messages->messages[flip_social_messages->index]),
                0,
                10,
                flip_social_messages->index != 0 ? "Prev" : NULL,
                flip_social_messages->index != flip_social_messages->count - 1 ? "Next" : NULL,
                "Create",
                messages_dialog_callback,
                flip_social_callback_to_messages_logged_in,
                &app_instance->view_dispatcher,
                app_instance))
        {
            return false;
        }
        return true;
    }
    return false;
}
char *updated_user_message(const char *user_message)
{
    if (user_message == NULL)
    {
        FURI_LOG_E(TAG, "User message is NULL.");
        return NULL;
    }

    size_t msg_length = strlen(user_message);
    size_t start = 0;
    int line_num = 0;

    // Allocate memory for the updated message
    char *updated_message = malloc(MAX_MESSAGE_LENGTH + 10);
    if (updated_message == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for updated_message.");
        return NULL;
    }
    size_t current_pos = 0;    // Tracks the current position in updated_message
    updated_message[0] = '\0'; // Initialize as empty string

    while (start < msg_length && line_num < 4)
    {
        size_t remaining = msg_length - start;
        size_t len = (remaining > MAX_LINE_LENGTH) ? MAX_LINE_LENGTH : remaining;

        // Adjust length to the last space if the line exceeds MAX_LINE_LENGTH
        if (remaining > MAX_LINE_LENGTH)
        {
            size_t last_space = len;
            while (last_space > 0 && user_message[start + last_space - 1] != ' ')
            {
                last_space--;
            }

            if (last_space > 0)
            {
                len = last_space; // Adjust len to the position of the last space
            }
        }

        // Check if the new line fits in the updated_message buffer
        if (current_pos + len + 1 >= (MAX_MESSAGE_LENGTH + 10))
        {
            FURI_LOG_E(TAG, "Updated message exceeds maximum length.");
            // break and return what we have so far
            break;
        }

        // Copy the line and append a newline character
        memcpy(updated_message + current_pos, user_message + start, len);
        current_pos += len;
        updated_message[current_pos++] = '\n'; // Append newline

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

    // Null-terminate the final string
    if (current_pos < (MAX_MESSAGE_LENGTH + 10))
    {
        updated_message[current_pos] = '\0';
    }
    else
    {
        FURI_LOG_E(TAG, "Buffer overflow while null-terminating.");
        free(updated_message);
        return NULL;
    }

    return updated_message;
}

typedef enum
{
    ActionNone,
    ActionBack,
    ActionNext,
    ActionPrev,
    ActionFlip,
} Action;

static Action action = ActionNone;

void on_input(const void *event, void *ctx)
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

// Make sure to define a suitable MAX_LINE_LENGTH
// For example:

#define MAX_LINES 4

static void draw_user_message(Canvas *canvas, const char *user_message, int x, int y)
{
    if (!user_message)
    {
        FURI_LOG_E(TAG, "User message is NULL.");
        return;
    }

    // We'll copy the user_message into a buffer we can safely tokenize
    char buffer[256]; // Adjust size if needed
    strncpy(buffer, user_message, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *token = strtok(buffer, " ");
    char line[MAX_LINE_LENGTH + 1];
    size_t line_len = 0;
    int line_num = 0;

    // Clear the line buffer
    line[0] = '\0';

    while (token && line_num < MAX_LINES)
    {
        size_t token_len = strlen(token);

        // If it's the first word in a line, add it directly if it fits
        if (line_len == 0)
        {
            if (token_len <= MAX_LINE_LENGTH)
            {
                strcpy(line, token);
                line_len = token_len;
            }
            else
            {
                // Single word longer than MAX_LINE_LENGTH; we must truncate or skip it
                // Truncation logic: Just take as many chars as fit.
                strncpy(line, token, MAX_LINE_LENGTH);
                line[MAX_LINE_LENGTH] = '\0';
                line_len = MAX_LINE_LENGTH;
            }
        }
        else
        {
            // Check if adding this word plus a space would exceed MAX_LINE_LENGTH
            if (line_len + 1 + token_len <= MAX_LINE_LENGTH)
            {
                // Append " space + token"
                line[line_len] = ' ';
                strcpy(&line[line_len + 1], token);
                line_len += 1 + token_len;
            }
            else
            {
                // Current word doesn't fit in this line
                // Draw the current line first
                canvas_draw_str_aligned(canvas, x, y + line_num * 10, AlignLeft, AlignTop, line);
                line_num++;

                // Start a new line with the current word (or truncated if too long)
                if (line_num < MAX_LINES)
                {
                    if (token_len <= MAX_LINE_LENGTH)
                    {
                        strcpy(line, token);
                        line_len = token_len;
                    }
                    else
                    {
                        strncpy(line, token, MAX_LINE_LENGTH);
                        line[MAX_LINE_LENGTH] = '\0';
                        line_len = MAX_LINE_LENGTH;
                    }
                }
            }
        }

        token = strtok(NULL, " ");
    }

    // Draw any remaining text in the buffer if we still have space
    if (line_len > 0 && line_num < MAX_LINES)
    {
        canvas_draw_str_aligned(canvas, x, y + line_num * 10, AlignLeft, AlignTop, line);
    }
}

static void flip_social_feed_draw_callback(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_clear(canvas);
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    canvas_draw_str(canvas, 0, 7, flip_feed_item->username);
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
    draw_user_message(canvas, flip_feed_item->message, 0, 12);
    canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
    char flip_message[32];
    snprintf(flip_message, sizeof(flip_message), "%u %s", flip_feed_item->flips, flip_feed_item->flips == 1 ? "flip" : "flips");
    canvas_draw_str(canvas, 0, 60, flip_message);                  // Draw the number of flips
    canvas_draw_str(canvas, 64, 60, flip_feed_item->date_created); // Draw the date
}

static bool flip_social_feed_input_callback(InputEvent *event, void *context)
{
    UNUSED(context);
    furi_assert(app_instance);

    // if back button is pressed
    if (event->type == InputTypePress && event->key == InputKeyBack)
    {
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        return true;
    }

    if (event->type == InputTypePress && event->key == InputKeyLeft) // Previous message
    {
        if (flip_feed_info->index > 0)
        {
            flip_feed_info->index--;
        }
        // switch view, free dialog, re-alloc dialog, switch back to dialog
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewWidgetResult);
        flip_social_free_feed_view();
        // load feed item
        if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            fhttp.state = ISSUE;
            return false;
        }
        if (feed_view_alloc())
        {
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to allocate feed dialog");
            fhttp.state = ISSUE;
            return false;
        }
    }
    else if (event->type == InputTypePress && event->key == InputKeyRight) // Next message
    {
        if (flip_feed_info->index < flip_feed_info->count - 1)
        {
            flip_feed_info->index++;
        }
        // switch view, free dialog, re-alloc dialog, switch back to dialog
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewWidgetResult);
        flip_social_free_feed_view();
        // load feed item
        if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            fhttp.state = ISSUE;
            return false;
        }
        if (feed_view_alloc())
        {
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to allocate feed dialog");
            fhttp.state = ISSUE;
            return false;
        }
    }
    else if (event->type == InputTypePress && event->key == InputKeyOk) // Flip/Unflip
    {
        // Moved to above the is_flipped check
        if (!flip_feed_item->is_flipped)
        {
            // increase the flip count
            flip_feed_item->flips++;
        }
        else
        {
            // decrease the flip count
            flip_feed_item->flips--;
        }
        // change the flip status
        flip_feed_item->is_flipped = !flip_feed_item->is_flipped;
        // send post request to flip the message
        if (app_instance->login_username_logged_in == NULL)
        {
            FURI_LOG_E(TAG, "Username is NULL");
            return false;
        }
        if (!flipper_http_init(flipper_http_rx_callback, app_instance))
        {
            FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
            return false;
        }
        auth_headers_alloc();
        char payload[256];
        snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"post_id\":\"%u\"}", app_instance->login_username_logged_in, flip_feed_item->id);
        if (flipper_http_post_request_with_headers("https://www.flipsocial.net/api/feed/flip/", auth_headers, payload))
        {
            // save feed item
            char new_save[256];
            snprintf(new_save, sizeof(new_save), "{\"id\":%u,\"username\":\"%s\",\"message\":\"%s\",\"flip_count\":%u,\"flipped\":%s}",
                     flip_feed_item->id, flip_feed_item->username, flip_feed_item->message, flip_feed_item->flips, flip_feed_item->is_flipped ? "true" : "false");
            // if (!flip_social_save_post((char *)flip_feed_item->id, new_save))
            // {
            //     FURI_LOG_E(TAG, "Failed to save the feed post");
            //     fhttp.state = ISSUE;
            //     return false;
            // }
        }
        // switch view, free dialog, re-alloc dialog, switch back to dialog
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewWidgetResult);
        flip_social_free_feed_view();
        if (feed_view_alloc())
        {
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to allocate feed dialog");
        }
        furi_delay_ms(1000);
        flipper_http_deinit();
    }
    return false;
}

bool feed_view_alloc()
{
    if (!app_instance)
    {
        return false;
    }
    if (!flip_feed_item)
    {
        FURI_LOG_E(TAG, "Feed item is NULL");
        return false;
    }
    flip_social_free_feed_view();
    if (!app_instance->view_feed)
    {
        if (!easy_flipper_set_view(
                &app_instance->view_feed,
                FlipSocialViewLoggedInFeed,
                flip_social_feed_draw_callback,
                flip_social_feed_input_callback,
                flip_social_callback_to_submenu_logged_in,
                &app_instance->view_dispatcher,
                app_instance))
        {
            return false;
        }
        return true;
    }
    return false;
}

bool alloc_text_input(uint32_t view_id)
{
    if (!app_instance)
    {
        return false;
    }
    if (!app_instance->text_input)
    {
        switch (view_id)
        {
        case FlipSocialViewLoggedOutWifiSettingsSSIDInput:
            // memset(app_instance->wifi_ssid_logged_out_temp_buffer, 0, app_instance->wifi_ssid_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter SSID", app_instance->wifi_ssid_logged_out_temp_buffer, app_instance->wifi_ssid_logged_out_temp_buffer_size, flip_social_logged_out_wifi_settings_ssid_updated, flip_social_callback_to_wifi_settings_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutWifiSettingsPasswordInput:
            // memset(app_instance->wifi_password_logged_out_temp_buffer, 0, app_instance->wifi_password_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->wifi_password_logged_out_temp_buffer, app_instance->wifi_password_logged_out_temp_buffer_size, flip_social_logged_out_wifi_settings_password_updated, flip_social_callback_to_wifi_settings_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutLoginUsernameInput:
            // memset(app_instance->login_username_logged_out_temp_buffer, 0, app_instance->login_username_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username", app_instance->login_username_logged_out_temp_buffer, app_instance->login_username_logged_out_temp_buffer_size, flip_social_logged_out_login_username_updated, flip_social_callback_to_login_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutLoginPasswordInput:
            // memset(app_instance->login_password_logged_out_temp_buffer, 0, app_instance->login_password_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->login_password_logged_out_temp_buffer, app_instance->login_password_logged_out_temp_buffer_size, flip_social_logged_out_login_password_updated, flip_social_callback_to_login_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterUsernameInput:
            memset(app_instance->register_username_logged_out_temp_buffer, 0, app_instance->register_username_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username", app_instance->register_username_logged_out_temp_buffer, app_instance->register_username_logged_out_temp_buffer_size, flip_social_logged_out_register_username_updated, flip_social_callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterPasswordInput:
            memset(app_instance->register_password_logged_out_temp_buffer, 0, app_instance->register_password_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->register_password_logged_out_temp_buffer, app_instance->register_password_logged_out_temp_buffer_size, flip_social_logged_out_register_password_updated, flip_social_callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterPassword2Input:
            memset(app_instance->register_password_2_logged_out_temp_buffer, 0, app_instance->register_password_2_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Confirm Password", app_instance->register_password_2_logged_out_temp_buffer, app_instance->register_password_2_logged_out_temp_buffer_size, flip_social_logged_out_register_password_2_updated, flip_social_callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInChangePasswordInput:
            // memset(app_instance->change_password_logged_in_temp_buffer, 0, app_instance->change_password_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Change Password", app_instance->change_password_logged_in_temp_buffer, app_instance->change_password_logged_in_temp_buffer_size, flip_social_logged_in_profile_change_password_updated, flip_social_callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInChangeBioInput:
            // memset(app_instance->change_bio_logged_in_temp_buffer, 0, app_instance->change_bio_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Bio", app_instance->change_bio_logged_in_temp_buffer, app_instance->change_bio_logged_in_temp_buffer_size, flip_social_logged_in_profile_change_bio_updated, flip_social_callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInComposeAddPreSaveInput:
            memset(app_instance->compose_pre_save_logged_in_temp_buffer, 0, app_instance->compose_pre_save_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Pre-Save Message", app_instance->compose_pre_save_logged_in_temp_buffer, app_instance->compose_pre_save_logged_in_temp_buffer_size, flip_social_logged_in_compose_pre_save_updated, flip_social_callback_to_compose_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInWifiSettingsSSIDInput:
            // memset(app_instance->wifi_ssid_logged_in_temp_buffer, 0, app_instance->wifi_ssid_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter SSID", app_instance->wifi_ssid_logged_in_temp_buffer, app_instance->wifi_ssid_logged_in_temp_buffer_size, flip_social_logged_in_wifi_settings_ssid_updated, flip_social_callback_to_wifi_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInWifiSettingsPasswordInput:
            // memset(app_instance->wifi_password_logged_in_temp_buffer, 0, app_instance->wifi_password_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->wifi_password_logged_in_temp_buffer, app_instance->wifi_password_logged_in_temp_buffer_size, flip_social_logged_in_wifi_settings_password_updated, flip_social_callback_to_wifi_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessagesNewMessageInput:
            memset(app_instance->messages_new_message_logged_in_temp_buffer, 0, app_instance->messages_new_message_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Message", app_instance->messages_new_message_logged_in_temp_buffer, app_instance->messages_new_message_logged_in_temp_buffer_size, flip_social_logged_in_messages_new_message_updated, flip_social_callback_to_messages_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput:
            memset(app_instance->message_user_choice_logged_in_temp_buffer, 0, app_instance->message_user_choice_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Message", app_instance->message_user_choice_logged_in_temp_buffer, app_instance->message_user_choice_logged_in_temp_buffer_size, flip_social_logged_in_messages_user_choice_message_updated, flip_social_callback_to_messages_user_choices, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInExploreInput:
            memset(app_instance->explore_logged_in_temp_buffer, 0, app_instance->explore_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username or Keyword", app_instance->explore_logged_in_temp_buffer, app_instance->explore_logged_in_temp_buffer_size, flip_social_logged_in_explore_updated, flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessageUsersInput:
            memset(app_instance->message_users_logged_in_temp_buffer, 0, app_instance->message_users_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username or Keyword", app_instance->message_users_logged_in_temp_buffer, app_instance->message_users_logged_in_temp_buffer_size, flip_social_logged_in_message_users_updated, flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        default:
            return false;
        }
    }
    return true;
}

bool about_widget_alloc(bool is_logged_in)
{
    if (!is_logged_in)
    {
        if (!app_instance->widget_logged_out_about)
        {
            return easy_flipper_set_widget(&app_instance->widget_logged_out_about, FlipSocialViewLoggedOutAbout, "Welcome to FlipSocial\n---\nThe social media app for\nFlipper Zero, created by\nJBlanked: www.flipsocial.net\n---\nPress BACK to return.", flip_social_callback_to_submenu_logged_out, &app_instance->view_dispatcher);
        }
    }
    else
    {
        if (!app_instance->widget_logged_in_about)
        {
            return easy_flipper_set_widget(&app_instance->widget_logged_in_about, FlipSocialViewLoggedInSettingsAbout, "Welcome to FlipSocial\n---\nThe social media app for\nFlipper Zero, created by\nJBlanked: www.flipsocial.net\n---\nPress BACK to return.", flip_social_callback_to_settings_logged_in, &app_instance->view_dispatcher);
        }
    }
    return true;
}

bool alloc_submenu(uint32_t view_id)
{
    if (!app_instance)
    {
        return false;
    }
    if (!app_instance->submenu)
    {
        switch (view_id)
        {
        case FlipSocialViewLoggedInSettings:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Settings", flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            submenu_add_item(app_instance->submenu, "About", FlipSocialSubmenuLoggedInIndexAbout, flip_social_callback_submenu_choices, app_instance);
            submenu_add_item(app_instance->submenu, "WiFi", FlipSocialSubmenuLoggedInIndexWifiSettings, flip_social_callback_submenu_choices, app_instance);
            break;
        case FlipSocialViewLoggedInCompose:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Create A Post", flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            submenu_add_item(app_instance->submenu, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, flip_social_callback_submenu_choices, app_instance);

            // Load the playlist
            if (load_playlist(&app_instance->pre_saved_messages))
            {
                // Update the playlist submenu
                for (uint32_t i = 0; i < app_instance->pre_saved_messages.count; i++)
                {
                    if (app_instance->pre_saved_messages.messages[i][0] != '\0') // Check if the string is not empty
                    {
                        submenu_add_item(app_instance->submenu, app_instance->pre_saved_messages.messages[i], FlipSocialSubemnuComposeIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
                    }
                }
            }
            break;
        case FlipSocialViewLoggedInFriendsSubmenu:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Friends", flip_social_callback_to_profile_logged_in, &app_instance->view_dispatcher))
            {
                FURI_LOG_E(TAG, "Failed to set submenu for friends");
                return false;
            }
            submenu_reset(app_instance->submenu);
            went_to_friends = true;
            break;
        case FlipSocialViewLoggedInMessagesUserChoices:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Users", flip_social_callback_to_messages_logged_in, &app_instance->view_dispatcher))
            {
                FURI_LOG_E(TAG, "Failed to set submenu for user choices");
                return false;
            }
            submenu_reset(app_instance->submenu);
            break;
        case FlipSocialViewLoggedInMessagesSubmenu:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Messages", flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            break;
        case FlipSocialViewLoggedInExploreSubmenu:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Explore", flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            break;
        }
    }
    return true;
}

bool alloc_variable_item_list(uint32_t view_id)
{
    if (!app_instance)
    {
        return false;
    }
    if (!app_instance->variable_item_list)
    {
        switch (view_id)
        {
        case FlipSocialViewLoggedOutWifiSettings:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, flip_social_text_input_logged_out_wifi_settings_item_selected, flip_social_callback_to_submenu_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_out_wifi_settings_ssid = variable_item_list_add(app_instance->variable_item_list, "SSID", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_wifi_settings_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            if (app_instance->wifi_ssid_logged_out)
                variable_item_set_current_value_text(app_instance->variable_item_logged_out_wifi_settings_ssid, app_instance->wifi_ssid_logged_out);
            return true;
        case FlipSocialViewLoggedOutLogin:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, flip_social_text_input_logged_out_login_item_selected, flip_social_callback_to_submenu_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_out_login_username = variable_item_list_add(app_instance->variable_item_list, "Username", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_login_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_login_button = variable_item_list_add(app_instance->variable_item_list, "Login", 0, NULL, app_instance);
            if (app_instance->login_username_logged_out)
                variable_item_set_current_value_text(app_instance->variable_item_logged_out_login_username, app_instance->login_username_logged_out);
            return true;
        case FlipSocialViewLoggedOutRegister:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, flip_social_text_input_logged_out_register_item_selected, flip_social_callback_to_submenu_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_out_register_username = variable_item_list_add(app_instance->variable_item_list, "Username", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_register_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_register_password_2 = variable_item_list_add(app_instance->variable_item_list, "Confirm Password", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_register_button = variable_item_list_add(app_instance->variable_item_list, "Register", 0, NULL, app_instance);
            return true;
        case FlipSocialViewLoggedInProfile:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, flip_social_text_input_logged_in_profile_item_selected, flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_in_profile_username = variable_item_list_add(app_instance->variable_item_list, "Username", 1, NULL, app_instance);
            app_instance->variable_item_logged_in_profile_change_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            app_instance->variable_item_logged_in_profile_change_bio = variable_item_list_add(app_instance->variable_item_list, "Bio", 1, NULL, app_instance);
            app_instance->variable_item_logged_in_profile_friends = variable_item_list_add(app_instance->variable_item_list, "Friends", 0, NULL, app_instance);
            if (app_instance->login_username_logged_in)
                variable_item_set_current_value_text(app_instance->variable_item_logged_in_profile_username, app_instance->login_username_logged_in);
            if (app_instance->change_bio_logged_in)
                variable_item_set_current_value_text(app_instance->variable_item_logged_in_profile_change_bio, app_instance->change_bio_logged_in);
            return true;
        case FlipSocialViewLoggedInSettingsWifi:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, flip_social_text_input_logged_in_wifi_settings_item_selected, flip_social_callback_to_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_in_wifi_settings_ssid = variable_item_list_add(app_instance->variable_item_list, "SSID", 1, NULL, app_instance);
            app_instance->variable_item_logged_in_wifi_settings_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            if (app_instance->wifi_ssid_logged_in)
                variable_item_set_current_value_text(app_instance->variable_item_logged_in_wifi_settings_ssid, app_instance->wifi_ssid_logged_in);
            return true;
        default:
            return false;
        }
    }
    return false;
}
