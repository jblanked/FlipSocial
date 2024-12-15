#include <alloc/flip_social_alloc.h>

FlipSocialApp *flip_social_app_alloc()
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
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, flip_social_custom_event_callback);
    // Main view
    if (!easy_flipper_set_view(&app->view_loader, FlipSocialViewLoader, flip_social_loader_draw_callback, NULL, flip_social_callback_to_submenu_logged_out, &app->view_dispatcher, app))
    {
        return NULL;
    }
    flip_social_loader_init(app->view_loader);
    if (!easy_flipper_set_widget(&app->widget_result, FlipSocialViewWidgetResult, "", flip_social_callback_to_submenu_logged_out, &app->view_dispatcher))
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
    if (!easy_flipper_set_submenu(&app->submenu_logged_out, FlipSocialViewLoggedOutSubmenu, VERSION_TAG, flip_social_callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }
    if (!easy_flipper_set_submenu(&app->submenu_logged_in, FlipSocialViewLoggedInSubmenu, VERSION_TAG, flip_social_callback_exit_app, &app->view_dispatcher))
    {
        return NULL;
    }

    if (!easy_flipper_set_submenu(&app->submenu_messages_user_choices, FlipSocialViewLoggedInMessagesUserChoices, "Users", flip_social_callback_to_messages_logged_in, &app->view_dispatcher))
    {
        return NULL;
    }

    submenu_add_item(app->submenu_logged_out, "Login", FlipSocialSubmenuLoggedOutIndexLogin, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_out, "Register", FlipSocialSubmenuLoggedOutIndexRegister, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_out, "About", FlipSocialSubmenuLoggedOutIndexAbout, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_out, "Settings", FlipSocialSubmenuLoggedOutIndexWifiSettings, flip_social_callback_submenu_choices, app);
    //
    submenu_add_item(app->submenu_logged_in, "Explore", FlipSocialSubmenuExploreIndex, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Feed", FlipSocialSubmenuLoggedInIndexFeed, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Post", FlipSocialSubmenuLoggedInIndexCompose, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Messages", FlipSocialSubmenuLoggedInIndexMessages, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Profile", FlipSocialSubmenuLoggedInIndexProfile, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Settings", FlipSocialSubmenuLoggedInIndexSettings, flip_social_callback_submenu_choices, app);
    submenu_add_item(app->submenu_logged_in, "Sign Out", FlipSocialSubmenuLoggedInSignOutButton, flip_social_callback_submenu_choices, app);

    // Setup Variable Item List(s)
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_logged_out_wifi_settings, FlipSocialViewLoggedOutWifiSettings, flip_social_text_input_logged_out_wifi_settings_item_selected, flip_social_callback_to_submenu_logged_out, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_logged_out_login, FlipSocialViewLoggedOutLogin, flip_social_text_input_logged_out_login_item_selected, flip_social_callback_to_submenu_logged_out, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_logged_out_register, FlipSocialViewLoggedOutRegister, flip_social_text_input_logged_out_register_item_selected, flip_social_callback_to_submenu_logged_out, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_logged_in_profile, FlipSocialViewLoggedInProfile, flip_social_text_input_logged_in_profile_item_selected, flip_social_callback_to_submenu_logged_in, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_logged_in_settings, FlipSocialViewLoggedInSettings, flip_social_text_input_logged_in_settings_item_selected, flip_social_callback_to_submenu_logged_in, &app->view_dispatcher, app))
    {
        return NULL;
    }
    if (!easy_flipper_set_variable_item_list(&app->variable_item_list_logged_in_settings_wifi, FlipSocialViewLoggedInSettingsWifi, flip_social_text_input_logged_in_wifi_settings_item_selected, flip_social_callback_to_settings_logged_in, &app->view_dispatcher, app))
    {
        return NULL;
    }

    app->variable_item_logged_out_wifi_settings_ssid = variable_item_list_add(app->variable_item_list_logged_out_wifi_settings, "SSID", 1, NULL, app);
    app->variable_item_logged_out_wifi_settings_password = variable_item_list_add(app->variable_item_list_logged_out_wifi_settings, "Password", 1, NULL, app);
    //
    app->variable_item_logged_out_login_username = variable_item_list_add(app->variable_item_list_logged_out_login, "Username", 1, NULL, app);
    app->variable_item_logged_out_login_password = variable_item_list_add(app->variable_item_list_logged_out_login, "Password", 1, NULL, app);
    app->variable_item_logged_out_login_button = variable_item_list_add(app->variable_item_list_logged_out_login, "Login", 0, NULL, app);
    //
    app->variable_item_logged_out_register_username = variable_item_list_add(app->variable_item_list_logged_out_register, "Username", 1, NULL, app);
    app->variable_item_logged_out_register_password = variable_item_list_add(app->variable_item_list_logged_out_register, "Password", 1, NULL, app);
    app->variable_item_logged_out_register_password_2 = variable_item_list_add(app->variable_item_list_logged_out_register, "Confirm Password", 1, NULL, app);
    app->variable_item_logged_out_register_button = variable_item_list_add(app->variable_item_list_logged_out_register, "Register", 0, NULL, app);
    //
    app->variable_item_logged_in_profile_username = variable_item_list_add(app->variable_item_list_logged_in_profile, "Username", 1, NULL, app);
    app->variable_item_logged_in_profile_change_password = variable_item_list_add(app->variable_item_list_logged_in_profile, "Password", 1, NULL, app);
    app->variable_item_logged_in_profile_change_bio = variable_item_list_add(app->variable_item_list_logged_in_profile, "Bio", 1, NULL, app);
    app->variable_item_logged_in_profile_friends = variable_item_list_add(app->variable_item_list_logged_in_profile, "Friends", 0, NULL, app);
    //
    app->variable_item_logged_in_settings_about = variable_item_list_add(app->variable_item_list_logged_in_settings, "About", 0, NULL, app);
    app->variable_item_logged_in_settings_wifi = variable_item_list_add(app->variable_item_list_logged_in_settings, "WiFi", 0, NULL, app);
    //
    app->variable_item_logged_in_wifi_settings_ssid = variable_item_list_add(app->variable_item_list_logged_in_settings_wifi, "SSID", 1, NULL, app);
    app->variable_item_logged_in_wifi_settings_password = variable_item_list_add(app->variable_item_list_logged_in_settings_wifi, "Password", 1, NULL, app);

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

        auth_headers_alloc();

        // set variable item text (ommit the passwords)
        variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_ssid, app->wifi_ssid_logged_in);
        variable_item_set_current_value_text(app->variable_item_logged_out_wifi_settings_ssid, app->wifi_ssid_logged_out);
        variable_item_set_current_value_text(app->variable_item_logged_out_login_username, app->login_username_logged_out);
        variable_item_set_current_value_text(app->variable_item_logged_in_profile_username, app->login_username_logged_in);
        variable_item_set_current_value_text(app->variable_item_logged_in_profile_change_bio, app->change_bio_logged_in);
        //

        if (app->is_logged_in != NULL && strcmp(app->is_logged_in, "true") == 0)
        {
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        }
        else
        {
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        }
    }

    return app;
}