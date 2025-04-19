#include <alloc/alloc.h>
#include <flip_storage/flip_social_storage.h>
#include <feed/feed.h>
#include <callback/callback.h>

FlipSocialApp *alloc_flip_social_app()
{
    // Initiailize the app
    FlipSocialApp *app = (FlipSocialApp *)malloc(sizeof(FlipSocialApp));

    // Initialize gui
    Gui *gui = furi_record_open(RECORD_GUI);

    // Allocate ViewDispatcher
    if (!easy_flipper_set_view_dispatcher(&app->view_dispatcher, gui, app))
    {
        return NULL;
    }

    // Allocate the text input buffers
    app->wifi_ssid_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->wifi_password_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->login_username_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->login_password_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->register_username_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->register_password_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->register_password_2_logged_out_temp_buffer_size = MAX_USER_LENGTH;
    app->change_password_logged_in_temp_buffer_size = MAX_USER_LENGTH;
    app->change_bio_logged_in_temp_buffer_size = MAX_MESSAGE_LENGTH;
    app->compose_pre_save_logged_in_temp_buffer_size = MAX_MESSAGE_LENGTH;
    app->wifi_ssid_logged_in_temp_buffer_size = MAX_USER_LENGTH;
    app->wifi_password_logged_in_temp_buffer_size = MAX_USER_LENGTH;
    app->is_logged_in_size = 8;
    app->login_username_logged_in_temp_buffer_size = MAX_USER_LENGTH;
    app->messages_new_message_logged_in_temp_buffer_size = MAX_MESSAGE_LENGTH;
    app->message_user_choice_logged_in_temp_buffer_size = MAX_MESSAGE_LENGTH;
    app->explore_logged_in_temp_buffer_size = MAX_USER_LENGTH;
    app->message_users_logged_in_temp_buffer_size = MAX_USER_LENGTH;
    if (!easy_flipper_set_buffer(&app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->register_username_logged_out_temp_buffer, app->register_username_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->register_password_logged_out_temp_buffer, app->register_password_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->register_password_2_logged_out_temp_buffer, app->register_password_2_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->change_password_logged_in_temp_buffer, app->change_password_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->change_bio_logged_in_temp_buffer, app->change_bio_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->compose_pre_save_logged_in_temp_buffer, app->compose_pre_save_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->is_logged_in, app->is_logged_in_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->login_username_logged_in_temp_buffer, app->login_username_logged_in_temp_buffer_size))
    {
        return NULL;
    }

    if (!easy_flipper_set_buffer(&app->wifi_ssid_logged_out, app->wifi_ssid_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->wifi_password_logged_out, app->wifi_password_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->login_username_logged_out, app->login_username_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->login_password_logged_out, app->login_password_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->register_username_logged_out, app->register_username_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->register_password_logged_out, app->register_password_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->register_password_2_logged_out, app->register_password_2_logged_out_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->change_password_logged_in, app->change_password_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->change_bio_logged_in, app->change_bio_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->compose_pre_save_logged_in, app->compose_pre_save_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->login_username_logged_in, app->login_username_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    //
    if (!easy_flipper_set_buffer(&app->messages_new_message_logged_in, app->messages_new_message_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->messages_new_message_logged_in_temp_buffer, app->messages_new_message_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->message_user_choice_logged_in, app->message_user_choice_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->message_user_choice_logged_in_temp_buffer, app->message_user_choice_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&selected_message, app->message_user_choice_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->explore_logged_in, app->explore_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->explore_logged_in_temp_buffer, app->explore_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->message_users_logged_in, app->message_users_logged_in_temp_buffer_size))
    {
        return NULL;
    }
    if (!easy_flipper_set_buffer(&app->message_users_logged_in_temp_buffer, app->message_users_logged_in_temp_buffer_size))
    {
        return NULL;
    }

    // Allocate Submenu(s)
    if (!easy_flipper_set_submenu(&app->submenu_logged_out, FlipSocialViewLoggedOutSubmenu, VERSION_TAG, callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_logged_in, FlipSocialViewLoggedInSubmenu, VERSION_TAG, callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }

    submenu_add_item(app->submenu_logged_out, "Login", FlipSocialSubmenuLoggedOutIndexLogin, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_out, "Register", FlipSocialSubmenuLoggedOutIndexRegister, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_out, "About", FlipSocialSubmenuLoggedOutIndexAbout, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_out, "Settings", FlipSocialSubmenuLoggedOutIndexWifiSettings, callback_submenu_choices, app);
    //
    submenu_add_item(app->submenu_logged_in, "Explore", FlipSocialSubmenuExploreIndex, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Feed", FlipSocialSubmenuLoggedInIndexFeed, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Post", FlipSocialSubmenuLoggedInIndexCompose, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Messages", FlipSocialSubmenuLoggedInIndexMessages, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Profile", FlipSocialSubmenuLoggedInIndexProfile, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Settings", FlipSocialSubmenuLoggedInIndexSettings, callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Sign Out", FlipSocialSubmenuLoggedInSignOutButton, callback_submenu_choices, app);

    // Load the settings
    if (!load_settings(app->wifi_ssid_logged_out,
                       app->wifi_ssid_logged_out_temp_buffer_size,
                       app->wifi_password_logged_out,
                       app->wifi_password_logged_out_temp_buffer_size,
                       app->login_username_logged_out,
                       app->login_username_logged_out_temp_buffer_size,
                       app->login_username_logged_in,
                       app->login_username_logged_in_temp_buffer_size,
                       app->login_password_logged_out,
                       app->login_password_logged_out_temp_buffer_size,
                       app->change_password_logged_in,
                       app->change_password_logged_in_temp_buffer_size,
                       app->change_bio_logged_in,
                       app->change_bio_logged_in_temp_buffer_size,
                       app->is_logged_in,
                       app->is_logged_in_size))

    {
        FURI_LOG_E(TAG, "Failed to load settings");

        if (app->is_logged_in == NULL)
        {
            app->is_logged_in = (char *)malloc(app->is_logged_in_size);
            app->is_logged_in = "false";
        }
        app_instance = app;
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
    }
    else
    {
        // Copy items into their temp buffers with safety checks
        if (app->wifi_ssid_logged_out && app->wifi_ssid_logged_out_temp_buffer)
        {
            strncpy(app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out, app->wifi_ssid_logged_out_temp_buffer_size - 1);
            app->wifi_ssid_logged_out_temp_buffer[app->wifi_ssid_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->wifi_password_logged_out && app->wifi_password_logged_out_temp_buffer)
        {
            strncpy(app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out, app->wifi_password_logged_out_temp_buffer_size - 1);
            app->wifi_password_logged_out_temp_buffer[app->wifi_password_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->login_username_logged_out && app->login_username_logged_out_temp_buffer)
        {
            strncpy(app->login_username_logged_out_temp_buffer, app->login_username_logged_out, app->login_username_logged_out_temp_buffer_size - 1);
            app->login_username_logged_out_temp_buffer[app->login_username_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->login_password_logged_out && app->login_password_logged_out_temp_buffer)
        {
            strncpy(app->login_password_logged_out_temp_buffer, app->login_password_logged_out, app->login_password_logged_out_temp_buffer_size - 1);
            app->login_password_logged_out_temp_buffer[app->login_password_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->register_username_logged_out && app->register_username_logged_out_temp_buffer)
        {
            strncpy(app->register_username_logged_out_temp_buffer, app->register_username_logged_out, app->register_username_logged_out_temp_buffer_size - 1);
            app->register_username_logged_out_temp_buffer[app->register_username_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->register_password_logged_out && app->register_password_logged_out_temp_buffer)
        {
            strncpy(app->register_password_logged_out_temp_buffer, app->register_password_logged_out, app->register_password_logged_out_temp_buffer_size - 1);
            app->register_password_logged_out_temp_buffer[app->register_password_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->register_password_2_logged_out && app->register_password_2_logged_out_temp_buffer)
        {
            strncpy(app->register_password_2_logged_out_temp_buffer, app->register_password_2_logged_out, app->register_password_2_logged_out_temp_buffer_size - 1);
            app->register_password_2_logged_out_temp_buffer[app->register_password_2_logged_out_temp_buffer_size - 1] = '\0';
        }
        if (app->change_password_logged_in && app->change_password_logged_in_temp_buffer)
        {
            strncpy(app->change_password_logged_in_temp_buffer, app->change_password_logged_in, app->change_password_logged_in_temp_buffer_size - 1);
            app->change_password_logged_in_temp_buffer[app->change_password_logged_in_temp_buffer_size - 1] = '\0';
        }
        if (app->change_bio_logged_in && app->change_bio_logged_in_temp_buffer)
        {
            strncpy(app->change_bio_logged_in_temp_buffer, app->change_bio_logged_in, app->change_bio_logged_in_temp_buffer_size - 1);
            app->change_bio_logged_in_temp_buffer[app->change_bio_logged_in_temp_buffer_size - 1] = '\0';
        }
        if (app->compose_pre_save_logged_in && app->compose_pre_save_logged_in_temp_buffer)
        {
            strncpy(app->compose_pre_save_logged_in_temp_buffer, app->compose_pre_save_logged_in, app->compose_pre_save_logged_in_temp_buffer_size - 1);
            app->compose_pre_save_logged_in_temp_buffer[app->compose_pre_save_logged_in_temp_buffer_size - 1] = '\0';
        }
        if (app->wifi_ssid_logged_in && app->wifi_ssid_logged_in_temp_buffer)
        {
            strncpy(app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size - 1);
            app->wifi_ssid_logged_in_temp_buffer[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';
        }
        if (app->wifi_password_logged_in && app->wifi_password_logged_in_temp_buffer)
        {
            strncpy(app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size - 1);
            app->wifi_password_logged_in_temp_buffer[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';
        }
        if (app->login_username_logged_in && app->login_username_logged_in_temp_buffer)
        {
            strncpy(app->login_username_logged_in_temp_buffer, app->login_username_logged_in, app->login_username_logged_in_temp_buffer_size - 1);
            app->login_username_logged_in_temp_buffer[app->login_username_logged_in_temp_buffer_size - 1] = '\0';
        }

        // if login username is empty but logged out isnt, copy it over
        if (strlen(app->login_username_logged_out) > 0 && strlen(app->login_username_logged_in) == 0)
        {
            strncpy(app->login_username_logged_in, app->login_username_logged_out, app->login_username_logged_in_temp_buffer_size - 1);
            strncpy(app->login_username_logged_in_temp_buffer, app->login_username_logged_out, app->login_username_logged_in_temp_buffer_size - 1);
            app->login_username_logged_in[app->login_username_logged_in_temp_buffer_size - 1] = '\0';
            app->login_username_logged_in_temp_buffer[app->login_username_logged_in_temp_buffer_size - 1] = '\0';
        }
        // logout username is empty but logged in isnt, copy it over
        if (strlen(app->login_username_logged_in) > 0 && strlen(app->login_username_logged_out) == 0)
        {
            strncpy(app->login_username_logged_out, app->login_username_logged_in, app->login_username_logged_in_temp_buffer_size - 1);
            strncpy(app->login_username_logged_out_temp_buffer, app->login_username_logged_in, app->login_username_logged_in_temp_buffer_size - 1);
            app->login_username_logged_out[app->login_username_logged_in_temp_buffer_size - 1] = '\0';
            app->login_username_logged_out_temp_buffer[app->login_username_logged_in_temp_buffer_size - 1] = '\0';
        }
        // if login password is empty but logged out isnt, copy it over
        if (strlen(app->login_password_logged_out) > 0 && strlen(app->change_password_logged_in) == 0)
        {
            strncpy(app->change_password_logged_in, app->login_password_logged_out, app->login_password_logged_out_temp_buffer_size - 1);
            strncpy(app->change_password_logged_in_temp_buffer, app->login_password_logged_out, app->login_password_logged_out_temp_buffer_size - 1);
            app->change_password_logged_in[app->login_password_logged_out_temp_buffer_size - 1] = '\0';
            app->change_password_logged_in_temp_buffer[app->login_password_logged_out_temp_buffer_size - 1] = '\0';
        }
        // if logout password is empty but logged in isnt, copy it over
        if (strlen(app->change_password_logged_in) > 0 && strlen(app->login_password_logged_out) == 0)
        {
            strncpy(app->login_password_logged_out, app->change_password_logged_in, app->login_password_logged_out_temp_buffer_size - 1);
            strncpy(app->login_password_logged_out_temp_buffer, app->change_password_logged_in, app->login_password_logged_out_temp_buffer_size - 1);
            app->login_password_logged_out[app->login_password_logged_out_temp_buffer_size - 1] = '\0';
            app->login_password_logged_out_temp_buffer[app->login_password_logged_out_temp_buffer_size - 1] = '\0';
        }
        // if wifi password is empty but logged out isnt, copy it over
        if (strlen(app->wifi_password_logged_out) > 0 && strlen(app->wifi_password_logged_in) == 0)
        {
            strncpy(app->wifi_password_logged_in, app->wifi_password_logged_out, app->wifi_password_logged_in_temp_buffer_size - 1);
            strncpy(app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_out, app->wifi_password_logged_in_temp_buffer_size - 1);
            app->wifi_password_logged_in[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';
            app->wifi_password_logged_in_temp_buffer[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';
        }
        // ifi wifi password is empty but logged in isnt, copy it over
        if (strlen(app->wifi_password_logged_in) > 0 && strlen(app->wifi_password_logged_out) == 0)
        {
            strncpy(app->wifi_password_logged_out, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size - 1);
            strncpy(app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size - 1);
            app->wifi_password_logged_out[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';
            app->wifi_password_logged_out_temp_buffer[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';
        }
        // if wifi ssid is empty but logged out isnt, copy it over
        if (strlen(app->wifi_ssid_logged_out) > 0 && strlen(app->wifi_ssid_logged_in) == 0)
        {
            strncpy(app->wifi_ssid_logged_in, app->wifi_ssid_logged_out, app->wifi_ssid_logged_in_temp_buffer_size - 1);
            strncpy(app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_out, app->wifi_ssid_logged_in_temp_buffer_size - 1);
            app->wifi_ssid_logged_in[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';
            app->wifi_ssid_logged_in_temp_buffer[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';
        }
        // if wifi ssid is empty but logged in isnt, copy it over
        if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_ssid_logged_out) == 0)
        {
            strncpy(app->wifi_ssid_logged_out, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size - 1);
            strncpy(app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size - 1);
            app->wifi_ssid_logged_out[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';
            app->wifi_ssid_logged_out_temp_buffer[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';
        }

        alloc_headers();

        if (app->is_logged_in != NULL && strcmp(app->is_logged_in, "true") == 0)
        {
            save_char("is_logged_in", "true");
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        }
        else
        {
            save_char("is_logged_in", "false");
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        }
    }

    app->empty_screen = empty_screen_alloc();
    view_dispatcher_add_view(app->view_dispatcher, FlipSocialViewEmpty, empty_screen_get_view(app->empty_screen));
    return app;
}

FlipSocialModel *alloc_friends_model()
{
    // Allocate memory for each username only if not already allocated
    FlipSocialModel *friends = malloc(sizeof(FlipSocialModel));
    if (friends == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for friends usernames.");
        return NULL;
    }
    return friends;
}

FlipSocialModel2 *alloc_messages()
{
    // Allocate memory for each username only if not already allocated
    FlipSocialModel2 *users = malloc(sizeof(FlipSocialModel2));
    if (users == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for message users");
        return NULL;
    }
    return users;
}

FlipSocialMessage *alloc_user_messages()
{
    // Allocate memory for each username only if not already allocated
    FlipSocialMessage *messages = malloc(sizeof(FlipSocialMessage));
    if (messages == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for messages");
        return NULL;
    }
    return messages;
}

FlipSocialModel *alloc_explore(void)
{
    // Allocate memory for each username only if not already allocated
    FlipSocialModel *explore = malloc(sizeof(FlipSocialModel));
    if (explore == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore model.");
        return NULL;
    }
    return explore;
}

void alloc_headers(void)
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

FlipSocialFeedMini *alloc_feed_info(void)
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
bool allow_messages_dialog(bool free_first)
{
    if (free_first)
    {
        free_messages_dialog();
    }
    if (!app_instance->dialog_messages)
    {
        if (!easy_flipper_set_dialog_ex(
                &app_instance->dialog_messages,
                FlipSocialViewMessagesDialog,
                flip_social_messages->usernames[flip_social_messages->index],
                0,
                0,
                alloc_format_message(flip_social_messages->messages[flip_social_messages->index]),
                0,
                10,
                flip_social_messages->index != 0 ? "Prev" : NULL,
                flip_social_messages->index != flip_social_messages->count - 1 ? "Next" : NULL,
                "Create",
                callback_message_dialog,
                callback_to_messages_logged_in,
                &app_instance->view_dispatcher,
                app_instance))
        {
            return false;
        }
        return true;
    }
    return false;
}
char *alloc_format_message(const char *user_message)
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

bool alloc_feed_view()
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
    free_feed_view();
    if (!app_instance->view_feed)
    {
        if (!easy_flipper_set_view(
                &app_instance->view_feed,
                FlipSocialViewLoggedInFeed,
                callback_feed_draw,
                callback_feed_input,
                callback_to_submenu_logged_in,
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
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter SSID", app_instance->wifi_ssid_logged_out_temp_buffer, app_instance->wifi_ssid_logged_out_temp_buffer_size, callback_logged_out_wifi_settings_ssid_updated, callback_to_wifi_settings_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutWifiSettingsPasswordInput:
            // memset(app_instance->wifi_password_logged_out_temp_buffer, 0, app_instance->wifi_password_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->wifi_password_logged_out_temp_buffer, app_instance->wifi_password_logged_out_temp_buffer_size, callback_logged_out_wifi_settings_password_updated, callback_to_wifi_settings_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutLoginUsernameInput:
            // memset(app_instance->login_username_logged_out_temp_buffer, 0, app_instance->login_username_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username", app_instance->login_username_logged_out_temp_buffer, app_instance->login_username_logged_out_temp_buffer_size, callback_logged_out_login_username_updated, callback_to_login_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutLoginPasswordInput:
            // memset(app_instance->login_password_logged_out_temp_buffer, 0, app_instance->login_password_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->login_password_logged_out_temp_buffer, app_instance->login_password_logged_out_temp_buffer_size, callback_logged_out_login_password_updated, callback_to_login_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterUsernameInput:
            memset(app_instance->register_username_logged_out_temp_buffer, 0, app_instance->register_username_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username", app_instance->register_username_logged_out_temp_buffer, app_instance->register_username_logged_out_temp_buffer_size, callback_logged_out_register_username_updated, callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterPasswordInput:
            memset(app_instance->register_password_logged_out_temp_buffer, 0, app_instance->register_password_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->register_password_logged_out_temp_buffer, app_instance->register_password_logged_out_temp_buffer_size, callback_logged_out_register_password_updated, callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedOutRegisterPassword2Input:
            memset(app_instance->register_password_2_logged_out_temp_buffer, 0, app_instance->register_password_2_logged_out_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Confirm Password", app_instance->register_password_2_logged_out_temp_buffer, app_instance->register_password_2_logged_out_temp_buffer_size, callback_logged_out_register_password_2_updated, callback_to_register_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInChangePasswordInput:
            // memset(app_instance->change_password_logged_in_temp_buffer, 0, app_instance->change_password_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Change Password", app_instance->change_password_logged_in_temp_buffer, app_instance->change_password_logged_in_temp_buffer_size, callback_logged_in_profile_change_password_updated, callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInChangeBioInput:
            // memset(app_instance->change_bio_logged_in_temp_buffer, 0, app_instance->change_bio_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Bio", app_instance->change_bio_logged_in_temp_buffer, app_instance->change_bio_logged_in_temp_buffer_size, callback_logged_in_profile_change_bio_updated, callback_to_profile_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInComposeAddPreSaveInput:
            memset(app_instance->compose_pre_save_logged_in_temp_buffer, 0, app_instance->compose_pre_save_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Pre-Save Message", app_instance->compose_pre_save_logged_in_temp_buffer, app_instance->compose_pre_save_logged_in_temp_buffer_size, callback_logged_in_compose_pre_save_updated, callback_to_compose_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInWifiSettingsSSIDInput:
            // memset(app_instance->wifi_ssid_logged_in_temp_buffer, 0, app_instance->wifi_ssid_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter SSID", app_instance->wifi_ssid_logged_in_temp_buffer, app_instance->wifi_ssid_logged_in_temp_buffer_size, callback_logged_in_wifi_settings_ssid_updated, callback_to_wifi_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInWifiSettingsPasswordInput:
            // memset(app_instance->wifi_password_logged_in_temp_buffer, 0, app_instance->wifi_password_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Password", app_instance->wifi_password_logged_in_temp_buffer, app_instance->wifi_password_logged_in_temp_buffer_size, callback_logged_in_wifi_settings_password_updated, callback_to_wifi_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessagesNewMessageInput:
            memset(app_instance->messages_new_message_logged_in_temp_buffer, 0, app_instance->messages_new_message_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Message", app_instance->messages_new_message_logged_in_temp_buffer, app_instance->messages_new_message_logged_in_temp_buffer_size, callback_logged_in_messages_new_message_updated, callback_to_messages_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput:
            memset(app_instance->message_user_choice_logged_in_temp_buffer, 0, app_instance->message_user_choice_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Message", app_instance->message_user_choice_logged_in_temp_buffer, app_instance->message_user_choice_logged_in_temp_buffer_size, callback_logged_in_messages_user_choice_message_updated, callback_to_messages_user_choices, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInExploreInput:
            memset(app_instance->explore_logged_in_temp_buffer, 0, app_instance->explore_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username or Keyword", app_instance->explore_logged_in_temp_buffer, app_instance->explore_logged_in_temp_buffer_size, callback_logged_in_explore_updated, callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            break;
        case FlipSocialViewLoggedInMessageUsersInput:
            memset(app_instance->message_users_logged_in_temp_buffer, 0, app_instance->message_users_logged_in_temp_buffer_size);
            if (!easy_flipper_set_uart_text_input(&app_instance->text_input, FlipSocialViewTextInput, "Enter Username or Keyword", app_instance->message_users_logged_in_temp_buffer, app_instance->message_users_logged_in_temp_buffer_size, callback_logged_in_message_users_updated, callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
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

bool alloc_about_widget(bool is_logged_in)
{
    if (!is_logged_in)
    {
        if (!app_instance->widget_logged_out_about)
        {
            return easy_flipper_set_widget(&app_instance->widget_logged_out_about, FlipSocialViewLoggedOutAbout, "Welcome to FlipSocial\n---\nThe social media app for\nFlipper Zero, created by\nJBlanked: www.jblanked.com/flipper\n---\nPress BACK to return.", callback_to_submenu_logged_out, &app_instance->view_dispatcher);
        }
    }
    else
    {
        if (!app_instance->widget_logged_in_about)
        {
            return easy_flipper_set_widget(&app_instance->widget_logged_in_about, FlipSocialViewLoggedInSettingsAbout, "Welcome to FlipSocial\n---\nThe social media app for\nFlipper Zero, created by\nJBlanked: www.jblanked.com/flipper\n---\nPress BACK to return.", callback_to_settings_logged_in, &app_instance->view_dispatcher);
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
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Settings", callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            submenu_add_item(app_instance->submenu, "About", FlipSocialSubmenuLoggedInIndexAbout, callback_submenu_choices, app_instance);
            submenu_add_item(app_instance->submenu, "WiFi", FlipSocialSubmenuLoggedInIndexWifiSettings, callback_submenu_choices, app_instance);
            submenu_add_item(app_instance->submenu, "User", FlipSocialSubmenuLoggedInIndexUserSettings, callback_submenu_choices, app_instance);
            break;
        case FlipSocialViewLoggedInCompose:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Create A Post", callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            submenu_add_item(app_instance->submenu, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, callback_submenu_choices, app_instance);

            // Load the playlist
            if (load_playlist(&app_instance->pre_saved_messages))
            {
                // Update the playlist submenu
                for (uint32_t i = 0; i < app_instance->pre_saved_messages.count; i++)
                {
                    if (app_instance->pre_saved_messages.messages[i][0] != '\0') // Check if the string is not empty
                    {
                        submenu_add_item(app_instance->submenu, app_instance->pre_saved_messages.messages[i], FlipSocialSubemnuComposeIndexStartIndex + i, callback_submenu_choices, app_instance);
                    }
                }
            }
            break;
        case FlipSocialViewLoggedInFriendsSubmenu:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Friends", callback_to_profile_logged_in, &app_instance->view_dispatcher))
            {
                FURI_LOG_E(TAG, "Failed to set submenu for friends");
                return false;
            }
            submenu_reset(app_instance->submenu);
            went_to_friends = true;
            break;
        case FlipSocialViewLoggedInMessagesUserChoices:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Users", callback_to_messages_logged_in, &app_instance->view_dispatcher))
            {
                FURI_LOG_E(TAG, "Failed to set submenu for user choices");
                return false;
            }
            submenu_reset(app_instance->submenu);
            break;
        case FlipSocialViewLoggedInMessagesSubmenu:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Messages", callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            break;
        case FlipSocialViewLoggedInExploreSubmenu:
            if (!easy_flipper_set_submenu(&app_instance->submenu, FlipSocialViewSubmenu, "Explore", callback_to_submenu_logged_in, &app_instance->view_dispatcher))
            {
                return false;
            }
            submenu_reset(app_instance->submenu);
            break;
        }
    }
    return true;
}
static void flip_social_feed_type_change(VariableItem *item)
{
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, flip_social_feed_type[index]);
    flip_social_feed_type_index = index;
    variable_item_set_current_value_index(item, index);

    // save the feed type
    save_char("user_feed_type", strstr(flip_social_feed_type[index], "Global") ? "global" : "friends");
}
static void flip_social_notification_type_change(VariableItem *item)
{
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, flip_social_notification_type[index]);
    flip_social_notification_type_index = index;
    variable_item_set_current_value_index(item, index);

    // save the notification type
    save_char("user_notifications", strstr(flip_social_notification_type[index], "ON") ? "on" : "off");
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
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, callback_logged_out_wifi_settings_item_selected, callback_to_submenu_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_out_wifi_settings_ssid = variable_item_list_add(app_instance->variable_item_list, "SSID", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_wifi_settings_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            if (app_instance->wifi_ssid_logged_out)
                variable_item_set_current_value_text(app_instance->variable_item_logged_out_wifi_settings_ssid, app_instance->wifi_ssid_logged_out);
            return true;
        case FlipSocialViewLoggedOutLogin:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, callback_logged_out_login_item_selected, callback_to_submenu_logged_out, &app_instance->view_dispatcher, app_instance))
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
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, callback_logged_out_register_item_selected, callback_to_submenu_logged_out, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_out_register_username = variable_item_list_add(app_instance->variable_item_list, "Username", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_register_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_register_password_2 = variable_item_list_add(app_instance->variable_item_list, "Confirm Password", 1, NULL, app_instance);
            app_instance->variable_item_logged_out_register_button = variable_item_list_add(app_instance->variable_item_list, "Register", 0, NULL, app_instance);
            return true;
        case FlipSocialViewLoggedInProfile:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, callback_logged_in_profile_item_selected, callback_to_submenu_logged_in, &app_instance->view_dispatcher, app_instance))
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
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, callback_logged_in_wifi_settings_item_selected, callback_to_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_in_wifi_settings_ssid = variable_item_list_add(app_instance->variable_item_list, "SSID", 1, NULL, app_instance);
            app_instance->variable_item_logged_in_wifi_settings_password = variable_item_list_add(app_instance->variable_item_list, "Password", 1, NULL, app_instance);
            if (app_instance->wifi_ssid_logged_in)
                variable_item_set_current_value_text(app_instance->variable_item_logged_in_wifi_settings_ssid, app_instance->wifi_ssid_logged_in);
            return true;
        case FlipSocialViewLoggedInSettingsUser:
            if (!easy_flipper_set_variable_item_list(&app_instance->variable_item_list, FlipSocialViewVariableItemList, callback_logged_in_user_settings_item_selected, callback_to_settings_logged_in, &app_instance->view_dispatcher, app_instance))
            {
                return false;
            }
            app_instance->variable_item_logged_in_user_settings_feed_type = variable_item_list_add(app_instance->variable_item_list, "Feed Type", 2, flip_social_feed_type_change, app_instance);
            app_instance->variable_item_logged_in_user_settings_notifications = variable_item_list_add(app_instance->variable_item_list, "Notifications", 2, flip_social_notification_type_change, app_instance);
            variable_item_set_current_value_text(app_instance->variable_item_logged_in_user_settings_feed_type, flip_social_feed_type[flip_social_feed_type_index]);
            variable_item_set_current_value_index(app_instance->variable_item_logged_in_user_settings_feed_type, flip_social_feed_type_index);
            variable_item_set_current_value_text(app_instance->variable_item_logged_in_user_settings_notifications, flip_social_notification_type[flip_social_notification_type_index]);
            variable_item_set_current_value_index(app_instance->variable_item_logged_in_user_settings_notifications, flip_social_notification_type_index);
            char user_feed_type[32];
            char user_notifications[32];
            if (load_char("user_feed_type", user_feed_type, sizeof(user_feed_type)))
            {
                flip_social_feed_type_index = strstr(user_feed_type, "friends") ? 1 : 0;
                variable_item_set_current_value_text(app_instance->variable_item_logged_in_user_settings_feed_type, flip_social_feed_type[flip_social_feed_type_index]);
                variable_item_set_current_value_index(app_instance->variable_item_logged_in_user_settings_feed_type, flip_social_feed_type_index);
            }
            if (load_char("user_notifications", user_notifications, sizeof(user_notifications)))
            {
                flip_social_notification_type_index = strstr(user_notifications, "on") ? 1 : 0;
                variable_item_set_current_value_text(app_instance->variable_item_logged_in_user_settings_notifications, flip_social_notification_type[flip_social_notification_type_index]);
                variable_item_set_current_value_index(app_instance->variable_item_logged_in_user_settings_notifications, flip_social_notification_type_index);
            }
            return true;
        default:
            return false;
        }
    }
    return false;
}

bool alloc_flipper_http()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (!app_instance->fhttp)
    {
        app_instance->fhttp = flipper_http_alloc();
        if (!app_instance->fhttp)
        {
            FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
            return false;
        }
        return true;
    }
    FURI_LOG_I(TAG, "FlipperHTTP already allocated");
    return false;
}