#include "flip_social.h"

FlipSocialModel *flip_social_friends = NULL;        // Store the friends
FlipSocialModel2 *flip_social_message_users = NULL; // Store the users that have sent messages to the logged in user
FlipSocialModel *flip_social_explore = NULL;        // Store the users to explore
FlipSocialMessage *flip_social_messages = NULL;     // Store the messages between the logged in user and the selected user
FlipSocialFeedMini *flip_feed_info = NULL;          // Store the feed info
FlipSocialFeedItem *flip_feed_item = NULL;          // Store a feed item
FlipSocialApp *app_instance = NULL;

bool flip_social_sent_login_request = false;
bool flip_social_sent_register_request = false;
bool flip_social_login_success = false;
bool flip_social_register_success = false;
bool flip_social_dialog_shown = false;
bool flip_social_dialog_stop = false;
bool flip_social_send_message = false;
char *last_explore_response = NULL;
char *selected_message = NULL;

char auth_headers[256] = {0};

void flip_social_loader_free_model(View *view);

/**
 * @brief Function to free the resources used by FlipSocialApp.
 * @details Cleans up all allocated resources before exiting the application.
 * @param app The FlipSocialApp object to free.
 * @return void
 */
void flip_social_app_free(FlipSocialApp *app)
{
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    if (!app->view_dispatcher)
    {
        FURI_LOG_E(TAG, "ViewDispatcher is NULL");
        return;
    }

    // Free Submenu(s)
    if (app->submenu_logged_out)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        submenu_free(app->submenu_logged_out);
    }
    if (app->submenu_logged_in)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        submenu_free(app->submenu_logged_in);
    }
    //
    if (app->submenu_messages_user_choices)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesUserChoices);
        submenu_free(app->submenu_messages_user_choices);
    }

    // Free Variable Item List(s)
    if (app->variable_item_list_logged_out_wifi_settings)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettings);
        variable_item_list_free(app->variable_item_list_logged_out_wifi_settings);
    }
    if (app->variable_item_list_logged_out_login)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutLogin);
        variable_item_list_free(app->variable_item_list_logged_out_login);
    }
    if (app->variable_item_list_logged_out_register)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutRegister);
        variable_item_list_free(app->variable_item_list_logged_out_register);
    }
    if (app->variable_item_list_logged_in_profile)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInProfile);
        variable_item_list_free(app->variable_item_list_logged_in_profile);
    }
    if (app->variable_item_list_logged_in_settings)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInSettings);
        variable_item_list_free(app->variable_item_list_logged_in_settings);
    }
    if (app->variable_item_list_logged_in_settings_wifi)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsWifi);
        variable_item_list_free(app->variable_item_list_logged_in_settings_wifi);
    }

    // Free Text Input(s)
    if (app->text_input_logged_out_wifi_settings_ssid)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsSSIDInput);
        uart_text_input_free(app->text_input_logged_out_wifi_settings_ssid);
    }
    if (app->text_input_logged_out_wifi_settings_password)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsPasswordInput);
        uart_text_input_free(app->text_input_logged_out_wifi_settings_password);
    }
    if (app->text_input_logged_out_login_username)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginUsernameInput);
        uart_text_input_free(app->text_input_logged_out_login_username);
    }
    if (app->text_input_logged_out_login_password)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginPasswordInput);
        uart_text_input_free(app->text_input_logged_out_login_password);
    }
    if (app->text_input_logged_out_register_username)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterUsernameInput);
        uart_text_input_free(app->text_input_logged_out_register_username);
    }
    if (app->text_input_logged_out_register_password)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterPasswordInput);
        uart_text_input_free(app->text_input_logged_out_register_password);
    }
    if (app->text_input_logged_out_register_password_2)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedOutRegisterPassword2Input);
        uart_text_input_free(app->text_input_logged_out_register_password_2);
    }
    if (app->text_input_logged_in_change_password)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInChangePasswordInput);
        uart_text_input_free(app->text_input_logged_in_change_password);
    }
    if (app->text_input_logged_in_compose_pre_save_input)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInComposeAddPreSaveInput);
        uart_text_input_free(app->text_input_logged_in_compose_pre_save_input);
    }
    if (app->text_input_logged_in_wifi_settings_ssid)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsSSIDInput);
        uart_text_input_free(app->text_input_logged_in_wifi_settings_ssid);
    }
    if (app->text_input_logged_in_wifi_settings_password)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInWifiSettingsPasswordInput);
        uart_text_input_free(app->text_input_logged_in_wifi_settings_password);
    }
    if (app->text_input_logged_in_messages_new_message)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageInput);
        uart_text_input_free(app->text_input_logged_in_messages_new_message);
    }
    if (app->text_input_logged_in_messages_new_message_user_choices)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput);
        uart_text_input_free(app->text_input_logged_in_messages_new_message_user_choices);
    }

    // Free Widget(s)
    if (app->widget_result)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewWidgetResult);
        widget_free(app->widget_result);
    }

    // Free View(s)
    if (app->view_loader)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoader);
        flip_social_loader_free_model(app->view_loader);
        view_free(app->view_loader);
    }
    if (app->view_process_feed)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInFeed);
        view_free(app->view_process_feed);
    }
    if (app->view_process_compose)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInProcessCompose);
        view_free(app->view_process_compose);
    }
    if (app->view_process_messages)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesProcess);
        view_free(app->view_process_messages);
    }

    if (app->view_dispatcher)
        view_dispatcher_free(app->view_dispatcher);

    // Free the app structure members
    if (app->wifi_ssid_logged_out)
        free(app->wifi_ssid_logged_out);
    if (app->wifi_ssid_logged_out_temp_buffer)
        free(app->wifi_ssid_logged_out_temp_buffer);
    if (app->wifi_password_logged_out)
        free(app->wifi_password_logged_out);
    if (app->wifi_password_logged_out_temp_buffer)
        free(app->wifi_password_logged_out_temp_buffer);
    if (app->login_username_logged_out)
        free(app->login_username_logged_out);
    if (app->login_username_logged_out_temp_buffer)
        free(app->login_username_logged_out_temp_buffer);
    if (app->login_password_logged_out)
        free(app->login_password_logged_out);
    if (app->login_password_logged_out_temp_buffer)
        free(app->login_password_logged_out_temp_buffer);
    if (app->register_username_logged_out)
        free(app->register_username_logged_out);
    if (app->register_username_logged_out_temp_buffer)
        free(app->register_username_logged_out_temp_buffer);
    if (app->register_password_logged_out)
        free(app->register_password_logged_out);
    if (app->register_password_logged_out_temp_buffer)
        free(app->register_password_logged_out_temp_buffer);
    if (app->register_password_2_logged_out)
        free(app->register_password_2_logged_out);
    if (app->register_password_2_logged_out_temp_buffer)
        free(app->register_password_2_logged_out_temp_buffer);
    if (app->change_password_logged_in)
        free(app->change_password_logged_in);
    if (app->change_password_logged_in_temp_buffer)
        free(app->change_password_logged_in_temp_buffer);
    if (app->compose_pre_save_logged_in)
        free(app->compose_pre_save_logged_in);
    if (app->compose_pre_save_logged_in_temp_buffer)
        free(app->compose_pre_save_logged_in_temp_buffer);
    if (app->wifi_ssid_logged_in)
        free(app->wifi_ssid_logged_in);
    if (app->wifi_ssid_logged_in_temp_buffer)
        free(app->wifi_ssid_logged_in_temp_buffer);
    if (app->wifi_password_logged_in)
        free(app->wifi_password_logged_in);
    if (app->wifi_password_logged_in_temp_buffer)
        free(app->wifi_password_logged_in_temp_buffer);
    if (app->is_logged_in)
        free(app->is_logged_in);
    if (app->login_username_logged_in)
        free(app->login_username_logged_in);
    if (app->login_username_logged_in_temp_buffer)
        free(app->login_username_logged_in_temp_buffer);
    if (app->messages_new_message_logged_in)
        free(app->messages_new_message_logged_in);
    if (app->messages_new_message_logged_in_temp_buffer)
        free(app->messages_new_message_logged_in_temp_buffer);
    if (app->message_user_choice_logged_in)
        free(app->message_user_choice_logged_in);
    if (app->message_user_choice_logged_in_temp_buffer)
        free(app->message_user_choice_logged_in_temp_buffer);
    if (last_explore_response)
        free(last_explore_response);
    if (selected_message)
        free(selected_message);

    if (app->input_event && app->input_event_queue)
        furi_pubsub_unsubscribe(app->input_event_queue, app->input_event);

    // DeInit UART
    flipper_http_deinit();

    // Free the app structure
    if (app_instance)
        free(app_instance);
}

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

void flip_feed_info_free(void)
{
    if (!flip_feed_info)
    {
        return;
    }
    free(flip_feed_info);
    flip_feed_info = NULL;
}
