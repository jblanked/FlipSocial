#include <free/free.h>
#include <callback/loader.h>
void free_all(bool should_free_variable_item_list, bool should_free_submenu, void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    furi_check(app, "FlipSocialApp is NULL");

    if (should_free_submenu)
    {
        free_explore();
        free_submenu();
    }
    if (should_free_variable_item_list)
    {
        free_variable_item_list();
    }
    free_text_input();
    free_friends();
    free_messages();
    free_feed_view();
    free_compose_dialog();
    free_explore_dialog();
    free_friends_dialog();
    free_messages_dialog();
    flip_feed_info_free();
    free_about_widget(true);
    free_about_widget(false);

    if (went_to_friends)
    {
        went_to_friends = false;
    }

    // free Derek's loader
    loader_view_free(app);

    // free flipper_http
    free_flipper_http();
}
void free_text_input()
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->text_input)
    {
        uart_text_input_free(app_instance->text_input);
        app_instance->text_input = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewTextInput);
    }
}
void free_explore_dialog()
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->dialog_explore)
    {
        dialog_ex_free(app_instance->dialog_explore);
        app_instance->dialog_explore = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewExploreDialog);
    }
}
void free_friends_dialog()
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->dialog_friends)
    {
        dialog_ex_free(app_instance->dialog_friends);
        app_instance->dialog_friends = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewFriendsDialog);
    }
}
void free_messages_dialog()
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->dialog_messages)
    {
        dialog_ex_free(app_instance->dialog_messages);
        app_instance->dialog_messages = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewMessagesDialog);
        return;
    }
}
void free_compose_dialog()
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->dialog_compose)
    {
        dialog_ex_free(app_instance->dialog_compose);
        app_instance->dialog_compose = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewComposeDialog);
    }
}
void free_feed_view()
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->view_feed)
    {
        view_free(app_instance->view_feed);
        app_instance->view_feed = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
    }
}

void free_about_widget(bool is_logged_in)
{
    if (!app_instance)
    {
        return;
    }
    if (is_logged_in && app_instance->widget_logged_in_about)
    {
        widget_free(app_instance->widget_logged_in_about);
        app_instance->widget_logged_in_about = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSettingsAbout);
    }
    if (!is_logged_in && app_instance->widget_logged_out_about)
    {
        widget_free(app_instance->widget_logged_out_about);
        app_instance->widget_logged_out_about = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedOutAbout);
    }
}

void free_friends(void)
{
    if (!flip_social_friends)
    {
        return;
    }
    free(flip_social_friends);
    flip_social_friends = NULL;
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

void free_variable_item_list(void)
{
    if (app_instance->variable_item_list)
    {
        variable_item_list_free(app_instance->variable_item_list);
        app_instance->variable_item_list = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewVariableItemList);
    }
}

void free_submenu(void)
{
    if (!app_instance)
    {
        return;
    }
    if (app_instance->submenu)
    {
        submenu_free(app_instance->submenu);
        app_instance->submenu = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewSubmenu);
    }
}

void free_message_users()
{
    if (flip_social_message_users == NULL)
    {
        return;
    }
    free(flip_social_message_users);
    flip_social_message_users = NULL;
}

void free_messages()
{
    if (flip_social_messages == NULL)
    {
        return;
    }
    free(flip_social_messages);
    flip_social_messages = NULL;
}

void free_explore(void)
{
    if (flip_social_explore)
    {
        free(flip_social_explore);
        flip_social_explore = NULL;
    }
}

void free_flipper_http()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    if (app_instance->fhttp)
    {
        flipper_http_free(app_instance->fhttp);
        app_instance->fhttp = NULL;
    }
}

/**
 * @brief Function to free the resources used by FlipSocialApp.
 * @details Cleans up all allocated resources before exiting the application.
 * @param app The FlipSocialApp object to free.
 * @return void
 */
void free_flip_social_app(FlipSocialApp *app)
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

    free_all(true, true, app);

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