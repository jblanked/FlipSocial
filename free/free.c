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
        // flipper_http_deinit();
        went_to_friends = false;
    }

    // free Derek's loader
    loader_view_free(app);
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
void free_explore_dialog()
{
    if (app_instance->dialog_explore)
    {
        dialog_ex_free(app_instance->dialog_explore);
        app_instance->dialog_explore = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewExploreDialog);
    }
}
void free_friends_dialog()
{
    if (app_instance->dialog_friends)
    {
        dialog_ex_free(app_instance->dialog_friends);
        app_instance->dialog_friends = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewFriendsDialog);
    }
}
void free_messages_dialog()
{
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
    if (app_instance->dialog_compose)
    {
        dialog_ex_free(app_instance->dialog_compose);
        app_instance->dialog_compose = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewComposeDialog);
    }
}
void free_feed_view()
{
    if (app_instance->view_feed)
    {
        view_free(app_instance->view_feed);
        app_instance->view_feed = NULL;
        view_dispatcher_remove_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
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