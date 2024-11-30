#include "flip_social_draw.h"
Action action = ActionNone;
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

// Function to draw the message on the canvas with word wrapping
void draw_user_message(Canvas *canvas, const char *user_message, int x, int y)
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

    while (start < msg_length && line_num < 4)
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
        }

        // Copy the substring to 'line' and null-terminate it
        memcpy(line, user_message + start, len);
        line[len] = '\0'; // Ensure the string is null-terminated

        // Draw the string on the canvas
        // Adjust the y-coordinate based on the line number
        canvas_draw_str_aligned(canvas, x, y + line_num * 10, AlignLeft, AlignTop, line);

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
}

void flip_social_callback_draw_compose(Canvas *canvas, void *model)
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
    if (!selected_message)
    {
        FURI_LOG_E(TAG, "Selected message is NULL");
        return;
    }

    if (strlen(selected_message) > MAX_MESSAGE_LENGTH)
    {
        FURI_LOG_E(TAG, "Message is too long");
        return;
    }

    if (!flip_social_dialog_shown)
    {
        flip_social_dialog_shown = true;
        app_instance->input_event_queue = furi_record_open(RECORD_INPUT_EVENTS);
        app_instance->input_event = furi_pubsub_subscribe(app_instance->input_event_queue, on_input, NULL);
        auth_headers_alloc();
    }

    draw_user_message(canvas, selected_message, 0, 2);

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
        // send selected_message
        if (selected_message && app_instance->login_username_logged_in)
        {
            if (strlen(selected_message) > MAX_MESSAGE_LENGTH)
            {
                FURI_LOG_E(TAG, "Message is too long");
                return;
            }
            // Send the selected_message
            char command[256];
            snprintf(command, sizeof(command), "{\"username\":\"%s\",\"content\":\"%s\"}",
                     app_instance->login_username_logged_in, selected_message);
            if (!flipper_http_post_request_with_headers(
                    "https://www.flipsocial.net/api/feed/post/",
                    auth_headers,
                    command))
            {
                FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
                fhttp.state = ISSUE;
                return;
            }

            fhttp.state = RECEIVING;
            furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        }
        else
        {
            FURI_LOG_E(TAG, "Message or username is NULL");
            return;
        }
        while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
        {
            // Wait for the feed to be received
            furi_delay_ms(100);

            // Draw the resulting string on the canvas
            canvas_draw_str(canvas, 0, 30, "Receiving..");
        }
        flip_social_dialog_stop = true;
        furi_timer_stop(fhttp.get_timeout_timer);
        break;
    case ActionPrev:
        // delete message
        app_instance->pre_saved_messages.messages[app_instance->pre_saved_messages.index] = NULL;

        for (uint32_t i = app_instance->pre_saved_messages.index; i < app_instance->pre_saved_messages.count - 1; i++)
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
        if (action == ActionNext)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "Sent successfully!");
            canvas_draw_str(canvas, 0, 50, "Loading feed :D");
            canvas_draw_str(canvas, 0, 60, "Please wait...");
            action = ActionNone;
            // Send the user to the feed

            if (!easy_flipper_set_loading(&app_instance->loading, FlipSocialViewLoading, flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                FURI_LOG_E(TAG, "Failed to set loading screen");
                return; // already on the submenu
            }
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoading);
            if (flip_social_get_feed()) // start the async request
            {
                furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
                fhttp.state = RECEIVING;
            }
            else
            {
                FURI_LOG_E(HTTP_TAG, "Failed to send request");
                fhttp.state = ISSUE;
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
                view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoading);
                loading_free(app_instance->loading);
                return;
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
                fhttp.state = ISSUE;
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
                view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoading);
                loading_free(app_instance->loading);
                return;
            }

            // load the current feed post
            if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
            {
                FURI_LOG_E(TAG, "Failed to load feed post");
                fhttp.state = ISSUE;
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
                view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoading);
                loading_free(app_instance->loading);
                return;
            }
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
            view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoading);
            loading_free(app_instance->loading);
        }
        else if (action == ActionBack)
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
void flip_social_canvas_draw_message(Canvas *canvas, char *user_username, char *user_message, bool is_flipped, bool show_prev, bool show_next, int flip_count)
{
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, user_username);
    canvas_set_font(canvas, FontSecondary);

    char flip_count_str[12];
    if (flip_count == 1)
    {
        snprintf(flip_count_str, sizeof(flip_count_str), "%d Flip", flip_count);
        canvas_draw_str_aligned(canvas, 106, 54, AlignLeft, AlignTop, flip_count_str);
    }
    else
    {
        snprintf(flip_count_str, sizeof(flip_count_str), "%d Flips", flip_count);

        if (flip_count < 10)
        {
            canvas_draw_str_aligned(canvas, 100, 54, AlignLeft, AlignTop, flip_count_str);
        }
        else if (flip_count < 100)
        {
            canvas_draw_str_aligned(canvas, 94, 54, AlignLeft, AlignTop, flip_count_str);
        }
        else
        {
            canvas_draw_str_aligned(canvas, 88, 54, AlignLeft, AlignTop, flip_count_str);
        }
    }

    draw_user_message(canvas, user_message, 0, 12);

    // combine and shift icons/labels around if not show_prev or show_next
    if (show_prev && show_next && !is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 6, 54, AlignLeft, AlignTop, "Prev");
        canvas_draw_icon(canvas, 30, 54, &I_ButtonRight_4x7);
        canvas_draw_str_aligned(canvas, 36, 54, AlignLeft, AlignTop, "Next");
        canvas_draw_icon(canvas, 58, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 67, 54, AlignLeft, AlignTop, "Flip");
    }
    else if (show_prev && !show_next && !is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 6, 54, AlignLeft, AlignTop, "Prev");
        canvas_draw_icon(canvas, 28, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 37, 54, AlignLeft, AlignTop, "Flip");
    }
    else if (!show_prev && show_next && !is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonRight_4x7);
        canvas_draw_str_aligned(canvas, 6, 54, AlignLeft, AlignTop, "Next");
        canvas_draw_icon(canvas, 28, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 37, 54, AlignLeft, AlignTop, "Flip");
    }
    else if (show_prev && show_next && is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 6, 54, AlignLeft, AlignTop, "Prev");
        canvas_draw_icon(canvas, 28, 54, &I_ButtonRight_4x7);
        canvas_draw_str_aligned(canvas, 34, 54, AlignLeft, AlignTop, "Next");
        canvas_draw_icon(canvas, 54, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 63, 54, AlignLeft, AlignTop, "UnFlip");
    }
    else if (show_prev && !show_next && is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 6, 54, AlignLeft, AlignTop, "Prev");
        canvas_draw_icon(canvas, 28, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 37, 54, AlignLeft, AlignTop, "UnFlip");
    }
    else if (!show_prev && show_next && is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonRight_4x7);
        canvas_draw_str_aligned(canvas, 6, 54, AlignLeft, AlignTop, "Next");
        canvas_draw_icon(canvas, 28, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 37, 54, AlignLeft, AlignTop, "UnFlip");
    }
    else if (!show_prev && !show_next && is_flipped)
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 9, 54, AlignLeft, AlignTop, "UnFlip");
    }
    else
    {
        canvas_draw_icon(canvas, 0, 54, &I_ButtonOK_7x7);
        canvas_draw_str_aligned(canvas, 9, 54, AlignLeft, AlignTop, "Flip");
    }
}
// Callback function to handle the feed dialog
void flip_social_callback_draw_feed(Canvas *canvas, void *model)
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
        auth_headers_alloc();
    }

    // handle action
    switch (action)
    {
    case ActionNone:
        flip_social_canvas_draw_message(canvas, flip_feed_item->username, flip_feed_item->message, flip_feed_item->is_flipped, flip_feed_info->index > 0, flip_feed_info->index < flip_feed_info->count - 1, flip_feed_item->flips);
        break;
    case ActionNext:
        canvas_clear(canvas);
        if (flip_feed_info->index < flip_feed_info->count - 1)
        {
            flip_feed_info->index++;
        }
        // load the next feed item
        if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            fhttp.state = ISSUE;
            return;
        }
        flip_social_canvas_draw_message(canvas, flip_feed_item->username, flip_feed_item->message, flip_feed_item->is_flipped, flip_feed_info->index > 0, flip_feed_info->index < flip_feed_info->count - 1, flip_feed_item->flips);
        action = ActionNone;
        break;
    case ActionPrev:
        canvas_clear(canvas);
        if (flip_feed_info->index > 0)
        {
            flip_feed_info->index--;
        }
        // load the previous feed item
        if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            fhttp.state = ISSUE;
            return;
        }
        flip_social_canvas_draw_message(canvas, flip_feed_item->username, flip_feed_item->message, flip_feed_item->is_flipped, flip_feed_info->index > 0, flip_feed_info->index < flip_feed_info->count - 1, flip_feed_item->flips);
        action = ActionNone;
        break;
    case ActionFlip:
        canvas_clear(canvas);
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
            return;
        }
        char payload[256];
        snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"post_id\":\"%u\"}", app_instance->login_username_logged_in, flip_feed_item->id);
        flipper_http_post_request_with_headers("https://www.flipsocial.net/api/feed/flip/", auth_headers, payload);
        flip_social_canvas_draw_message(canvas, flip_feed_item->username, flip_feed_item->message, flip_feed_item->is_flipped, flip_feed_info->index > 0, flip_feed_info->index < flip_feed_info->count - 1, flip_feed_item->flips);
        action = ActionNone;
        break;
    case ActionBack:
        canvas_clear(canvas);
        flip_social_dialog_stop = true;
        flip_feed_info->index = 0;
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

