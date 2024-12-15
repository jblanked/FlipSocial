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
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Password", app_instance->change_password_logged_in_temp_buffer, app_instance->change_password_logged_in_temp_buffer_size, flip_social_logged_in_profile_change_password_updated, flip_social_callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
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
