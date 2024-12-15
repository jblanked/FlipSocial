#include <alloc/free.h>
void free_all(bool should_free_variable_item_list)
{
    free_text_input();
    flip_social_free_friends();
    flip_social_free_messages();
    flip_social_free_explore();
    flip_social_free_feed_dialog();
    flip_social_free_compose_dialog();
    flip_social_free_explore_dialog();
    flip_social_free_friends_dialog();
    flip_social_free_messages_dialog();
    flip_feed_info_free();
    free_pre_saved_messages();
    free_about_widget(true);
    free_about_widget(false);
    if (should_free_variable_item_list)
        free_variable_item_list();
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