void flip_social_canvas_draw_user_message(Canvas *canvas, char *user_username, char *user_message, bool show_prev, bool show_next)
{
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignCenter, user_username);
    canvas_set_font(canvas, FontSecondary);

    draw_user_message(canvas, user_message, 0, 12);

    canvas_set_font(canvas, FontSecondary);
    if (show_prev)
    {
        canvas_draw_icon(canvas, 0, 53, &I_ButtonLeft_4x7);
        canvas_draw_str_aligned(canvas, 9, 54, AlignLeft, AlignTop, "Prev");
    }

    canvas_draw_icon(canvas, 47, 53, &I_ButtonOK_7x7);
    canvas_draw_str_aligned(canvas, 56, 54, AlignLeft, AlignTop, "Create");

    if (show_next)
    {
        canvas_draw_icon(canvas, 98, 53, &I_ButtonRight_4x7);
        canvas_draw_str_aligned(canvas, 107, 54, AlignLeft, AlignTop, "Next");
    }
}

// Callback function to handle the messages dialog
void flip_social_callback_draw_messages(Canvas *canvas, void *model)
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
        flip_social_canvas_draw_user_message(canvas, flip_social_messages->usernames[flip_social_messages->index], flip_social_messages->messages[flip_social_messages->index], flip_social_messages->index > 0, flip_social_messages->index < flip_social_messages->count - 1);
        action = ActionNone;
        break;
    case ActionNext:
        // view next message (if any)
        canvas_clear(canvas);
        if (flip_social_messages->index < flip_social_messages->count - 1)
        {
            flip_social_messages->index++;
        }
        flip_social_canvas_draw_user_message(canvas, flip_social_messages->usernames[flip_social_messages->index], flip_social_messages->messages[flip_social_messages->index], flip_social_messages->index > 0, flip_social_messages->index < flip_social_messages->count - 1);
        action = ActionNone;
        break;
    case ActionPrev:
        // view previous message (if any)
        canvas_clear(canvas);
        if (flip_social_messages->index > 0)
        {
            flip_social_messages->index--;
        }
        flip_social_canvas_draw_user_message(canvas, flip_social_messages->usernames[flip_social_messages->index], flip_social_messages->messages[flip_social_messages->index], flip_social_messages->index > 0, flip_social_messages->index < flip_social_messages->count - 1);
        action = ActionNone;
        break;
    case ActionBack:
        //  go back to the previous view
        flip_social_dialog_stop = true;
        action = ActionNone;
        break;
    case ActionFlip:
        // go to the input view
        flip_social_dialog_stop = true;
        flip_social_send_message = true;
        action = ActionNone;
        break;
    default:
        action = ActionNone;
        break;
    }

    if (flip_social_dialog_stop && flip_social_dialog_shown)
    {
        furi_pubsub_unsubscribe(app_instance->input_event_queue, app_instance->input_event);
        flip_social_dialog_shown = false;
        flip_social_dialog_stop = false;
        if (flip_social_send_message)
        {
            FURI_LOG_I(TAG, "Switching to new message input view");
            action = ActionNone;
            flip_social_send_message = false;
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageInput);
        }
        else
        {
            action = ActionNone;
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInMessagesSubmenu);
        }
    }
}