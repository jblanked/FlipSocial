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

static void flip_social_request_error_draw(Canvas *canvas)
{
    if (canvas == NULL)
    {
        FURI_LOG_E(TAG, "flip_social_request_error_draw - canvas is NULL");
        DEV_CRASH();
        return;
    }
    if (fhttp.last_response != NULL)
    {
        if (strstr(fhttp.last_response, "[ERROR] Not connected to Wifi. Failed to reconnect.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(fhttp.last_response, "[ERROR] Failed to connect to Wifi.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] Not connected to Wifi.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(fhttp.last_response, "[ERROR] GET request failed or returned empty data.") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[ERROR] WiFi error.");
            canvas_draw_str(canvas, 0, 50, "Update your WiFi settings.");
            canvas_draw_str(canvas, 0, 60, "Press BACK to return.");
        }
        else if (strstr(fhttp.last_response, "[PONG]") != NULL)
        {
            canvas_clear(canvas);
            canvas_draw_str(canvas, 0, 10, "[STATUS]Connecting to AP...");
        }
        else
        {
            canvas_clear(canvas);
            FURI_LOG_E(TAG, "Received an error: %s", fhttp.last_response);
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
    flip_social_free_explore();
    flip_social_free_friends();
    flip_social_free_message_users();
    flip_social_free_messages();
    flip_feed_info_free();
    if (flip_feed_item)
    {
        free(flip_feed_item);
        flip_feed_item = NULL;
    }
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
    return FlipSocialViewLoggedOutLogin;
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
    return FlipSocialViewLoggedOutRegister;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutWifiSettings)
 */
uint32_t flip_social_callback_to_wifi_settings_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedOutWifiSettings;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t flip_social_callback_to_wifi_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInSettingsWifi;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t flip_social_callback_to_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInSettings;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Compose screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInCompose)
 */
uint32_t flip_social_callback_to_compose_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInCompose;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Profile screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInProfile)
 */
uint32_t flip_social_callback_to_profile_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInProfile;
}

/**
 * @brief Navigation callback to bring the user back to the Explore submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInExploreSubmenu)
 */
uint32_t flip_social_callback_to_explore_logged_in(void *context)
{
    UNUSED(context);
    flip_social_dialog_stop = false;
    last_explore_response = "";
    flip_social_dialog_shown = false;
    flip_social_explore->index = 0;
    action = ActionNone;
    return FlipSocialViewLoggedInExploreSubmenu;
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
    last_explore_response = "";
    flip_social_dialog_shown = false;
    flip_social_friends->index = 0;
    action = ActionNone;
    return FlipSocialViewLoggedInFriendsSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the Messages submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesSubmenu)
 */
uint32_t flip_social_callback_to_messages_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInMessagesSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the User Choices screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesUserChoices)
 */
uint32_t flip_social_callback_to_messages_user_choices(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedInMessagesUserChoices;
}

