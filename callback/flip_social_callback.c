#include <callback/flip_social_callback.h>

// Below added by Derek Jamison
// FURI_LOG_DEV will log only during app development. Be sure that Settings/System/Log Device is "LPUART"; so we dont use serial port.
#ifdef DEVELOPMENT
#define FURI_LOG_DEV(tag, format, ...) furi_log_print_format(FuriLogLevelInfo, tag, format, ##__VA_ARGS__)
#define DEV_CRASH() furi_crash()
#else
#define FURI_LOG_DEV(tag, format, ...)
#define DEV_CRASH()
#endif

static void flip_social_request_error_draw(Canvas *canvas, DataLoaderModel *model)
{
    if (canvas == NULL)
    {
        FURI_LOG_E(TAG, "error_draw - canvas is NULL");
        DEV_CRASH();
        return;
    }
    if (model->fhttp->last_response != NULL)
    {
        if (strstr(model->fhttp->last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(model->fhttp->last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(model->fhttp->last_response, "[ERROR] GET request failed or returned empty data.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] WiFi error.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(model->fhttp->last_response, "[PONG]") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[STATUS]Connecting to AP...");
        }
        else
        {
            canvas_clear(canvas);
            FURI_LOG_E(TAG, "Received an error: %s", model->fhttp->last_response);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Unusual error...");
            canvas_draw_str(canvas, 0, 60, "Press BACK and retry.");
        }
    }
    else
    {
        canvas_clear(canvas);
        canvas_draw_str(canvas, 0, 10, "[ERROR] Unknown error.");
        canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
        canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
    }
}

static bool flip_social_login_fetch(DataLoaderModel *model)
{
    UNUSED(model);
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!model->fhttp)
    {
        FURI_LOG_E(TAG, "fhttp is NULL");
        return false;
    }
    if (!app_instance->login_username_logged_out || !app_instance->login_password_logged_out || strlen(app_instance->login_username_logged_out) == 0 || strlen(app_instance->login_password_logged_out) == 0)
    {
        return false;
    }
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->login_username_logged_out, app_instance->login_password_logged_out);
    auth_headers_alloc();
    return flipper_http_request(model->fhttp, POST, "https://www.jblanked.com/flipper/api/user/login/", auth_headers, buffer);
}

static char *flip_social_login_parse(DataLoaderModel *model)
{
    UNUSED(model);
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return "Failed to login...";
    }
    if (!model->fhttp->last_response)
    {
        return "Failed to login...";
    }
    // read response
    if (strstr(model->fhttp->last_response, "[SUCCESS]") != NULL || strstr(model->fhttp->last_response, "User found") != NULL)
    {
        app_instance->is_logged_in = "true";

        // set the logged_in_username and change_password_logged_in
        if (app_instance->login_username_logged_out)
        {
            strcpy(app_instance->login_username_logged_in, app_instance->login_username_logged_out);
        }
        if (app_instance->login_password_logged_out)
        {
            strcpy(app_instance->change_password_logged_in, app_instance->login_password_logged_out);
        }

        save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

        // send user to the logged in submenu
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        return "Login successful!";
    }
    else if (strstr(model->fhttp->last_response, "User not found") != NULL)
    {
        return "Account not found...";
    }
    else
    {
        return "Failed to login...";
    }
}

static void flip_social_login_switch_to_view(FlipSocialApp *app)
{
    flip_social_generic_switch_to_view(app, "Logging in...", flip_social_login_fetch, flip_social_login_parse, 1, flip_social_callback_to_login_logged_out, FlipSocialViewLoader);
}

static bool flip_social_register_fetch(DataLoaderModel *model)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return "Failed to login...";
    }
    if (!model->fhttp->last_response)
    {

        return "Failed to login...";
    }
    // check if the username and password are valid
    if (!app_instance->register_username_logged_out || !app_instance->register_password_logged_out || strlen(app_instance->register_username_logged_out) == 0 || strlen(app_instance->register_password_logged_out) == 0)
    {
        FURI_LOG_E(TAG, "Username or password is NULL");
        return false;
    }

    // check if both passwords match
    if (strcmp(app_instance->register_password_logged_out, app_instance->register_password_2_logged_out) != 0)
    {
        FURI_LOG_E(TAG, "Passwords do not match");
        return false;
    }
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->register_username_logged_out, app_instance->register_password_logged_out);

    return flipper_http_request(model->fhttp, POST, "https://www.jblanked.com/flipper/api/user/register/", "{\"Content-Type\":\"application/json\"}", buffer);
}

static char *flip_social_register_parse(DataLoaderModel *model)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return "Failed to login...";
    }
    if (!model->fhttp->last_response)
    {
        return "Failed to login...";
    }
    // read response
    if (model->fhttp->last_response != NULL && (strstr(model->fhttp->last_response, "[SUCCESS]") != NULL || strstr(model->fhttp->last_response, "User created") != NULL))
    {
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

        // update header credentials
        auth_headers_alloc();

        // save the credentials
        save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

        // send user to the logged in submenu
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        return "Registration successful!\nWelcome to FlipSocial!";
    }
    else if (strstr(model->fhttp->last_response, "Username or password not provided") != NULL)
    {

        return "Please enter your credentials.\nPress BACK to return.";
    }
    else if (strstr(model->fhttp->last_response, "User already exists") != NULL || strstr(model->fhttp->last_response, "Multiple users found") != NULL)
    {

        return "Registration failed...\nUsername already exists.\nPress BACK to return.";
    }
    else
    {

        return "Registration failed...\nUpdate your credentials.\nPress BACK to return.";
    }
}

static void flip_social_register_switch_to_view(FlipSocialApp *app)
{
    flip_social_generic_switch_to_view(app, "Registering...", flip_social_register_fetch, flip_social_register_parse, 1, flip_social_callback_to_register_logged_out, FlipSocialViewLoader);
}

/**
 * @brief Navigation callback to go back to the submenu Logged out.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutSubmenu)
 */
uint32_t flip_social_callback_to_submenu_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedOutSubmenu;
}

/**
 * @brief Navigation callback to go back to the submenu Logged in.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSubmenu)
 */
uint32_t flip_social_callback_to_submenu_logged_in(void *context)
{
    UNUSED(context);
    free_about_widget(false);
    //
    flip_social_free_friends();
    flip_social_free_message_users();
    flip_feed_info_free();
    if (flip_feed_item)
    {
        free(flip_feed_item);
        flip_feed_item = NULL;
    }
    // free the about widget if it exists
    free_about_widget(true);
    flip_social_free_explore_dialog();
    flip_social_free_friends_dialog();
    flip_social_free_messages_dialog();
    flip_social_free_compose_dialog();

    return FlipSocialViewLoggedInSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Login screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutLogin)
 */
uint32_t flip_social_callback_to_login_logged_out(void *context)
{
    UNUSED(context);
    flip_social_sent_login_request = false;
    flip_social_login_success = false;
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Register screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutRegister)
 */
uint32_t flip_social_callback_to_register_logged_out(void *context)
{
    UNUSED(context);
    flip_social_sent_register_request = false;
    flip_social_register_success = false;
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutWifiSettings)
 */
uint32_t flip_social_callback_to_wifi_settings_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t flip_social_callback_to_wifi_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t flip_social_callback_to_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Compose screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInCompose)
 */
