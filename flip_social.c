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
char *selected_message = NULL;

char auth_headers[256] = {0};
char *flip_social_feed_type[] = {"Global", "Friends"};
uint8_t flip_social_feed_type_index = 0;
char *flip_social_notification_type[] = {"OFF", "ON"};
uint8_t flip_social_notification_type_index = 0;

bool went_to_friends = false;

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

    if (app->fhttp)
        flipper_http_free(app->fhttp);

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
    if (app->change_bio_logged_in)
        free(app->change_bio_logged_in);
    if (app->compose_pre_save_logged_in)
        free(app->compose_pre_save_logged_in);
    if (app->compose_pre_save_logged_in_temp_buffer)
        free(app->compose_pre_save_logged_in_temp_buffer);
    if (app->explore_logged_in)
        free(app->explore_logged_in);
    if (app->explore_logged_in_temp_buffer)
        free(app->explore_logged_in_temp_buffer);
    if (app->message_users_logged_in)
        free(app->message_users_logged_in);
    if (app->message_users_logged_in_temp_buffer)
        free(app->message_users_logged_in_temp_buffer);
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
    if (selected_message)
        free(selected_message);
    if (app->explore_user_bio)
        free(app->explore_user_bio);

    if (app->empty_screen)
    {
        view_dispatcher_remove_view(app->view_dispatcher, FlipSocialViewEmpty);
        empty_screen_free(app->empty_screen);
        app->empty_screen = NULL;
    }

    // Free the app structure
    if (app_instance)
        free(app_instance);
}