/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
uint32_t flip_social_callback_exit_app(void *context)
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
    case FlipSocialSubmenuLoggedInIndexMessages:
        flipper_http_loading_task(
            flip_social_get_message_users,         // get the message users
            flip_social_parse_json_message_users,  // parse the message users
            FlipSocialViewLoggedInMessagesSubmenu, // switch to the messages submenu if successful
            FlipSocialViewLoggedInSubmenu,         // switch back to the main submenu if failed
            &app->view_dispatcher);                // view dispatcher
        break;
    case FlipSocialSubmenuLoggedInIndexMessagesNewMessage:
        flipper_http_loading_task(
            flip_social_get_explore,                     // get the explore users
            flip_social_parse_json_message_user_choices, // parse the explore users
            FlipSocialViewLoggedInMessagesUserChoices,   // switch to the user choices if successful
            FlipSocialViewLoggedInSubmenu,               // switch back to the main submenu if failed
            &app->view_dispatcher);                      // view dispatcher
        break;
    case FlipSocialSubmenuLoggedInIndexFeed:
        if (fhttp.state == INACTIVE)
        {
            FURI_LOG_E(TAG, "HTTP state is INACTIVE");
            return;
        }
        if (!easy_flipper_set_loading(&app->loading, FlipSocialViewLoading, flip_social_callback_to_submenu_logged_in, &app->view_dispatcher))
        {
            FURI_LOG_E(TAG, "Failed to set loading screen");
            return; // already on the submenu
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoading);
        if (flip_social_get_feed()) // start the async request
        {
            furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
            fhttp.state = RECEIVING;
        }
        else
        {
            FURI_LOG_E(HTTP_TAG, "Failed to send request");
            fhttp.state = ISSUE;
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSubmenu);
            view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoading);
            loading_free(app->loading);
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
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSubmenu);
            view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoading);
            loading_free(app->loading);
            return;
        }

        // load the current feed post
        if (!flip_social_load_feed_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load feed post");
            fhttp.state = ISSUE;
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSubmenu);
            view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoading);
            loading_free(app->loading);
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInFeed);
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoading);
        loading_free(app->loading);
        break;
    case FlipSocialSubmenuExploreIndex:
        flipper_http_loading_task(
            flip_social_get_explore,              // get the explore users
            flip_social_parse_json_explore,       // parse the explore users
            FlipSocialViewLoggedInExploreSubmenu, // switch to the explore submenu if successful
            FlipSocialViewLoggedInSubmenu,        // switch back to the main submenu if failed
            &app->view_dispatcher);               // view dispatcher
        break;
    case FlipSocialSubmenuLoggedInIndexCompose:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInCompose);
        break;
    case FlipSocialSubmenuLoggedInIndexSettings:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettings);
        break;
    case FlipSocialSubmenuLoggedInSignOutButton:
        app->is_logged_in = "false";

        save_settings(app->wifi_ssid_logged_out, app->wifi_password_logged_out, app->login_username_logged_out, app->login_username_logged_in, app->login_password_logged_out, app->change_password_logged_in, app->is_logged_in);

        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        break;
    case FlipSocialSubmenuComposeIndexAddPreSave:
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInComposeAddPreSaveInput);
        break;
    default:
        action = ActionNone;
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
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInProcessCompose);
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
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInExploreProccess);
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
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInFriendsProcess);
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
            flipper_http_loading_task(
                flip_social_get_messages_with_user,    // get the messages with the selected user
                flip_social_parse_json_messages,       // parse the messages
                FlipSocialViewLoggedInMessagesProcess, // switch to the messages process if successful
                FlipSocialViewLoggedInMessagesSubmenu, // switch back to the messages submenu if failed
                &app->view_dispatcher);                // view dispatcher
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
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput);
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
        if (!flipper_http_save_wifi(app->wifi_ssid_logged_out, app->wifi_password_logged_out))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
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
        if (!flipper_http_save_wifi(app->wifi_ssid_logged_out, app->wifi_password_logged_out))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
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
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLogin);
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
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLogin);
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
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

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
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
void flip_social_logged_in_compose_pre_save_updated(void *context)
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
    auth_headers_alloc();
    char payload[256];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"old_password\":\"%s\",\"new_password\":\"%s\"}", app->login_username_logged_out, old_password, app->change_password_logged_in);
    if (!flipper_http_post_request_with_headers("https://www.flipsocial.net/api/user/change-password/", auth_headers, payload))
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
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInChangePasswordInput);
        break;
    case 2: // Friends
        flipper_http_loading_task(flip_social_get_friends, flip_social_parse_json_friends, FlipSocialViewLoggedInFriendsSubmenu, FlipSocialViewLoggedInProfile, &app->view_dispatcher);
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
void flip_social_text_input_logged_in_settings_item_selected(void *context, uint32_t index)
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
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput);
        return;
    }

    // Store the entered message
    strncpy(app->message_user_choice_logged_in, app->message_user_choice_logged_in_temp_buffer, app->message_user_choice_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->message_user_choice_logged_in[app->message_user_choice_logged_in_temp_buffer_size - 1] = '\0';

    // send post request to send message
    auth_headers_alloc();
    char url[128];
    char payload[256];
    snprintf(url, sizeof(url), "https://www.flipsocial.net/api/messages/%s/post/", app->login_username_logged_in);
    snprintf(payload, sizeof(payload), "{\"receiver\":\"%s\",\"content\":\"%s\"}", flip_social_explore->usernames[flip_social_explore->index], app->message_user_choice_logged_in);

    if (flipper_http_post_request_with_headers(url, auth_headers, payload)) // start the async request
    {
        furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        fhttp.state = RECEIVING;
    }
    else
    {
        FURI_LOG_E(TAG, "Failed to send post request to send message");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        fhttp.state = ISSUE;
        return;
    }
    while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
    {
        // Wait for the request to be received
        furi_delay_ms(10);
    }
    furi_timer_stop(fhttp.get_timeout_timer);

    // add user to the message list
    strncpy(flip_social_message_users->usernames[flip_social_message_users->count], flip_social_explore->usernames[flip_social_explore->index], strlen(flip_social_explore->usernames[flip_social_explore->index]));
    flip_social_message_users->count++;

    // redraw submenu
    flip_social_update_messages_submenu();
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesSubmenu);
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

    // send post request to send message
    auth_headers_alloc();
    char url[128];
    char payload[256];
    snprintf(url, sizeof(url), "https://www.flipsocial.net/api/messages/%s/post/", app->login_username_logged_in);
    snprintf(payload, sizeof(payload), "{\"receiver\":\"%s\",\"content\":\"%s\"}", flip_social_message_users->usernames[flip_social_message_users->index], app->messages_new_message_logged_in);

    if (flipper_http_post_request_with_headers(url, auth_headers, payload)) // start the async request
    {
        furi_timer_start(fhttp.get_timeout_timer, TIMEOUT_DURATION_TICKS);
        fhttp.state = RECEIVING;
    }
    else
    {
        FURI_LOG_E(TAG, "Failed to send post request to send message");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        fhttp.state = ISSUE;
        return;
    }
    while (fhttp.state == RECEIVING && furi_timer_is_running(fhttp.get_timeout_timer) > 0)
    {
        // Wait for the request to be received
        furi_delay_ms(10);
    }
    furi_timer_stop(fhttp.get_timeout_timer);
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesSubmenu);
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

    SerialState http_state = fhttp.state;
    DataLoaderModel *data_loader_model = (DataLoaderModel *)model;
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
        flip_social_request_error_draw(canvas);
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
        canvas_draw_str(canvas, 0, 27, "Sending...");
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
        FURI_LOG_E(TAG, "flip_social_loader_process_callback - context is NULL");
        DEV_CRASH();
        return;
    }

    FlipSocialApp *app = (FlipSocialApp *)context;
    View *view = app->view_loader;

    DataState current_data_state;
    with_view_model(view, DataLoaderModel * model, { current_data_state = model->data_state; }, false);

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
                strncpy(fhttp.last_response, "", 1);
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
        if (fhttp.state == IDLE && fhttp.last_response != NULL)
        {
            if (strstr(fhttp.last_response, "[PONG]") != NULL)
            {
                FURI_LOG_DEV(TAG, "PONG received.");
            }
            else if (strncmp(fhttp.last_response, "[SUCCESS]", 9) == 0)
            {
                FURI_LOG_DEV(TAG, "SUCCESS received. %s", fhttp.last_response ? fhttp.last_response : "NULL");
            }
            else if (strncmp(fhttp.last_response, "[ERROR]", 9) == 0)
            {
                FURI_LOG_DEV(TAG, "ERROR received. %s", fhttp.last_response ? fhttp.last_response : "NULL");
            }
            else if (strlen(fhttp.last_response) == 0)
            {
                // Still waiting on response
            }
            else
            {
                with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateReceived; }, true);
            }
        }
        else if (fhttp.state == SENDING || fhttp.state == RECEIVING)
        {
            // continue waiting
        }
        else if (fhttp.state == INACTIVE)
        {
            // inactive. try again
        }
        else if (fhttp.state == ISSUE)
        {
            with_view_model(view, DataLoaderModel * model, { model->data_state = DataStateError; }, true);
        }
        else
        {
            FURI_LOG_DEV(TAG, "Unexpected state: %d lastresp: %s", fhttp.state, fhttp.last_response ? fhttp.last_response : "NULL");
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
                FURI_LOG_DEV(TAG, "Parsed data: %s\r\ntext: %s", fhttp.last_response ? fhttp.last_response : "NULL", data_text ? data_text : "NULL");
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
                    flip_social_widget_set_text(model->data_text != NULL ? model->data_text : "Empty result", &app_instance->widget_result);
                    if (model->data_text != NULL)
                    {
                        free(model->data_text);
                        model->data_text = NULL;
                    }
                    view_set_previous_callback(widget_get_view(app_instance->widget_result), model->back_callback);
                    view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewWidgetResult);
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
        FURI_LOG_E(TAG, "flip_social_loader_free_model - view is NULL");
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
                free(model->parser_context);
                model->parser_context = NULL;
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
        },
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, view_id);
}