uint32_t flip_social_callback_to_compose_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Profile screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInProfile)
 */
uint32_t flip_social_callback_to_profile_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the Explore submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewSubmenu)
 */
uint32_t flip_social_callback_to_explore_logged_in(void *context)
{
    UNUSED(context);
    flip_social_dialog_stop = false;
    flip_social_dialog_shown = false;
    if (flip_social_explore)
    {
        flip_social_explore->index = 0;
    }
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the Friends submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInFriendsSubmenu)
 */
uint32_t flip_social_callback_to_friends_logged_in(void *context)
{
    UNUSED(context);
    flip_social_dialog_stop = false;
    flip_social_dialog_shown = false;
    flip_social_friends->index = 0;
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the Messages submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesSubmenu)
 */
uint32_t flip_social_callback_to_messages_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the User Choices screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesUserChoices)
 */
uint32_t flip_social_callback_to_messages_user_choices(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
uint32_t flip_social_callback_exit_app(void *context)
{
    // Exit the application
    UNUSED(context);
    free_all(true, true);
    return VIEW_NONE;
}

void explore_dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Remove
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            // remove friend
            char remove_payload[128];
            snprintf(remove_payload, sizeof(remove_payload), "{\"username\":\"%s\",\"friend\":\"%s\"}", app_instance->login_username_logged_in, flip_social_explore->usernames[flip_social_explore->index]);
            auth_headers_alloc();
            flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/remove-friend/", auth_headers, remove_payload);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
            flip_social_free_explore_dialog();
            furi_delay_ms(1000);
            flipper_http_free(fhttp);
        }
    }
    else if (result == DialogExResultRight)
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            // add friend
            char add_payload[128];
            snprintf(add_payload, sizeof(add_payload), "{\"username\":\"%s\",\"friend\":\"%s\"}", app_instance->login_username_logged_in, flip_social_explore->usernames[flip_social_explore->index]);
            auth_headers_alloc();
            flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/add-friend/", auth_headers, add_payload);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
            flip_social_free_explore_dialog();
            furi_delay_ms(1000);
            flipper_http_free(fhttp);
        }
    }
}
static void friends_dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Remove
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            // remove friend
            char remove_payload[128];
            snprintf(remove_payload, sizeof(remove_payload), "{\"username\":\"%s\",\"friend\":\"%s\"}", app_instance->login_username_logged_in, flip_social_friends->usernames[flip_social_friends->index]);
            auth_headers_alloc();
            flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/remove-friend/", auth_headers, remove_payload);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
            flip_social_free_friends_dialog();
            furi_delay_ms(1000);
            flipper_http_free(fhttp);
        }
    }
}
void messages_dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Previous message
    {
        if (flip_social_messages->index > 0)
        {
            flip_social_messages->index--;
            dialog_ex_reset(app->dialog_messages);
            dialog_ex_set_header(app->dialog_messages, flip_social_messages->usernames[flip_social_messages->index], 0, 0, AlignLeft, AlignTop);
            dialog_ex_set_text(app->dialog_messages, updated_user_message(flip_social_messages->messages[flip_social_messages->index]), 0, 10, AlignLeft, AlignTop);
            if (flip_social_messages->index != 0)
            {
                dialog_ex_set_left_button_text(app->dialog_messages, "Prev");
            }
            dialog_ex_set_right_button_text(app->dialog_messages, "Next");
            dialog_ex_set_center_button_text(app->dialog_messages, "Create");
            // switch view, free dialog, re-alloc dialog, switch back to dialog
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewWidgetResult);
            flip_social_free_messages_dialog();
            messages_dialog_alloc(false);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewMessagesDialog);
        }
    }
    else if (result == DialogExResultRight) // Next message
    {
        if (flip_social_messages->index < flip_social_messages->count - 1)
        {
            flip_social_messages->index++;
            dialog_ex_reset(app->dialog_messages);
            dialog_ex_set_header(app->dialog_messages, flip_social_messages->usernames[flip_social_messages->index], 0, 0, AlignLeft, AlignTop);
            dialog_ex_set_text(app->dialog_messages, updated_user_message(flip_social_messages->messages[flip_social_messages->index]), 0, 10, AlignLeft, AlignTop);
            dialog_ex_set_left_button_text(app->dialog_messages, "Prev");
            if (flip_social_messages->index != flip_social_messages->count - 1)
            {
                dialog_ex_set_right_button_text(app->dialog_messages, "Next");
            }
            dialog_ex_set_center_button_text(app->dialog_messages, "Create");
            // switch view, free dialog, re-alloc dialog, switch back to dialog
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewWidgetResult);
            flip_social_free_messages_dialog();
            messages_dialog_alloc(false);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewMessagesDialog);
        }
    }
    else if (result == DialogExResultCenter) // new message
    {
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInMessagesNewMessageInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
    }
}

static void compose_dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Delete
    {
        // Ensure index is within bounds
        if (app_instance->pre_saved_messages.index >= app_instance->pre_saved_messages.count)
        {
            FURI_LOG_E(TAG, "Invalid index for deletion: %zu", app_instance->pre_saved_messages.index);
            return;
        }

        // Shift messages to remove the selected message
        for (size_t i = app_instance->pre_saved_messages.index; i < app_instance->pre_saved_messages.count - 1; i++)
        {
            strncpy(app_instance->pre_saved_messages.messages[i],
                    app_instance->pre_saved_messages.messages[i + 1],
                    MAX_MESSAGE_LENGTH);
        }

        // Clear the last message after shifting
        memset(app_instance->pre_saved_messages.messages[app_instance->pre_saved_messages.count - 1], 0, MAX_MESSAGE_LENGTH);
        app_instance->pre_saved_messages.count--;

        // Reset and rebuild the submenu
        submenu_reset(app_instance->submenu);
        submenu_add_item(app_instance->submenu, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, flip_social_callback_submenu_choices, app);

        for (size_t i = 0; i < app_instance->pre_saved_messages.count; i++)
        {
            submenu_add_item(app_instance->submenu,
                             app_instance->pre_saved_messages.messages[i],
                             FlipSocialSubemnuComposeIndexStartIndex + i,
                             flip_social_callback_submenu_choices,
                             app);
        }

        // Save the updated playlist
        save_playlist(&app_instance->pre_saved_messages);

        // Switch back to the compose view
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);

        // Free the dialog resources
        flip_social_free_compose_dialog();
    }

    else if (result == DialogExResultRight) // Post
    {
        // post the message
        // send selected_message
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (!fhttp)
        {
            FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
            return;
        }
        if (selected_message && app_instance->login_username_logged_in)
        {
            if (strlen(selected_message) > MAX_MESSAGE_LENGTH)
            {
                FURI_LOG_E(TAG, "Message is too long");
                flipper_http_free(fhttp);
                return;
            }
            // Send the selected_message
            char command[256];
            snprintf(command, sizeof(command), "{\"username\":\"%s\",\"content\":\"%s\"}",
                     app_instance->login_username_logged_in, selected_message);

            if (!flipper_http_request(
                    fhttp,
                    POST,
                    "https://www.jblanked.com/flipper/api/feed/post/",
                    auth_headers,
                    command))
            {
                FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
                flipper_http_free(fhttp);
                return;
            }

            fhttp->state = RECEIVING;
            furi_timer_start(fhttp->get_timeout_timer, TIMEOUT_DURATION_TICKS);
        }
        else
        {
            FURI_LOG_E(TAG, "Message or username is NULL");
            flipper_http_free(fhttp);
            return;
        }
        while (fhttp->state == RECEIVING && furi_timer_is_running(fhttp->get_timeout_timer) > 0)
        {
            furi_delay_ms(100);
        }
        if (flip_social_load_initial_feed(false, 1))
        {
            flip_social_free_compose_dialog();
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to load the initial feed");
            flipper_http_free(fhttp);
        }
    }
}

