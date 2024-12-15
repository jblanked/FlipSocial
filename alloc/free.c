#include <alloc/free.h>
void free_all()
{
    free_text_input();
    flip_social_free_friends();
    flip_social_free_messages();
    flip_social_free_explore();
    flip_social_free_feed_dialog();
}
void free_text_input()
{
    if (app_instance->text_input)
    {
        uart_text_input_free(app_instance->text_input);
        app_instance->text_input = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewTextInput);
    }
}
void flip_social_free_explore_dialog()
{
    if (app_instance->dialog_explore)
    {
        dialog_ex_free(app_instance->dialog_explore);
        app_instance->dialog_explore = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewExploreDialog);
    }
}
void flip_social_free_friends_dialog()
{
    if (app_instance->dialog_friends)
    {
        dialog_ex_free(app_instance->dialog_friends);
        app_instance->dialog_friends = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewFriendsDialog);
    }
}
void flip_social_free_messages_dialog()
{
    if (app_instance->dialog_messages)
    {
        dialog_ex_free(app_instance->dialog_messages);
        app_instance->dialog_messages = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewMessagesDialog);
        return;
    }
}
void flip_social_free_compose_dialog()
{
    if (app_instance->dialog_compose)
    {
        dialog_ex_free(app_instance->dialog_compose);
        app_instance->dialog_compose = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewComposeDialog);
    }
}
void flip_social_free_feed_dialog()
{
    if (app_instance->dialog_feed)
    {
        dialog_ex_free(app_instance->dialog_feed);
        app_instance->dialog_feed = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewFeedDialog);
    }
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
void free_about_widget(bool is_logged_in)
{
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

void free_pre_saved_messages(void)
{
    if (app_instance->submenu_compose)
    {
        submenu_free(app_instance->submenu_compose);
        app_instance->submenu_compose = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedInCompose);
    }
}

void flip_social_free_friends(void)
{
    if (!flip_social_friends)
    {
        return;
    }
    free(flip_social_friends);
    flip_social_friends = NULL;
    if (app_instance->submenu_friends)
    {
        submenu_free(app_instance->submenu_friends);
        app_instance->submenu_friends = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFriendsSubmenu);
    }
}
