#include <alloc/alloc.h>
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
                flip_social_messages->messages[flip_social_messages->index],
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
static char *updated_user_message(const char *user_message)
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

bool feed_dialog_alloc()
{
    if (!flip_feed_item)
    {
        FURI_LOG_E(TAG, "Feed item is NULL");
        return false;
    }
    flip_social_free_feed_dialog();
    if (!app_instance->dialog_feed)
    {
        char updated_message[MAX_MESSAGE_LENGTH + 10];
        snprintf(updated_message, MAX_MESSAGE_LENGTH + 10, "%s (%u %s)", flip_feed_item->message, flip_feed_item->flips, flip_feed_item->flips == 1 ? "flip" : "flips");
        char *real_message = updated_user_message(updated_message);
        if (!real_message)
        {
            FURI_LOG_E(TAG, "Failed to update the user message");
            return false;
        }
        if (!easy_flipper_set_dialog_ex(
                &app_instance->dialog_feed,
                FlipSocialViewFeedDialog,
                flip_feed_item->username,
                0,
                0,
                updated_message,
                0,
                10,
                flip_feed_info->index != 0 ? "Prev" : NULL,
                flip_feed_info->index != flip_feed_info->count - 1 ? "Next" : NULL,
                flip_feed_item->is_flipped ? "Unflip" : "Flip",
                feed_dialog_callback,
                flip_social_callback_to_submenu_logged_in,
                &app_instance->view_dispatcher,
                app_instance))
        {
            free(real_message);
            return false;
        }
        free(real_message);
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
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter SSID", app_instance->wifi_ssid_logged_out_temp_buffer, app_instance->wifi_ssid_logged_out_temp_buffer_size, flip_social_logged_out_wifi_settings_ssid_updated, flip_social_callback_to_wifi_settings_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutWifiSettingsPasswordInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->wifi_password_logged_out_temp_buffer, app_instance->wifi_password_logged_out_temp_buffer_size, flip_social_logged_out_wifi_settings_password_updated, flip_social_callback_to_wifi_settings_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutLoginUsernameInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username", app_instance->login_username_logged_out_temp_buffer, app_instance->login_username_logged_out_temp_buffer_size, flip_social_logged_out_login_username_updated, flip_social_callback_to_login_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutLoginPasswordInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->login_password_logged_out_temp_buffer, app_instance->login_password_logged_out_temp_buffer_size, flip_social_logged_out_login_password_updated, flip_social_callback_to_login_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterUsernameInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username", app_instance->register_username_logged_out_temp_buffer, app_instance->register_username_logged_out_temp_buffer_size, flip_social_logged_out_register_username_updated, flip_social_callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterPasswordInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->register_password_logged_out_temp_buffer, app_instance->register_password_logged_out_temp_buffer_size, flip_social_logged_out_register_password_updated, flip_social_callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterPassword2Input:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Confirm Password", app_instance->register_password_2_logged_out_temp_buffer, app_instance->register_password_2_logged_out_temp_buffer_size, flip_social_logged_out_register_password_2_updated, flip_social_callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInChangePasswordInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Change Password", app_instance->change_password_logged_in_temp_buffer, app_instance->change_password_logged_in_temp_buffer_size, flip_social_logged_in_profile_change_password_updated, flip_social_callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInChangeBioInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Bio", app_instance->change_bio_logged_in_temp_buffer, app_instance->change_bio_logged_in_temp_buffer_size, flip_social_logged_in_profile_change_bio_updated, flip_social_callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInComposeAddPreSaveInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Pre-Save Message", app_instance->compose_pre_save_logged_in_temp_buffer, app_instance->compose_pre_save_logged_in_temp_buffer_size, flip_social_logged_in_compose_pre_save_updated, flip_social_callback_to_compose_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInWifiSettingsSSIDInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter SSID", app_instance->wifi_ssid_logged_in_temp_buffer, app_instance->wifi_ssid_logged_in_temp_buffer_size, flip_social_logged_in_wifi_settings_ssid_updated, flip_social_callback_to_wifi_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInWifiSettingsPasswordInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->wifi_password_logged_in_temp_buffer, app_instance->wifi_password_logged_in_temp_buffer_size, flip_social_logged_in_wifi_settings_password_updated, flip_social_callback_to_wifi_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessagesNewMessageInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Message", app_instance->messages_new_message_logged_in_temp_buffer, app_instance->messages_new_message_logged_in_temp_buffer_size, flip_social_logged_in_messages_new_message_updated, flip_social_callback_to_messages_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Message", app_instance->message_user_choice_logged_in_temp_buffer, app_instance->message_user_choice_logged_in_temp_buffer_size, flip_social_logged_in_messages_user_choice_message_updated, flip_social_callback_to_messages_user_choices, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInExploreInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username or Keyword", app_instance->explore_logged_in_temp_buffer, app_instance->explore_logged_in_temp_buffer_size, flip_social_logged_in_explore_updated, flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessageUsersInput:
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username or Keyword", app_instance->message_users_logged_in_temp_buffer, app_instance->message_users_logged_in_temp_buffer_size, flip_social_logged_in_message_users_updated, flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
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
bool pre_saved_messages_alloc(void)
{
    if (!app_instance)
    {
        return false;
    }
    if (!app_instance->submenu_compose)
    {
        if (!easy_flipper_set_submenu(&app_instance->submenu_compose, FlipSocialViewLoggedInCompose, "Create A Post", flip_social_callback_to_submenu_logged_in, &app_instance->view_dispatcher))
        {
            return false;
        }
        submenu_reset(app_instance->submenu_compose);
        submenu_add_item(app_instance->submenu_compose, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, flip_social_callback_submenu_choices, app_instance);

        // Load the playlist
        if (load_playlist(&app_instance->pre_saved_messages))
        {
            // Update the playlist submenu
            for (uint32_t i = 0; i < app_instance->pre_saved_messages.count; i++)
            {
                if (app_instance->pre_saved_messages.messages[i][0] != '\0') // Check if the string is not empty
                {
                    submenu_add_item(app_instance->submenu_compose, app_instance->pre_saved_messages.messages[i], FlipSocialSubemnuComposeIndexStartIndex + i, flip_social_callback_submenu_choices, app_instance);
                }
            }
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
    }
    return false;
}