bool flip_social_get_user_info(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    char url[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/user/users/%s/extended/", flip_social_explore->usernames[flip_social_explore->index]);
    if (!flipper_http_request(fhttp, GET, url, auth_headers, NULL))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for user info");
        fhttp->state = ISSUE;
        return false;
    }
    fhttp->state = RECEIVING;
    return true;
}
bool flip_social_parse_user_info(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    if (fhttp->last_response == NULL)
    {
        FURI_LOG_E(TAG, "Response is NULL");
        return false;
    }
    if (!app_instance->explore_user_bio)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    char *bio = get_json_value("bio", fhttp->last_response);
    char *friends = get_json_value("friends", fhttp->last_response);
    if (bio && friends)
    {
        if (strlen(bio) != 0)
        {
            snprintf(app_instance->explore_user_bio, MAX_MESSAGE_LENGTH, "%s (%s friends)", bio, friends);
        }
        else
        {
            snprintf(app_instance->explore_user_bio, MAX_MESSAGE_LENGTH, "%s friends", friends);
        }
        free(bio);
        free(friends);
        return true;
    }
    return false;
}

/**
 * @brief Handle ALL submenu item selections.
 * @param context The context - FlipSocialApp object.
 * @param index The FlipSocialSubmenuIndex item that was clicked.
 * @return void
 */
