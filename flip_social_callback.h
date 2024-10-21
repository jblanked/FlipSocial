// flip_social_callback.h
#ifndef FLIP_SOCIAL_CALLBACK_H
#define FLIP_SOCIAL_CALLBACK_H
#include "flip_social_feed.h"

bool flip_social_sent_login_request = false;
bool flip_social_sent_register_request = false;
bool flip_social_login_success = false;
bool flip_social_register_success = false;
bool flip_social_dialog_shown = false;
bool flip_social_dialog_stop = false;

uint32_t flip_social_pre_saved_message_clicked_index = 0;
static void flip_social_logged_in_compose_pre_save_updated(void *context);
static void flip_social_callback_submenu_choices(void *context, uint32_t index);

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
        }

        // Copy the substring to 'line' and null-terminate it
        memcpy(line, user_message + start, len);
        line[len] = '\0'; // Ensure the string is null-terminated

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
            char command[128];
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

            char dots_str[64] = "Receiving";

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

    if (!flip_social_board_is_active(canvas))
    {
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
        if (flip_social_login_success)
        {
            fhttp.state = RECEIVING;
            return;
        }
        else
        {
            fhttp.state = ISSUE;
            return;
        }
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
            else
            {
                flip_social_handle_error(canvas);
            }
        }
        else if ((fhttp.state == ISSUE || fhttp.state == INACTIVE) && fhttp.received_data != NULL)
        {
            flip_social_handle_error(canvas);
        }
        else if (fhttp.state == IDLE && fhttp.received_data == NULL)
        {
            flip_social_handle_error(canvas);
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

    if (!flip_social_board_is_active(canvas))
    {
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

        char buffer[128];
        snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->register_username_logged_out, app_instance->register_password_logged_out);
        flip_social_register_success = flipper_http_post_request_with_headers("https://www.flipsocial.net/api/register/", "{\"Content-Type\":\"application/json\"}", buffer);

        flip_social_sent_register_request = true;
        if (flip_social_register_success)
        {
            // Set the state to RECEIVING to ensure we continue to see the receiving message
            fhttp.state = RECEIVING;
        }
        else
        {
            fhttp.state = ISSUE;
        }
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
                if (app_instance->login_username_logged_in)
                {
                    app_instance->login_username_logged_in = app_instance->register_username_logged_out;
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
            flip_social_handle_error(canvas);
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

    // Store the entered name in the logged in name field
    strncpy(app->wifi_ssid_logged_in, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size);
    strncpy(app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size);

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
    // Store the entered WiFi password
    strncpy(app->wifi_password_logged_out, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);

    // Store the entered WiFi password in the logged in password field
    strncpy(app->wifi_password_logged_in, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);
    strncpy(app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);

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
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsSSIDInput);
        break;
    case 1: // Input Password
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

    // Store the entered name in the logged in username field
    strncpy(app->login_username_logged_in, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size);
    strncpy(app->login_username_logged_in_temp_buffer, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size);

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

    // Store the entered password in the change password field
    strncpy(app->change_password_logged_in, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size);
    strncpy(app->change_password_logged_in_temp_buffer, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->login_password_logged_out[app->login_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_login_password)
    {
        // dont show the password on the screen (version 0.2)
        // variable_item_set_current_value_text(app->variable_item_logged_out_login_password, app->login_password_logged_out);
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
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginUsernameInput);
        break;
    case 1: // Input Password
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
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterUsernameInput);
        break;
    case 1: // Input Password
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterPasswordInput);
        break;
    case 2: // Input Password 2
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
    // Store the entered SSID
    strncpy(app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in_temp_buffer_size);

    // Store the entered SSID in the logged out SSID
    strncpy(app->wifi_ssid_logged_out, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size);
    strncpy(app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->wifi_ssid_logged_in[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_in_wifi_settings_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_ssid, app->wifi_ssid_logged_in);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_in, app_instance->wifi_password_logged_in, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_password_logged_in) > 0)
    {
        if (!flipper_http_save_wifi(app->wifi_ssid_logged_in, app->wifi_password_logged_in))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
    }

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

    // Store the entered password in the logged out password
    strncpy(app->login_password_logged_out, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size);
    strncpy(app->login_password_logged_out_temp_buffer, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->wifi_password_logged_in[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_in_wifi_settings_password)
    {
        // dont show the password on the screen (version 0.2)
        // variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_password, app->wifi_password_logged_in);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_in, app_instance->wifi_password_logged_in, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_password_logged_in) > 0)
    {
        if (!flipper_http_save_wifi(app->wifi_ssid_logged_in, app->wifi_password_logged_in))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
    }

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
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsSSIDInput);
        break;
    case 1: // Input Password
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
    strncpy(app->login_password_logged_out_temp_buffer, app->change_password_logged_in, app->login_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->change_password_logged_in[app->change_password_logged_in_temp_buffer_size - 1] = '\0';

    // Update the message item text
    if (app->variable_item_logged_in_profile_change_password)
    {
        // dont show the password on the screen (version 0.2)
        // variable_item_set_current_value_text(app->variable_item_logged_in_profile_change_password, app->change_password_logged_in);
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