void flip_social_callback_submenu_choices(void *context, uint32_t index)
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
        free_all(true, true);
        if (!alloc_variable_item_list(FlipSocialViewLoggedOutLogin))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedOutIndexRegister:
        flip_social_sent_register_request = false;
        flip_social_register_success = false;
        free_all(true, true);
        if (!alloc_variable_item_list(FlipSocialViewLoggedOutRegister))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedOutIndexAbout:
        if (!about_widget_alloc(false))
        {
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutAbout);
        break;
    case FlipSocialSubmenuLoggedOutIndexWifiSettings:
        free_all(false, false);
        if (!alloc_variable_item_list(FlipSocialViewLoggedOutWifiSettings))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInIndexProfile:
        free_all(true, true);
        if (!alloc_variable_item_list(FlipSocialViewLoggedInProfile))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInIndexMessages:
        free_all(true, true);
        if (!alloc_submenu(FlipSocialViewLoggedInMessagesSubmenu))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu");
            return;
        }
        // flipper_http_loading_task(
        //     flip_social_get_message_users,        // get the message users
        //     flip_social_parse_json_message_users, // parse the message users
        //     FlipSocialViewSubmenu,                // switch to the messages submenu if successful
        //     FlipSocialViewLoggedInSubmenu,        // switch back to the main submenu if failed
        //     &app->view_dispatcher);               // view dispatcher
        break;
    case FlipSocialSubmenuLoggedInIndexMessagesNewMessage:
        // they need to search for the user to send a message
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInMessageUsersInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case FlipSocialSubmenuLoggedInIndexFeed:
        free_all(true, true);
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (!fhttp)
        {
            FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
            return;
        }
        if (!flip_social_load_initial_feed(fhttp, 1))
        {
            FURI_LOG_E(TAG, "Failed to load the initial feed");
            return;
        }
        flipper_http_free(fhttp);
        break;
    case FlipSocialSubmenuExploreIndex:
        free_all(true, true);
        if (!alloc_text_input(FlipSocialViewLoggedInExploreInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case FlipSocialSubmenuLoggedInIndexCompose:
        free_all(true, true);
        if (!alloc_submenu(FlipSocialViewLoggedInCompose))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        break;
    case FlipSocialSubmenuLoggedInIndexSettings:
        free_all(true, true);
        if (!alloc_submenu(FlipSocialViewLoggedInSettings))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        break;
    case FlipSocialSubmenuLoggedInIndexAbout:
        free_all(true, false);
        if (!about_widget_alloc(true))
        {
            FURI_LOG_E(TAG, "Failed to allocate about widget");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsAbout);
        break;
    case FlipSocialSubmenuLoggedInIndexWifiSettings:
        free_all(true, false);
        if (!alloc_variable_item_list(FlipSocialViewLoggedInSettingsWifi))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInIndexUserSettings:
        free_all(true, false);
        if (!alloc_variable_item_list(FlipSocialViewLoggedInSettingsUser))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInSignOutButton:
        free_all(true, true);
        app->is_logged_in = "false";

        save_settings(app->wifi_ssid_logged_out, app->wifi_password_logged_out, app->login_username_logged_out, app->login_username_logged_in, app->login_password_logged_out, app->change_password_logged_in, app->change_bio_logged_in, app->is_logged_in);

        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        break;
    case FlipSocialSubmenuComposeIndexAddPreSave:
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInComposeAddPreSaveInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    default:
        // Handle the pre-saved message selection (has a max of 25 items)
        if (index >= FlipSocialSubemnuComposeIndexStartIndex && index < FlipSocialSubemnuComposeIndexStartIndex + MAX_PRE_SAVED_MESSAGES)
        {
            app->pre_saved_messages.index = index - FlipSocialSubemnuComposeIndexStartIndex;
            snprintf(selected_message, MAX_MESSAGE_LENGTH, "%s", app->pre_saved_messages.messages[app->pre_saved_messages.index]);
            if (!selected_message)
            {
                FURI_LOG_E(TAG, "Selected message is NULL");
                return;
            }
            flip_social_free_compose_dialog();
            if (!app->dialog_compose)
            {
                if (!easy_flipper_set_dialog_ex(
                        &app->dialog_compose,
                        FlipSocialViewComposeDialog,
                        "New Feed Post",
                        0,
                        0,
                        updated_user_message(selected_message),
                        0,
                        10,
                        "Delete",
                        "Post",
                        NULL,
                        compose_dialog_callback,
                        flip_social_callback_to_compose_logged_in,
                        &app->view_dispatcher,
                        app))
                {
                    return;
                }
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewComposeDialog);
        }

        // Handle the explore selection
        else if (index >= FlipSocialSubmenuExploreIndexStartIndex && index < FlipSocialSubmenuExploreIndexStartIndex + MAX_EXPLORE_USERS)
        {
            if (!flip_social_explore)
            {
                FURI_LOG_E(TAG, "FlipSocialExplore is NULL");
                return;
            }
            flip_social_explore->index = index - FlipSocialSubmenuExploreIndexStartIndex;
            // loading task to get the user info
            if (app->explore_user_bio)
            {
                free(app->explore_user_bio);
                app->explore_user_bio = NULL;
            }
            if (!easy_flipper_set_buffer(&app->explore_user_bio, MAX_MESSAGE_LENGTH))
            {
                return;
            }
            // if (flipper_http_process_response_async(flip_social_get_user_info, flip_social_parse_user_info))
            // {
            //     flip_social_free_explore_dialog();
            //     if (!app->dialog_explore)
            //     {
            //         if (!easy_flipper_set_dialog_ex(
            //                 &app->dialog_explore,
            //                 FlipSocialViewExploreDialog,
            //                 flip_social_explore->usernames[flip_social_explore->index],
            //                 0,
            //                 0,
            //                 updated_user_message(app->explore_user_bio),
            //                 0,
            //                 10,
            //                 "Remove", // remove if user is a friend (future update)
            //                 "Add",    // add if user is not a friend (future update)
            //                 NULL,
            //                 explore_dialog_callback,
            //                 flip_social_callback_to_explore_logged_in,
            //                 &app->view_dispatcher,
            //                 app))
            //         {
            //             return;
            //         }
            //     }
            //     view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewExploreDialog);
            // }
            // else
            // {
            //     flip_social_free_explore_dialog();
            //     if (!app->dialog_explore)
            //     {
            //         if (!easy_flipper_set_dialog_ex(
            //                 &app->dialog_explore,
            //                 FlipSocialViewExploreDialog,
            //                 flip_social_explore->usernames[flip_social_explore->index],
            //                 0,
            //                 0,
            //                 "",
            //                 0,
            //                 10,
            //                 "Remove", // remove if user is a friend (future update)
            //                 "Add",    // add if user is not a friend (future update)
            //                 NULL,
            //                 explore_dialog_callback,
            //                 flip_social_callback_to_explore_logged_in,
            //                 &app->view_dispatcher,
            //                 app))
            //         {
            //             return;
            //         }
            //     }
            //     view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewExploreDialog);
            // }
        }

        // handle the friends selection
        else if (index >= FlipSocialSubmenuLoggedInIndexFriendsStart && index < FlipSocialSubmenuLoggedInIndexFriendsStart + MAX_FRIENDS)
        {
            if (!flip_social_friends)
            {
                FURI_LOG_E(TAG, "FlipSocialFriends is NULL");
                return;
            }
            flip_social_friends->index = index - FlipSocialSubmenuLoggedInIndexFriendsStart;
            flip_social_free_friends_dialog();
            if (!app->dialog_friends)
            {
                if (!easy_flipper_set_dialog_ex(
                        &app->dialog_friends,
                        FlipSocialViewFriendsDialog,
                        "Friend Options",
                        0,
                        0,
                        flip_social_friends->usernames[flip_social_friends->index],
                        0,
                        10,
                        "Remove",
                        "",
                        NULL,
                        friends_dialog_callback,
                        flip_social_callback_to_friends_logged_in,
                        &app->view_dispatcher,
                        app))
                {
                    return;
                }
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewFriendsDialog);
        }

        // handle the messages selection
        else if (index >= FlipSocialSubmenuLoggedInIndexMessagesUsersStart && index < FlipSocialSubmenuLoggedInIndexMessagesUsersStart + MAX_MESSAGE_USERS)
        {
            if (!flip_social_message_users)
            {
                FURI_LOG_E(TAG, "FlipSocialMessageUsers is NULL");
                return;
            }
            flip_social_message_users->index = index - FlipSocialSubmenuLoggedInIndexMessagesUsersStart;
            // flipper_http_loading_task(
            //     flip_social_get_messages_with_user,    // get the messages with the selected user
            //     flip_social_parse_json_messages,       // parse the messages
            //     FlipSocialViewMessagesDialog,          // switch to the messages process if successful
            //     FlipSocialViewLoggedInMessagesSubmenu, // switch back to the messages submenu if failed
            //     &app->view_dispatcher                  // view dispatcher
            // );
        }

        // handle the messages user choices selection
        else if (index >= FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart && index < FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart + MAX_EXPLORE_USERS)
        {
            if (!flip_social_explore)
            {
                FURI_LOG_E(TAG, "FlipSocialExplore is NULL");
                return;
            }
            flip_social_explore->index = index - FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart;
            free_text_input();
            if (!alloc_text_input(FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput))
            {
                FURI_LOG_E(TAG, "Failed to allocate text input");
                return;
            } // switch to the text input view
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        }
        else
        {
            FURI_LOG_E(TAG, "Unknown submenu index");
        }

        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_out_wifi_settings_ssid_updated(void *context)
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
    if (strlen(app->wifi_ssid_logged_out) > 0 && strlen(app->wifi_password_logged_out) > 0)
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            if (!flipper_http_save_wifi(fhttp, app->wifi_ssid_logged_out, app->wifi_password_logged_out))
            {
                FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
                FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
            }
            furi_delay_ms(500);
            flipper_http_free(fhttp);
        }
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_out_wifi_settings_password_updated(void *context)
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
    if (strlen(app->wifi_ssid_logged_out) > 0 && strlen(app->wifi_password_logged_out) > 0)
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            if (!flipper_http_save_wifi(fhttp, app->wifi_ssid_logged_out, app->wifi_password_logged_out))
            {
                FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
                FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
            }
            furi_delay_ms(500);
            flipper_http_free(fhttp);
        }
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void flip_social_text_input_logged_out_wifi_settings_item_selected(void *context, uint32_t index)
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
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsSSIDInput);
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedOutWifiSettingsSSIDInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsPasswordInput);
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedOutWifiSettingsPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
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
void flip_social_logged_out_login_username_updated(void *context)
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
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */

void flip_social_logged_out_login_password_updated(void *context)
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
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void flip_social_text_input_logged_out_login_item_selected(void *context, uint32_t index)
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
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginUsernameInput);
        free_all(false, true);
        if (!alloc_text_input(FlipSocialViewLoggedOutLoginUsernameInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginPasswordInput);
        free_all(false, true);
        if (!alloc_text_input(FlipSocialViewLoggedOutLoginPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 2: // Login Button
        // if (!flipper_http_init(flipper_http_rx_callback, app_instance))
        // {
        //     FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
        //     return;
        // }
        flip_social_login_switch_to_view(app);
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
void flip_social_logged_out_register_username_updated(void *context)
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_out_register_password_updated(void *context)
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password 2 on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_out_register_password_2_updated(void *context)
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void flip_social_text_input_logged_out_register_item_selected(void *context, uint32_t index)
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
        free_all(false, true);
        if (!alloc_text_input(FlipSocialViewLoggedOutRegisterUsernameInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        free_all(false, true);
        if (!alloc_text_input(FlipSocialViewLoggedOutRegisterPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 2: // Input Password 2
        free_all(false, true);
        if (!alloc_text_input(FlipSocialViewLoggedOutRegisterPassword2Input))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 3: // Register button
        // if (!flipper_http_init(flipper_http_rx_callback, app_instance))
        // {
        //     FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
        //     return;
        // }
        flip_social_register_switch_to_view(app);
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
void flip_social_logged_in_wifi_settings_ssid_updated(void *context)
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
    save_settings(app_instance->wifi_ssid_logged_in, app_instance->wifi_password_logged_in, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_password_logged_in) > 0)
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            if (!flipper_http_save_wifi(fhttp, app->wifi_ssid_logged_in, app->wifi_password_logged_in))
            {
                FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
                FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
            }
            furi_delay_ms(500);
            flipper_http_free(fhttp);
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_in_wifi_settings_password_updated(void *context)
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
    save_settings(app_instance->wifi_ssid_logged_in, app_instance->wifi_password_logged_in, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_password_logged_in) > 0)
    {
        FlipperHTTP *fhttp = flipper_http_alloc();
        if (fhttp)
        {
            if (!flipper_http_save_wifi(fhttp, app->wifi_ssid_logged_in, app->wifi_password_logged_in))
            {
                FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
                FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
            }
            furi_delay_ms(500);
            flipper_http_free(fhttp);
        }
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void flip_social_text_input_logged_in_wifi_settings_item_selected(void *context, uint32_t index)
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
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsSSIDInput);
        free_all(false, false);
        if (!alloc_text_input(FlipSocialViewLoggedInWifiSettingsSSIDInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for SSID");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsPasswordInput);
        free_all(false, false);
        if (!alloc_text_input(FlipSocialViewLoggedInWifiSettingsPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for Password");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

void flip_social_logged_in_user_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // Switch to the appropriate view
    switch (index)
    {
    case 0: // Feed Type
        break;
    case 1: // Notifications
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their message on the compose (logged in) screen for Add Text
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_in_compose_pre_save_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // Check if the message is empty or if adding the message would exceed MAX_PRE_SAVED_MESSAGES
    if (app->compose_pre_save_logged_in_temp_buffer_size == 0 || app->pre_saved_messages.count >= MAX_PRE_SAVED_MESSAGES)
    {
        FURI_LOG_E(TAG, "Message is empty or would exceed the maximum number of pre-saved messages");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        return;
    }

    // Copy the entered message into the next available slot
    strncpy(
        app->pre_saved_messages.messages[app->pre_saved_messages.count],
        app->compose_pre_save_logged_in_temp_buffer,
        MAX_MESSAGE_LENGTH - 1);

    // Ensure null-termination
    app->pre_saved_messages.messages[app->pre_saved_messages.count][MAX_MESSAGE_LENGTH - 1] = '\0';

    // Increment the count
    app->pre_saved_messages.count++;

    // Rebuild the submenu
    submenu_reset(app->submenu);
    submenu_add_item(
        app->submenu,
        "Add Pre-Save",
        FlipSocialSubmenuComposeIndexAddPreSave,
        flip_social_callback_submenu_choices,
        app);

    for (size_t i = 0; i < app->pre_saved_messages.count; i++)
    {
        submenu_add_item(
            app->submenu,
            app->pre_saved_messages.messages[i],
            FlipSocialSubemnuComposeIndexStartIndex + i,
            flip_social_callback_submenu_choices,
            app);
    }

    // Save the updated playlist
    save_playlist(&app->pre_saved_messages);

    // Switch back to the compose view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
}

/**
 * @brief Text input callback for when the user finishes entering their message on the profile (logged in) screen for change password
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_in_profile_change_password_updated(void *context)
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
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (fhttp)
    {
        auth_headers_alloc();
        char payload[256];
        snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"old_password\":\"%s\",\"new_password\":\"%s\"}", app->login_username_logged_out, old_password, app->change_password_logged_in);
        if (!flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/change-password/", auth_headers, payload))
        {
            FURI_LOG_E(TAG, "Failed to send post request to change password");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
        flipper_http_free(fhttp);
    }
    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    // instead of going to a view, just show a success message
    easy_flipper_dialog("Success", "Password updated successfully\n\n\nPress BACK to return :D");
}

void flip_social_logged_in_profile_change_bio_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // Store the entered message
    strncpy(app->change_bio_logged_in, app->change_bio_logged_in_temp_buffer, app->change_bio_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->change_bio_logged_in[app->change_bio_logged_in_temp_buffer_size - 1] = '\0';

    // Update the message item text
    if (app->variable_item_logged_in_profile_change_bio)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_profile_change_bio, app->change_bio_logged_in);
    }

    // send post request to change bio
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (fhttp)
    {
        auth_headers_alloc();
        char payload[256];
        snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"bio\":\"%s\"}", app->login_username_logged_out, app->change_bio_logged_in);
        if (!flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/change-bio/", auth_headers, payload))
        {
            FURI_LOG_E(TAG, "Failed to send post request to change bio");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
        furi_delay_ms(500);
        flipper_http_free(fhttp);
    }
    // Save the settings
    save_settings(app->wifi_ssid_logged_out, app->wifi_password_logged_out, app->login_username_logged_out, app->login_username_logged_in, app->login_password_logged_out, app->change_password_logged_in, app->change_bio_logged_in, app->is_logged_in);

    // instead of going to a view, just show a success message
    easy_flipper_dialog("Success", "Bio updated successfully\n\n\nPress BACK to return :D");
}

/**
 * @brief Callback when a user selects a menu item in the profile (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void flip_social_text_input_logged_in_profile_item_selected(void *context, uint32_t index)
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
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInChangePasswordInput);
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInChangePasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for change password");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 2: // Change Bio
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInChangeBioInput);
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInChangeBioInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for change bio");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 3: // Friends
        free_all(false, true);
        if (!alloc_submenu(FlipSocialViewLoggedInFriendsSubmenu))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu for friends");
            return;
        }
        // if (!flipper_http_init(flipper_http_rx_callback, app))
        // {
        //     FURI_LOG_E(TAG, "Failed to initialize FlipperHTTP");
        //     return;
        // }
        // flipper_http_loading_task(
        //     flip_social_get_friends,
        //     flip_social_parse_json_friends,
        //     FlipSocialViewSubmenu,
        //     FlipSocialViewVariableItemList,
        //     &app->view_dispatcher);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their message to send to the selected user choice (user choice messages view)
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_in_messages_user_choice_message_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (app->message_user_choice_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        return;
    }

    // Store the entered message
    strncpy(app->message_user_choice_logged_in, app->message_user_choice_logged_in_temp_buffer, app->message_user_choice_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->message_user_choice_logged_in[app->message_user_choice_logged_in_temp_buffer_size - 1] = '\0';

    // send post request to send message
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "Failed to initialize HTTP");
        return;
    }
    auth_headers_alloc();
    char url[128];
    char payload[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/messages/%s/post/", app->login_username_logged_in);
    snprintf(payload, sizeof(payload), "{\"receiver\":\"%s\",\"content\":\"%s\"}", flip_social_explore->usernames[flip_social_explore->index], app->message_user_choice_logged_in);

    if (!flipper_http_request(fhttp, POST, url, auth_headers, payload)) // start the async request
    {
        FURI_LOG_E(TAG, "Failed to send post request to send message");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }
    furi_delay_ms(1000);
    flipper_http_free(fhttp);
    // add user to the top of the list if not already there
    for (int i = 0; i < flip_social_message_users->count; i++)
    {
        if (strcmp(flip_social_message_users->usernames[i], flip_social_explore->usernames[flip_social_explore->index]) == 0)
        {
            // remove the user from the list
            for (int j = i; j < flip_social_message_users->count - 1; j++)
            {
                strncpy(flip_social_message_users->usernames[j], flip_social_message_users->usernames[j + 1], strlen(flip_social_message_users->usernames[j + 1]));
            }
            flip_social_message_users->count--;
            break;
        }
    }
    // add the user to the top of the list
    for (int i = flip_social_message_users->count; i > 0; i--)
    {
        strncpy(flip_social_message_users->usernames[i], flip_social_message_users->usernames[i - 1], strlen(flip_social_message_users->usernames[i - 1]));
    }
    strncpy(flip_social_message_users->usernames[0], flip_social_explore->usernames[flip_social_explore->index], strlen(flip_social_explore->usernames[flip_social_explore->index]));
    flip_social_message_users->count++;

    // redraw submenu
    flip_social_update_messages_submenu();
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
}

/**
 * @brief Text input callback for when the user finishes entering their message to the selected user (messages view)
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void flip_social_logged_in_messages_new_message_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (app->messages_new_message_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageInput);
        return;
    }

    // Store the entered message
    strncpy(app->messages_new_message_logged_in, app->messages_new_message_logged_in_temp_buffer, app->messages_new_message_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->messages_new_message_logged_in[app->messages_new_message_logged_in_temp_buffer_size - 1] = '\0';

    FlipperHTTP *fhttp = flipper_http_alloc();
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "Failed to initialize HTTP");
        return;
    }

    bool send_message_to_user()
    {
        // send post request to send message
        auth_headers_alloc();
        char url[128];
        char payload[256];
        snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/messages/%s/post/", app->login_username_logged_in);
        snprintf(payload, sizeof(payload), "{\"receiver\":\"%s\",\"content\":\"%s\"}", flip_social_message_users->usernames[flip_social_message_users->index], app->messages_new_message_logged_in);
        if (!flipper_http_request(fhttp, POST, url, auth_headers, payload))
        {
            FURI_LOG_E(TAG, "Failed to send post request to send message");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
            easy_flipper_dialog("Error", "Failed to send message\n\n\nPress BACK to return :D");
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
            return false;
        }
        fhttp->state = RECEIVING;
        return true;
    }
    bool parse_message_to_user()
    {
        while (fhttp->state != IDLE)
        {
            furi_delay_ms(10);
        }
        return true;
    }

    // well, we got a freeze here, so let's use the loading task to switch views and force refresh
    flipper_http_loading_task(
        fhttp,
        send_message_to_user,
        parse_message_to_user,
        FlipSocialViewSubmenu,
        FlipSocialViewLoggedInMessagesNewMessageInput,
        &app->view_dispatcher);

    flipper_http_free(fhttp);
}

void flip_social_logged_in_explore_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (app->explore_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        strncpy(app->explore_logged_in, "a", 2);
    }
    else
    {
        // Store the entered message
        strncpy(app->explore_logged_in, app->explore_logged_in_temp_buffer, app->explore_logged_in_temp_buffer_size);
    }

    // Ensure null-termination
    app->explore_logged_in[app->explore_logged_in_temp_buffer_size - 1] = '\0';
    free_submenu();

    if (!alloc_submenu(FlipSocialViewLoggedInExploreSubmenu))
    {
        FURI_LOG_E(TAG, "Failed to allocate submenu for explore");
        return;
    }

    // flipper_http_loading_task(
    //     flip_social_get_explore,        // get the explore users
    //     flip_social_parse_json_explore, // parse the explore users
    //     FlipSocialViewSubmenu,          // switch to the explore submenu if successful
    //     FlipSocialViewLoggedInSubmenu,  // switch back to the main submenu if failed
    //     &app->view_dispatcher);         // view dispatcher
}

void flip_social_logged_in_message_users_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (!app->message_users_logged_in_temp_buffer || app->message_users_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        strncpy(app->message_users_logged_in, "a", 2);
    }
    else
    {
        // Store the entered message
        strncpy(app->message_users_logged_in, app->message_users_logged_in_temp_buffer, app->message_users_logged_in_temp_buffer_size);
    }

    // Ensure null-termination
    app->message_users_logged_in[app->message_users_logged_in_temp_buffer_size - 1] = '\0';

    free_submenu();

    if (!alloc_submenu(FlipSocialViewLoggedInExploreSubmenu))
    {
        FURI_LOG_E(TAG, "Failed to allocate submenu for explore");
        return;
    }

    // get users
    // flipper_http_loading_task(
    //     flip_social_get_explore_2,                   // get the explore users
    //     flip_social_parse_json_message_user_choices, // parse the explore users
    //     FlipSocialViewSubmenu,                       // switch to the explore submenu if successful
    //     FlipSocialViewLoggedInSubmenu,               // switch back to the main submenu if failed
    //     &app->view_dispatcher);                      // view dispatcher
}

static void flip_social_widget_set_text(char *message, Widget **widget)
{
    if (widget == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_set_widget_text - widget is NULL");
        DEV_CRASH();
        return;
    }
    if (message == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_set_widget_text - message is NULL");
        DEV_CRASH();
        return;
    }
    widget_reset(*widget);

    uint32_t message_length = strlen(message); // Length of the message
    uint32_t i = 0;                            // Index tracker
    uint32_t formatted_index = 0;              // Tracker for where we are in the formatted message
    char *formatted_message;                   // Buffer to hold the final formatted message

    // Allocate buffer with double the message length plus one for safety
    if (!easy_flipper_set_buffer(&formatted_message, message_length * 2 + 1))
    {
        return;
    }

    while (i < message_length)
    {
        uint32_t max_line_length = 31;                  // Maximum characters per line
        uint32_t remaining_length = message_length - i; // Remaining characters
        uint32_t line_length = (remaining_length < max_line_length) ? remaining_length : max_line_length;

        // Check for newline character within the current segment
        uint32_t newline_pos = i;
        bool found_newline = false;
        for (; newline_pos < i + line_length && newline_pos < message_length; newline_pos++)
        {
            if (message[newline_pos] == '\n')
            {
                found_newline = true;
                break;
            }
        }

        if (found_newline)
        {
            // If newline found, set line_length up to the newline
            line_length = newline_pos - i;
        }

        // Temporary buffer to hold the current line
        char line[32];
        strncpy(line, message + i, line_length);
        line[line_length] = '\0';

        // If newline was found, skip it for the next iteration
        if (found_newline)
        {
            i += line_length + 1; // +1 to skip the '\n' character
        }
        else
        {
            // Check if the line ends in the middle of a word and adjust accordingly
            if (line_length == max_line_length && message[i + line_length] != '\0' && message[i + line_length] != ' ')
            {
                // Find the last space within the current line to avoid breaking a word
                char *last_space = strrchr(line, ' ');
                if (last_space != NULL)
                {
                    // Adjust the line_length to avoid cutting the word
                    line_length = last_space - line;
                    line[line_length] = '\0'; // Null-terminate at the space
                }
            }

            // Move the index forward by the determined line_length
            i += line_length;

            // Skip any spaces at the beginning of the next line
            while (i < message_length && message[i] == ' ')
            {
                i++;
            }
        }

        // Manually copy the fixed line into the formatted_message buffer
        for (uint32_t j = 0; j < line_length; j++)
        {
            formatted_message[formatted_index++] = line[j];
        }

        // Add a newline character for line spacing
        formatted_message[formatted_index++] = '\n';
    }

    // Null-terminate the formatted_message
    formatted_message[formatted_index] = '\0';

    // Add the formatted message to the widget
    widget_add_text_scroll_element(*widget, 0, 0, 128, 64, formatted_message);
}

void flip_social_loader_draw_callback(Canvas *canvas, void *model)
{
    if (!canvas || !model)
    {
        FURI_LOG_E(TAG, "flip_social_loader_draw_callback - canvas or model is NULL");
        return;
    }

    DataLoaderModel *data_loader_model = (DataLoaderModel *)model;
    HTTPState http_state = data_loader_model->fhttp->state;
    DataState data_state = data_loader_model->data_state;
    char *title = data_loader_model->title;

    canvas_set_font(canvas, FontSecondary);

    if (http_state == INACTIVE)
    {
        canvas_draw_str(canvas, 0, 7, "Wifi Dev Board disconnected.");
        canvas_draw_str(canvas, 0, 17, "Please connect to the board.");
        canvas_draw_str(canvas, 0, 32, "If your board is connected,");
        canvas_draw_str(canvas, 0, 42, "make sure you have flashed");
        canvas_draw_str(canvas, 0, 52, "your WiFi Devboard with the");
        canvas_draw_str(canvas, 0, 62, "latest FlipperHTTP flash.");
        return;
    }

    if (data_state == DataStateError || data_state == DataStateParseError)
    {
        flip_social_request_error_draw(canvas, data_loader_model);
        return;
    }

    canvas_draw_str(canvas, 0, 7, title);
    canvas_draw_str(canvas, 0, 17, "Loading...");

    if (data_state == DataStateInitial)
    {
        return;
    }

    if (http_state == SENDING)
    {
        canvas_draw_str(canvas, 0, 27, "Fetching...");
        return;
    }

    if (http_state == RECEIVING || data_state == DataStateRequested)
    {
        canvas_draw_str(canvas, 0, 27, "Receiving...");
        return;
    }

    if (http_state == IDLE && data_state == DataStateReceived)
    {
        canvas_draw_str(canvas, 0, 27, "Processing...");
        return;
    }

    if (http_state == IDLE && data_state == DataStateParsed)
    {
        canvas_draw_str(canvas, 0, 27, "Processed...");
        return;
    }
}

static void flip_social_loader_process_callback(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "loader_process_callback - context is NULL");
        DEV_CRASH();
        return;
    }

    FlipSocialApp *app = (FlipSocialApp *)context;
    View *view = app->view_loader;

    DataState current_data_state;
    DataLoaderModel *loader_model = NULL;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            current_data_state = model->data_state;
            loader_model = model;
        },
        false);
    if (!loader_model || !loader_model->fhttp)
    {
        FURI_LOG_E(TAG, "Model or fhttp is NULL");
        DEV_CRASH();
        return;
    }

    if (current_data_state == DataStateInitial)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                model->data_state = DataStateRequested;
                DataLoaderFetch fetch = model->fetcher;
                if (fetch == NULL)
                {
                    FURI_LOG_E(TAG, "Model doesn't have Fetch function assigned.");
                    model->data_state = DataStateError;
                    return;
                }

                // Clear any previous responses
                strncpy(model->fhttp->last_response, "", 1);
                bool request_status = fetch(model);
                if (!request_status)
                {
                    model->data_state = DataStateError;
                }
            },
            true);
    }
    else if (current_data_state == DataStateRequested || current_data_state == DataStateError)
    {
        if (loader_model->fhttp->state == IDLE && loader_model->fhttp->last_response != NULL)
        {
            if (strstr(loader_model->fhttp->last_response, "[PONG]") != NULL)
            {
                FURI_LOG_DEV(TAG, "PONG received.");
            }
            else if (strncmp(loader_model->fhttp->last_response, "[SUCCESS]", 9))
            {
                FURI_LOG_DEV(TAG, "SUCCESS received. %s", loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            }
            else if (strncmp(loader_model->fhttp->last_response, "[ERROR]", 9))
            {
                FURI_LOG_DEV(TAG, "ERROR received. %s", loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            }
            else if (strlen(loader_model->fhttp->last_response))
            {
                // Still waiting on response
            }
            else
            {
                with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateReceived; }, true);
            }
        }
        else if (loader_model->fhttp->state == SENDING || loader_model->fhttp->state == RECEIVING)
        {
            // continue waiting
        }
        else if (loader_model->fhttp->state == INACTIVE)
        {
            // inactive. try again
        }
        else if (loader_model->fhttp->state == ISSUE)
        {
            with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateError; }, true);
        }
        else
        {
            FURI_LOG_DEV(TAG, "Unexpected state: %d lastresp: %s", loader_model->fhttp->state, loader_model->fhttp->last_response ? loader_model->fhttp->last_response : "NULL");
            DEV_CRASH();
        }
    }
    else if (current_data_state == DataStateReceived)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                char *data_text;
                if (model->parser == NULL)
                {
                    data_text = NULL;
                    FURI_LOG_DEV(TAG, "Parser is NULL");
                    DEV_CRASH();
                }
                else
                {
                    data_text = model->parser(model);
                }
                FURI_LOG_DEV(TAG, "Parsed data: %s\r\ntext: %s", model->fhttp->last_response ? model->fhttp->last_response : "NULL", data_text ? data_text : "NULL");
                model->data_text = data_text;
                if (data_text == NULL)
                {
                    model->data_state = DataStateParseError;
                }
                else
                {
                    model->data_state = DataStateParsed;
                }
            },
            true);
    }
    else if (current_data_state == DataStateParsed)
    {
        with_view_model(
            view,
            DataLoaderModel * model,
            {
                if (++model->request_index < model->request_count)
                {
                    model->data_state = DataStateInitial;
                }
                else
                {
                    flip_social_widget_set_text(model->data_text != NULL ? model->data_text : "", &app->widget_result);
                    if (model->data_text != NULL)
                    {
                        free(model->data_text);
                        model->data_text = NULL;
                    }
                    view_set_previous_callback(widget_get_view(app->widget_result), model->back_callback);
                    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewWidgetResult);
                }
            },
            true);
    }
}

static void flip_social_loader_timer_callback(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_loader_timer_callback - context is NULL");
        DEV_CRASH();
        return;
    }
    FlipSocialApp *app = (FlipSocialApp *)context;
    view_dispatcher_send_custom_event(app->view_dispatcher, FlipSocialCustomEventProcess);
}

static void flip_social_loader_on_enter(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_loader_on_enter - context is NULL");
        DEV_CRASH();
        return;
    }
    FlipSocialApp *app = (FlipSocialApp *)context;
    View *view = app->view_loader;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            view_set_previous_callback(view, model->back_callback);
            if (model->timer == NULL)
            {
                model->timer = furi_timer_alloc(flip_social_loader_timer_callback, FuriTimerTypePeriodic, app);
            }
            furi_timer_start(model->timer, 250);
        },
        true);
}

static void flip_social_loader_on_exit(void *context)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_loader_on_exit - context is NULL");
        DEV_CRASH();
        return;
    }
    FlipSocialApp *app = (FlipSocialApp *)context;
    View *view = app->view_loader;
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            if (model->timer)
            {
                furi_timer_stop(model->timer);
            }
        },
        false);
}

void flip_social_loader_init(View *view)
{
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_loader_init - view is NULL");
        DEV_CRASH();
        return;
    }
    view_allocate_model(view, ViewModelTypeLocking, sizeof(DataLoaderModel));
    view_set_enter_callback(view, flip_social_loader_on_enter);
    view_set_exit_callback(view, flip_social_loader_on_exit);
}

void flip_social_loader_free_model(View *view)
{
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "loader_free_model - view is NULL");
        DEV_CRASH();
        return;
    }
    with_view_model(
        view,
        DataLoaderModel * model,
        {
            if (model->timer)
            {
                furi_timer_free(model->timer);
                model->timer = NULL;
            }
            if (model->parser_context)
            {
                // do not free the context here, it is the app context
                // free(model->parser_context);
                // model->parser_context = NULL;
            }
            if (model->fhttp)
            {
                flipper_http_free(model->fhttp);
                model->fhttp = NULL;
            }
        },
        false);
}

bool flip_social_custom_event_callback(void *context, uint32_t index)
{
    if (context == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_custom_event_callback - context is NULL");
        DEV_CRASH();
        return false;
    }

    switch (index)
    {
    case FlipSocialCustomEventProcess:
        flip_social_loader_process_callback(context);
        return true;
    default:
        FURI_LOG_DEV(TAG, "flip_social_custom_event_callback. Unknown index: %ld", index);
        return false;
    }
}

void flip_social_generic_switch_to_view(FlipSocialApp *app, char *title, DataLoaderFetch fetcher, DataLoaderParser parser, size_t request_count, ViewNavigationCallback back, uint32_t view_id)
{
    if (app == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_generic_switch_to_view - app is NULL");
        DEV_CRASH();
        return;
    }

    View *view = app->view_loader;
    if (view == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_generic_switch_to_view - view is NULL");
        DEV_CRASH();
        return;
    }

    with_view_model(
        view,
        DataLoaderModel * model,
        {
            model->title = title;
            model->fetcher = fetcher;
            model->parser = parser;
            model->request_index = 0;
            model->request_count = request_count;
            model->back_callback = back;
            model->data_state = DataStateInitial;
            model->data_text = NULL;
            //
            model->parser_context = app;
            if (!model->fhttp)
            {
                model->fhttp = flipper_http_alloc();
            }
        },
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, view_id);
}

bool flip_social_get_home_notification(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    // Create the directory for saving settings
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);
    furi_record_close(RECORD_STORAGE);
    auth_headers_alloc();

    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data/notification.json");

    fhttp->save_received_data = true;
    return flipper_http_request(fhttp, GET, "https://www.jblanked.com/flipper/api/flip-social-notifications/", auth_headers, NULL);
}
bool flip_social_parse_home_notification(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }

    // Load the notification from the file
    FuriString *notification = flipper_http_load_from_file(fhttp->file_path);
    if (notification == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load notification from file");
        return false;
    }

    // Check if announcement and analytics key exists
    FuriString *announcement_json = get_json_value_furi("announcement", notification);
    FuriString *analytics_json = get_json_value_furi("analytics", notification);
    if (announcement_json == NULL || analytics_json == NULL)
    {
        FURI_LOG_E(TAG, "Failed to get announcement or analytics from notification");
        if (announcement_json)
        {
            furi_string_free(announcement_json);
        }
        if (analytics_json)
        {
            furi_string_free(analytics_json);
        }
        furi_string_free(notification);
        return false;
    }

    // Extract values from JSON
    FuriString *announcement_value = get_json_value_furi("content", announcement_json);
    FuriString *announcement_time = get_json_value_furi("date_created", announcement_json);
    FuriString *analytics_value = get_json_value_furi("count", analytics_json);
    FuriString *analytics_time = get_json_value_furi("time", analytics_json);
    if (!announcement_value || !announcement_time || !analytics_value || !analytics_time)
    {
        FURI_LOG_E(TAG, "Failed to get announcement or analytics value from notification");
        if (announcement_value)
        {
            furi_string_free(announcement_value);
        }
        if (announcement_time)
        {
            furi_string_free(announcement_time);
        }
        if (analytics_value)
        {
            furi_string_free(analytics_value);
        }
        if (analytics_time)
        {
            furi_string_free(analytics_time);
        }
        furi_string_free(announcement_json);
        furi_string_free(analytics_json);
        furi_string_free(notification);
        return false;
    }

    // Load previous announcement and analytics times
    char past_analytics_time[32] = {0};
    char past_announcement_time[32] = {0};
    bool analytics_time_loaded = load_char("analytics_time", past_analytics_time, sizeof(past_analytics_time));
    bool announcement_time_loaded = load_char("announcement_time", past_announcement_time, sizeof(past_announcement_time));

    bool new_announcement = false;
    bool new_analytics = false;

    // Check for new announcement
    if (!announcement_time_loaded || strcmp(furi_string_get_cstr(announcement_time), past_announcement_time) != 0)
    {
        new_announcement = true;
    }

    // Check for new analytics
    if (!analytics_time_loaded || strcmp(furi_string_get_cstr(analytics_time), past_analytics_time) != 0)
    {
        new_analytics = true;
    }

    // If no new announcement and no new analytics, exit early
    if (!new_announcement && !new_analytics)
    {
        FURI_LOG_D(TAG, "No new announcement or analytics");
        furi_string_free(announcement_value);
        furi_string_free(announcement_time);
        furi_string_free(analytics_value);
        furi_string_free(analytics_time);
        furi_string_free(announcement_json);
        furi_string_free(analytics_json);
        furi_string_free(notification);
        return true;
    }

    // Save the new announcement and analytics times if they are new
    if (new_announcement)
    {
        save_char("announcement_time", furi_string_get_cstr(announcement_time));
    }

    if (new_analytics)
    {
        save_char("analytics_time", furi_string_get_cstr(analytics_time));
    }

    // Prepare and show dialogs based on what is new
    if (new_announcement)
    {
        easy_flipper_dialog("Announcement", (char *)furi_string_get_cstr(announcement_value));
    }

    if (new_analytics)
    {
        char analytics_text[128] = {0};
        // Determine the new posts count
        if (atoi(furi_string_get_cstr(analytics_value)) > 0)
        {
            char past_analytics_value[32] = {0};
            int new_posts = 0;
            if (load_char("analytics_value", past_analytics_value, sizeof(past_analytics_value)))
            {
                int past_posts = atoi(past_analytics_value);
                int current_posts = atoi(furi_string_get_cstr(analytics_value));
                new_posts = current_posts - past_posts;
                snprintf(analytics_text, sizeof(analytics_text), "%d new posts", new_posts);
            }
            else
            {
                snprintf(analytics_text, sizeof(analytics_text), "%s feed posts", furi_string_get_cstr(analytics_value));
            }
            save_char("analytics_value", furi_string_get_cstr(analytics_value));
        }
        else
        {
            snprintf(analytics_text, sizeof(analytics_text), "%s feed posts", furi_string_get_cstr(analytics_value));
        }

        easy_flipper_dialog("Notifications", analytics_text);
    }

    // Free allocated resources
    furi_string_free(announcement_value);
    furi_string_free(announcement_time);
    furi_string_free(analytics_value);
    furi_string_free(analytics_time);
    furi_string_free(announcement_json);
    furi_string_free(analytics_json);
    furi_string_free(notification);

    return true;
}

// home notification
bool flip_social_home_notification()
{
    // return flipper_http_process_response_async(flip_social_get_home_notification, flip_social_parse_home_notification);
    return true; // for now
}