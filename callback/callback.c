#include <callback/callback.h>
#include <callback/loader.h>
#include <messages/messages.h>
#include <friends/friends.h>
#include <explore/explore.h>
#include <feed/feed.h>
#include <flip_storage/flip_social_storage.h>
#include <free/free.h>
#include <alloc/alloc.h>

// for callback_draw_user_message
#define MAX_LINES 6
#define LINE_HEIGHT 8
#define MAX_LINE_WIDTH_PX 128
#define TEMP_BUF_SIZE 128
//

void callback_loading_task(FlipperHTTP *fhttp,
                           LoadingCallback http_request,
                           LoadingCallback parse_response,
                           uint32_t success_view_id,
                           uint32_t failure_view_id,
                           ViewDispatcher **view_dispatcher,
                           bool should_free_loader)
{
    if (!fhttp)
    {
        FURI_LOG_E(HTTP_TAG, "Failed to get context.");
        return;
    }
    if (fhttp->state == INACTIVE)
    {
        view_dispatcher_switch_to_view(*view_dispatcher, failure_view_id);
        return;
    }
    int32_t loading_view_id = 387654321; // Random ID

    // free the loading view if it exists
    if (loading_global)
    {
        loading_free(loading_global);
        loading_global = NULL;
    }

    loading_global = loading_alloc();
    if (!loading_global)
    {
        FURI_LOG_E(HTTP_TAG, "Failed to allocate loading");
        view_dispatcher_switch_to_view(*view_dispatcher, failure_view_id);
        return;
    }

    view_dispatcher_add_view(*view_dispatcher, loading_view_id, loading_get_view(loading_global));

    // Switch to the loading view
    view_dispatcher_switch_to_view(*view_dispatcher, loading_view_id);

    // Make the request
    if (!callback_request_await(fhttp, http_request, parse_response))
    {
        FURI_LOG_E(HTTP_TAG, "Failed to make request");
        view_dispatcher_switch_to_view(*view_dispatcher, failure_view_id);
        view_dispatcher_remove_view(*view_dispatcher, loading_view_id);
        if (should_free_loader)
        {
            loading_free(loading_global);
            loading_global = NULL;
        }
        return;
    }

    view_dispatcher_switch_to_view(*view_dispatcher, success_view_id);
    view_dispatcher_remove_view(*view_dispatcher, loading_view_id);

    if (should_free_loader)
    {
        loading_free(loading_global);
        loading_global = NULL;
    }
}

bool callback_request_await(FlipperHTTP *fhttp, LoadingCallback http_request, LoadingCallback parse_response)
{
    if (!fhttp)
    {
        FURI_LOG_E(HTTP_TAG, "FlipperHTTP is NULL.");
        return false;
    }
    if (http_request(fhttp))
    {
        furi_timer_start(fhttp->get_timeout_timer, TIMEOUT_DURATION_TICKS);
        fhttp->state = RECEIVING;
    }
    else
    {
        FURI_LOG_E(HTTP_TAG, "Failed to send request");
        return false;
    }
    while (fhttp->state == RECEIVING && furi_timer_is_running(fhttp->get_timeout_timer) > 0)
    {
        furi_delay_ms(100);
    }
    furi_timer_stop(fhttp->get_timeout_timer);
    if (!parse_response(fhttp))
    {
        FURI_LOG_E(HTTP_TAG, "Failed to parse the JSON...");
        return false;
    }
    return true;
}

static bool callback_login_fetch(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "fhttp is NULL");
        return false;
    }
    if (!app_instance->login_username_logged_out || !app_instance->login_password_logged_out || strlen(app_instance->login_username_logged_out) == 0 || strlen(app_instance->login_password_logged_out) == 0)
    {
        FURI_LOG_E(TAG, "Username or password is NULL");
        easy_flipper_dialog("Login failed", "Username or password is empty.\nPress BACK to return.");
        return false;
    }
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->login_username_logged_out, app_instance->login_password_logged_out);
    alloc_headers();
    return flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/login/", auth_headers, buffer);
}

static bool callback_login_parse(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        fhttp->state = ISSUE;
        return false;
    }
    if (!fhttp->last_response)
    {
        fhttp->state = ISSUE;
        FURI_LOG_E(TAG, "last_response is NULL");
        return false;
    }
    // read response
    if (strstr(fhttp->last_response, "[SUCCESS]") != NULL || strstr(fhttp->last_response, "User found") != NULL)
    {
        app_instance->is_logged_in = "true";

        // set the logged_in_username and change_password_logged_in
        if (app_instance->login_username_logged_out)
        {
            strcpy(app_instance->login_username_logged_in, app_instance->login_username_logged_out);
        }
        if (app_instance->login_password_logged_out)
        {
            strcpy(app_instance->change_password_logged_in, app_instance->login_password_logged_out);
        }
        save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);
        FURI_LOG_I(TAG, "Login successful");
        return true;
    }
    else if (strstr(fhttp->last_response, "User not found") != NULL)
    {
        FURI_LOG_E(TAG, "User not found");
        fhttp->state = ISSUE;
        easy_flipper_dialog("Login Failed", "Please check your credentials.\nPress BACK to return.");
        return false;
    }
    else
    {
        FURI_LOG_E(TAG, "Login failed");
        fhttp->state = ISSUE;
        easy_flipper_dialog("Login Failed", "Please check your credentials.\nPress BACK to return.");
        return false;
    }
}

static bool callback_register_fetch(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "fhttp is NULL");
        return false;
    }

    // check if the username and password are valid
    if (!app_instance->register_username_logged_out || !app_instance->register_password_logged_out || strlen(app_instance->register_username_logged_out) == 0 || strlen(app_instance->register_password_logged_out) == 0)
    {
        FURI_LOG_E(TAG, "Username or password is NULL");
        fhttp->state = ISSUE;
        return false;
    }

    // check if both passwords match
    if (strcmp(app_instance->register_password_logged_out, app_instance->register_password_2_logged_out) != 0)
    {
        FURI_LOG_E(TAG, "Passwords do not match");
        fhttp->state = ISSUE;
        return false;
    }
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "{\"username\":\"%s\",\"password\":\"%s\"}", app_instance->register_username_logged_out, app_instance->register_password_logged_out);

    return flipper_http_request(fhttp, POST, "https://www.jblanked.com/flipper/api/user/register/", "{\"Content-Type\":\"application/json\"}", buffer);
}

static bool callback_register_parse(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        fhttp->state = ISSUE;
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "fhttp is NULL");
        fhttp->state = ISSUE;
        return false;
    }
    if (!fhttp->last_response)
    {
        fhttp->state = ISSUE;
        FURI_LOG_E(TAG, "last_response is NULL");
        return false;
    }
    // read response
    if (fhttp->last_response != NULL && (strstr(fhttp->last_response, "[SUCCESS]") != NULL || strstr(fhttp->last_response, "User created") != NULL))
    {
        // set the login credentials
        if (app_instance->login_username_logged_out)
        {
            app_instance->login_username_logged_out = app_instance->register_username_logged_out;
        }
        if (app_instance->login_password_logged_out)
        {
            app_instance->login_password_logged_out = app_instance->register_password_logged_out;
            app_instance->change_password_logged_in = app_instance->register_password_logged_out;
        }
        if (app_instance->login_username_logged_in)
        {
            app_instance->login_username_logged_in = app_instance->register_username_logged_out;
        }

        app_instance->is_logged_in = "true";

        // update header credentials
        alloc_headers();

        // save the credentials
        save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

        // send user to the logged in submenu
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        easy_flipper_dialog("Success", "Welcome to FlipSocial!");
        return true;
    }
    else if (strstr(fhttp->last_response, "Username or password not provided") != NULL)
    {
        fhttp->state = ISSUE;
        FURI_LOG_E(TAG, "Username or password not provided");
        easy_flipper_dialog("Error", "Please enter your credentials.\nPress BACK to return.");
        return false;
    }
    else if (strstr(fhttp->last_response, "User already exists") != NULL || strstr(fhttp->last_response, "Multiple users found") != NULL)
    {
        fhttp->state = ISSUE;
        FURI_LOG_E(TAG, "User already exists");
        easy_flipper_dialog("Error", "Registration failed...\nUsername already exists.\nPress BACK to return.");
        return false;
    }
    else
    {
        fhttp->state = ISSUE;
        FURI_LOG_E(TAG, "Registration failed");
        char error_message[128];
        snprintf(error_message, sizeof(error_message), "Registration failed...\n%s\nPress BACK to return.", fhttp->last_response);
        easy_flipper_dialog("Error", error_message);
        return false;
    }
}

/**
 * @brief Navigation callback to go back to the submenu Logged out.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutSubmenu)
 */
uint32_t callback_to_submenu_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewLoggedOutSubmenu;
}

/**
 * @brief Navigation callback to go back to the submenu Logged in.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSubmenu)
 */
uint32_t callback_to_submenu_logged_in(void *context)
{
    UNUSED(context);
    free_about_widget(false);
    //
    free_friends();
    free_message_users();
    flip_feed_info_free();
    if (flip_feed_item)
    {
        free(flip_feed_item);
        flip_feed_item = NULL;
    }
    // free the about widget if it exists
    free_about_widget(true);
    free_explore_dialog();
    free_friends_dialog();
    free_messages_dialog();
    free_compose_dialog();

    return FlipSocialViewLoggedInSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Login screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutLogin)
 */
uint32_t callback_to_login_logged_out(void *context)
{
    UNUSED(context);
    flip_social_sent_login_request = false;
    flip_social_login_success = false;
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Register screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutRegister)
 */
uint32_t callback_to_register_logged_out(void *context)
{
    UNUSED(context);
    flip_social_sent_register_request = false;
    flip_social_register_success = false;
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutWifiSettings)
 */
uint32_t callback_to_wifi_settings_logged_out(void *context)
{
    UNUSED(context);
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t callback_to_wifi_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t callback_to_settings_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Compose screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInCompose)
 */
uint32_t callback_to_compose_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Profile screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInProfile)
 */
uint32_t callback_to_profile_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewVariableItemList;
}

/**
 * @brief Navigation callback to bring the user back to the Explore submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewSubmenu)
 */
uint32_t callback_to_explore_logged_in(void *context)
{
    UNUSED(context);
    flip_social_dialog_stop = false;
    flip_social_dialog_shown = false;
    if (flip_social_explore)
    {
        flip_social_explore->index = 0;
    }
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the Friends submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInFriendsSubmenu)
 */
uint32_t callback_to_friends_logged_in(void *context)
{
    UNUSED(context);
    flip_social_dialog_stop = false;
    flip_social_dialog_shown = false;
    flip_social_friends->index = 0;
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the Messages submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesSubmenu)
 */
uint32_t callback_to_messages_logged_in(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback to bring the user back to the User Choices screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesUserChoices)
 */
uint32_t callback_to_messages_user_choices(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}

/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
uint32_t callback_exit_app(void *context)
{
    // Exit the application
    UNUSED(context);
    return VIEW_NONE;
}

static void callback_draw_user_message(Canvas *canvas, const char *user_message, int x, int y)
{
    if (!user_message)
    {
        FURI_LOG_E(TAG, "User message is NULL.");
        return;
    }

    // We will read through user_message and extract words manually
    const char *p = user_message;

    // Skip leading spaces
    while (*p == ' ')
        p++;

    char line[TEMP_BUF_SIZE];
    size_t line_len = 0;
    line[0] = '\0';
    int line_num = 0;

    while (*p && line_num < MAX_LINES)
    {
        // Find the end of the next word
        const char *word_start = p;
        while (*p && *p != ' ')
            p++;
        size_t word_len = p - word_start;

        // Extract the word into a temporary buffer
        char word[TEMP_BUF_SIZE];
        if (word_len > TEMP_BUF_SIZE - 1)
        {
            word_len = TEMP_BUF_SIZE - 1; // Just to avoid overflow if extremely large
        }
        memcpy(word, word_start, word_len);
        word[word_len] = '\0';

        // Skip trailing spaces for the next iteration
        while (*p == ' ')
            p++;

        if (word_len == 0)
        {
            // Empty word (consecutive spaces?), just continue
            continue;
        }

        // Check how the word fits into the current line
        char test_line[TEMP_BUF_SIZE + 128];
        if (line_len == 0)
        {
            // If line is empty, the line would just be this word
            strncpy(test_line, word, sizeof(test_line) - 1);
            test_line[sizeof(test_line) - 1] = '\0';
        }
        else
        {
            // If not empty, we add a space and then the word
            snprintf(test_line, sizeof(test_line), "%s %s", line, word);
        }

        uint16_t width = canvas_string_width(canvas, test_line);
        if (width <= MAX_LINE_WIDTH_PX)
        {
            // The word fits on this line
            strcpy(line, test_line);
            line_len = strlen(line);
        }
        else
        {
            // The word doesn't fit on this line
            // First, draw the current line if it's not empty
            if (line_len > 0)
            {
                canvas_draw_str_aligned(canvas, x, y + line_num * LINE_HEIGHT, AlignLeft, AlignTop, line);
                line_num++;
                if (line_num >= MAX_LINES)
                    break;
            }

            // Now we try to put the current word on a new line
            // Check if the word itself fits on an empty line
            width = canvas_string_width(canvas, word);
            if (width <= MAX_LINE_WIDTH_PX)
            {
                // The whole word fits on a new line
                strcpy(line, word);
                line_len = word_len;
            }
            else
            {
                // The word alone doesn't fit. We must truncate it.
                // We'll find the largest substring of the word that fits.
                size_t truncate_len = word_len;
                while (truncate_len > 0)
                {
                    char truncated[TEMP_BUF_SIZE];
                    strncpy(truncated, word, truncate_len);
                    truncated[truncate_len] = '\0';
                    if (canvas_string_width(canvas, truncated) <= MAX_LINE_WIDTH_PX)
                    {
                        // Found a substring that fits
                        strcpy(line, truncated);
                        line_len = truncate_len;
                        break;
                    }
                    truncate_len--;
                }

                if (line_len == 0)
                {
                    // Could not fit a single character. Skip this word.
                }
            }
        }
    }

    // Draw any remaining text in the buffer if we have lines left
    if (line_len > 0 && line_num < MAX_LINES)
    {
        canvas_draw_str_aligned(canvas, x, y + line_num * LINE_HEIGHT, AlignLeft, AlignTop, line);
    }
}

void callback_feed_draw(Canvas *canvas, void *model)
{
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    canvas_draw_str(canvas, 0, 7, flip_feed_item->username);
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
    callback_draw_user_message(canvas, flip_feed_item->message, 0, 12);
    canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
    char flip_message[32];
    snprintf(flip_message, sizeof(flip_message), "%u %s", flip_feed_item->flips, flip_feed_item->flips == 1 ? "flip" : "flips");
    canvas_draw_str(canvas, 0, 60, flip_message); // Draw the number of flips
    char flip_status[16];
    snprintf(flip_status, sizeof(flip_status), flip_feed_item->is_flipped ? "Unflip" : "Flip");
    canvas_draw_str(canvas, 32, 60, flip_status);                  // Draw the flip status
    canvas_draw_str(canvas, 64, 60, flip_feed_item->date_created); // Draw the date
}

bool callback_feed_input(InputEvent *event, void *context)
{
    UNUSED(context);
    furi_check(app_instance);

    // if back button is pressed
    if (event->type == InputTypePress && event->key == InputKeyBack)
    {
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInSubmenu);
        return true;
    }

    if (event->type == InputTypePress && event->key == InputKeyLeft) // Previous message
    {
        if (flip_feed_info->index > 0)
        {
            flip_feed_info->index--;
        }
        // switch view, free dialog, re-alloc dialog, switch back to dialog
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, BUFFER_VIEW);
        free_feed_view();
        // load feed item
        if (!feed_load_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            return false;
        }
        if (alloc_feed_view())
        {
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to allocate feed dialog");
            return false;
        }
    }
    else if (event->type == InputTypePress && event->key == InputKeyRight) // Next message
    {
        // if next message is the last message, then use flip_social_load_initial_feed
        if (flip_feed_info->index == flip_feed_info->count - 1)
        {
            char series_index[16];
            load_char("series_index", series_index, sizeof(series_index));
            flip_feed_info->series_index = atoi(series_index) + 1;
            char new_series_index[16];
            snprintf(new_series_index, sizeof(new_series_index), "%d", flip_feed_info->series_index);

            save_char("series_index", new_series_index);

            free_flipper_http();
            if (!alloc_flipper_http())
            {
                return false;
            }
            if (!feed_load_initial_feed(app_instance->fhttp, flip_feed_info->series_index))
            {
                FURI_LOG_E(TAG, "Failed to load initial feed");
                free_flipper_http();
                return false;
            }
            free_flipper_http();
            // switch view, free dialog, re-alloc dialog, switch back to dialog
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, BUFFER_VIEW);
            free_feed_view();
            // load feed item
            if (!feed_load_post(flip_feed_info->ids[flip_feed_info->index]))
            {
                FURI_LOG_E(TAG, "Failed to load nexy feed post");
                return false;
            }
            if (alloc_feed_view())
            {
                view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
            }
            else
            {
                FURI_LOG_E(TAG, "Failed to allocate feed dialog");
                return false;
            }
        }
        if (flip_feed_info->index < flip_feed_info->count - 1)
        {
            flip_feed_info->index++;
        }
        // switch view, free dialog, re-alloc dialog, switch back to dialog
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, BUFFER_VIEW);
        free_feed_view();
        // load feed item
        if (!feed_load_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            return false;
        }
        if (alloc_feed_view())
        {
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to allocate feed dialog");
            return false;
        }
    }
    else if (event->type == InputTypePress && event->key == InputKeyOk) // Flip/Unflip
    {
        // Moved to above the is_flipped check
        if (!flip_feed_item->is_flipped)
        {
            // increase the flip count
            flip_feed_item->flips++;
        }
        else
        {
            // decrease the flip count
            if (flip_feed_item->flips > 0)
                flip_feed_item->flips--;
        }
        // change the flip status
        flip_feed_item->is_flipped = !flip_feed_item->is_flipped;

        // send post request to flip the message
        if (app_instance->login_username_logged_in == NULL)
        {
            FURI_LOG_E(TAG, "Username is NULL");
            return false;
        }
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return false;
        }
        alloc_headers();
        char payload[256];
        snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"post_id\":\"%u\"}", app_instance->login_username_logged_in, flip_feed_item->id);
        if (flipper_http_request(app_instance->fhttp, POST, "https://www.jblanked.com/flipper/api/feed/flip/", auth_headers, payload))
        {
            // save feed item
            char new_save[512];
            snprintf(new_save, sizeof(new_save), "{\"id\":%u,\"username\":\"%s\",\"message\":\"%s\",\"flip_count\":%u,\"flipped\":%s,\"date_created\":\"%s\"}",
                     flip_feed_item->id, flip_feed_item->username, flip_feed_item->message, flip_feed_item->flips, flip_feed_item->is_flipped ? "true" : "false", flip_feed_item->date_created);
            char id[16];
            snprintf(id, sizeof(id), "%u", flip_feed_item->id);
            if (!flip_social_save_post(id, new_save))
            {
                FURI_LOG_E(TAG, "Failed to save the feed post");
                free_flipper_http();
                return false;
            }
        }
        free_flipper_http();
        // switch view, free dialog, re-alloc dialog, switch back to dialog
        view_dispatcher_switch_to_view(app_instance->view_dispatcher, BUFFER_VIEW);
        free_feed_view();
        // load feed item
        if (!feed_load_post(flip_feed_info->ids[flip_feed_info->index]))
        {
            FURI_LOG_E(TAG, "Failed to load nexy feed post");
            app_instance->fhttp->state = ISSUE;
            return false;
        }
        if (alloc_feed_view())
        {
            view_dispatcher_switch_to_view(app_instance->view_dispatcher, FlipSocialViewLoggedInFeed);
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to allocate feed dialog");
        }
    }
    return false;
}

void explore_dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Remove
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        // remove friend
        char remove_payload[128];
        snprintf(remove_payload, sizeof(remove_payload), "{\"username\":\"%s\",\"friend\":\"%s\"}", app_instance->login_username_logged_in, flip_social_explore->usernames[flip_social_explore->index]);
        alloc_headers();
        flipper_http_request(app->fhttp, POST, "https://www.jblanked.com/flipper/api/user/remove-friend/", auth_headers, remove_payload);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        free_explore_dialog();
        furi_delay_ms(1000);
        free_flipper_http();
    }
    else if (result == DialogExResultRight)
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        // add friend
        char add_payload[128];
        snprintf(add_payload, sizeof(add_payload), "{\"username\":\"%s\",\"friend\":\"%s\"}", app_instance->login_username_logged_in, flip_social_explore->usernames[flip_social_explore->index]);
        alloc_headers();
        flipper_http_request(app->fhttp, POST, "https://www.jblanked.com/flipper/api/user/add-friend/", auth_headers, add_payload);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        free_explore_dialog();
        furi_delay_ms(1000);
        free_flipper_http();
    }
}
static void friends_dialog_callback(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Remove
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        // remove friend
        char remove_payload[128];
        snprintf(remove_payload, sizeof(remove_payload), "{\"username\":\"%s\",\"friend\":\"%s\"}", app_instance->login_username_logged_in, flip_social_friends->usernames[flip_social_friends->index]);
        alloc_headers();
        flipper_http_request(app->fhttp, POST, "https://www.jblanked.com/flipper/api/user/remove-friend/", auth_headers, remove_payload);
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        free_friends_dialog();
        furi_delay_ms(1000);
        free_flipper_http();
    }
}
void callback_message_dialog(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Previous message
    {
        if (flip_social_messages->index > 0)
        {
            flip_social_messages->index--;
            dialog_ex_reset(app->dialog_messages);
            dialog_ex_set_header(app->dialog_messages, flip_social_messages->usernames[flip_social_messages->index], 0, 0, AlignLeft, AlignTop);
            dialog_ex_set_text(app->dialog_messages, alloc_format_message(flip_social_messages->messages[flip_social_messages->index]), 0, 10, AlignLeft, AlignTop);
            if (flip_social_messages->index != 0)
            {
                dialog_ex_set_left_button_text(app->dialog_messages, "Prev");
            }
            dialog_ex_set_right_button_text(app->dialog_messages, "Next");
            dialog_ex_set_center_button_text(app->dialog_messages, "Create");
            // switch view, free dialog, re-alloc dialog, switch back to dialog
            view_dispatcher_switch_to_view(app->view_dispatcher, BUFFER_VIEW);
            free_messages_dialog();
            allow_messages_dialog(false);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewMessagesDialog);
        }
    }
    else if (result == DialogExResultRight) // Next message
    {
        if (flip_social_messages->index < flip_social_messages->count - 1)
        {
            flip_social_messages->index++;
            dialog_ex_reset(app->dialog_messages);
            dialog_ex_set_header(app->dialog_messages, flip_social_messages->usernames[flip_social_messages->index], 0, 0, AlignLeft, AlignTop);
            dialog_ex_set_text(app->dialog_messages, alloc_format_message(flip_social_messages->messages[flip_social_messages->index]), 0, 10, AlignLeft, AlignTop);
            dialog_ex_set_left_button_text(app->dialog_messages, "Prev");
            if (flip_social_messages->index != flip_social_messages->count - 1)
            {
                dialog_ex_set_right_button_text(app->dialog_messages, "Next");
            }
            dialog_ex_set_center_button_text(app->dialog_messages, "Create");
            // switch view, free dialog, re-alloc dialog, switch back to dialog
            view_dispatcher_switch_to_view(app->view_dispatcher, BUFFER_VIEW);
            free_messages_dialog();
            allow_messages_dialog(false);
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewMessagesDialog);
        }
    }
    else if (result == DialogExResultCenter) // new message
    {
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInMessagesNewMessageInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
    }
}

static void callback_compose_dialog(DialogExResult result, void *context)
{
    furi_assert(context);
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (result == DialogExResultLeft) // Delete
    {
        // Ensure index is within bounds
        if (app_instance->pre_saved_messages.index >= app_instance->pre_saved_messages.count)
        {
            FURI_LOG_E(TAG, "Invalid index for deletion: %zu", app_instance->pre_saved_messages.index);
            return;
        }

        // Shift messages to remove the selected message
        for (size_t i = app_instance->pre_saved_messages.index; i < app_instance->pre_saved_messages.count - 1; i++)
        {
            strncpy(app_instance->pre_saved_messages.messages[i],
                    app_instance->pre_saved_messages.messages[i + 1],
                    MAX_MESSAGE_LENGTH);
        }

        // Clear the last message after shifting
        memset(app_instance->pre_saved_messages.messages[app_instance->pre_saved_messages.count - 1], 0, MAX_MESSAGE_LENGTH);
        app_instance->pre_saved_messages.count--;

        // Reset and rebuild the submenu
        submenu_reset(app_instance->submenu);
        submenu_add_item(app_instance->submenu, "Add Pre-Save", FlipSocialSubmenuComposeIndexAddPreSave, callback_submenu_choices, app);

        for (size_t i = 0; i < app_instance->pre_saved_messages.count; i++)
        {
            submenu_add_item(app_instance->submenu,
                             app_instance->pre_saved_messages.messages[i],
                             FlipSocialSubemnuComposeIndexStartIndex + i,
                             callback_submenu_choices,
                             app);
        }

        // Save the updated playlist
        save_playlist(&app_instance->pre_saved_messages);

        // Switch back to the compose view
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);

        // Free the dialog resources
        free_compose_dialog();
    }

    else if (result == DialogExResultRight) // Post
    {
        // post the message
        // send selected_message
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        if (selected_message && app_instance->login_username_logged_in)
        {
            if (strlen(selected_message) > MAX_MESSAGE_LENGTH)
            {
                FURI_LOG_E(TAG, "Message is too long");
                free_flipper_http();
                return;
            }
            // Send the selected_message
            char command[256];
            snprintf(command, sizeof(command), "{\"username\":\"%s\",\"content\":\"%s\"}",
                     app_instance->login_username_logged_in, selected_message);

            if (!flipper_http_request(
                    app->fhttp,
                    POST,
                    "https://www.jblanked.com/flipper/api/feed/post/",
                    auth_headers,
                    command))
            {
                FURI_LOG_E(TAG, "Failed to send HTTP request for feed");
                free_flipper_http();
                return;
            }

            app->fhttp->state = RECEIVING;
            furi_timer_start(app->fhttp->get_timeout_timer, TIMEOUT_DURATION_TICKS);
        }
        else
        {
            FURI_LOG_E(TAG, "Selected message or username is NULL");
            free_flipper_http();
            return;
        }
        while (app->fhttp->state == RECEIVING && furi_timer_is_running(app->fhttp->get_timeout_timer) > 0)
        {
            furi_delay_ms(100);
        }
        if (feed_load_initial_feed(app->fhttp, 1))
        {
            free_compose_dialog();
        }
        else
        {
            FURI_LOG_E(TAG, "Failed to load feed");
            free_flipper_http();
            return;
        }
        furi_timer_stop(app->fhttp->get_timeout_timer);
        free_flipper_http();
    }
}

static bool callback_get_user_info(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    char url[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/user/users/%s/extended/", flip_social_explore->usernames[flip_social_explore->index]);
    if (!flipper_http_request(fhttp, GET, url, auth_headers, NULL))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for user info");
        fhttp->state = ISSUE;
        return false;
    }
    fhttp->state = RECEIVING;
    return true;
}

static bool callback_parse_user_info(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    if (fhttp->last_response == NULL)
    {
        FURI_LOG_E(TAG, "Response is NULL");
        return false;
    }
    if (!app_instance->explore_user_bio)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    char *bio = get_json_value("bio", fhttp->last_response);
    char *friends = get_json_value("friends", fhttp->last_response);
    if (bio && friends)
    {
        if (strlen(bio) != 0)
        {
            snprintf(app_instance->explore_user_bio, MAX_MESSAGE_LENGTH, "%s (%s friends)", bio, friends);
        }
        else
        {
            snprintf(app_instance->explore_user_bio, MAX_MESSAGE_LENGTH, "%s friends", friends);
        }
        free(bio);
        free(friends);
        return true;
    }
    return false;
}

/**
 * @brief Handle ALL submenu item selections.
 * @param context The context - FlipSocialApp object.
 * @param index The FlipSocialSubmenuIndex item that was clicked.
 * @return void
 */
void callback_submenu_choices(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case FlipSocialSubmenuLoggedOutIndexLogin:
        flip_social_sent_login_request = false;
        flip_social_login_success = false;
        free_all(true, true, context);
        if (!alloc_variable_item_list(FlipSocialViewLoggedOutLogin))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedOutIndexRegister:
        flip_social_sent_register_request = false;
        flip_social_register_success = false;
        free_all(true, true, context);
        if (!alloc_variable_item_list(FlipSocialViewLoggedOutRegister))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedOutIndexAbout:
        if (!alloc_about_widget(false))
        {
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutAbout);
        break;
    case FlipSocialSubmenuLoggedOutIndexWifiSettings:
        free_all(false, false, app);
        if (!alloc_variable_item_list(FlipSocialViewLoggedOutWifiSettings))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInIndexProfile:
        free_all(true, true, context);
        if (!alloc_variable_item_list(FlipSocialViewLoggedInProfile))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInIndexMessages:
        free_all(true, true, context);
        if (!alloc_submenu(FlipSocialViewLoggedInMessagesSubmenu))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu");
            return;
        }
        if (!alloc_flipper_http())
        {
            return;
        }
        callback_loading_task(
            app->fhttp,
            messages_get_message_users,        // get the message users
            messages_parse_json_message_users, // parse the message users
            FlipSocialViewSubmenu,             // switch to the messages submenu if successful
            FlipSocialViewLoggedInSubmenu,     // switch back to the main submenu if failed
            &app->view_dispatcher,             // view dispatcher
            true);
        free_flipper_http();
        break;
    case FlipSocialSubmenuLoggedInIndexMessagesNewMessage:
        // they need to search for the user to send a message
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInMessageUsersInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case FlipSocialSubmenuLoggedInIndexFeed:
        free_all(true, true, context);
        if (!alloc_flipper_http())
        {
            return;
        }
        if (!feed_load_initial_feed(app->fhttp, 1))
        {
            FURI_LOG_E(TAG, "Failed to load the initial feed");
            free_flipper_http();
            return;
        }
        free_flipper_http();
        break;
    case FlipSocialSubmenuExploreIndex:
        free_all(true, true, context);
        if (!alloc_text_input(FlipSocialViewLoggedInExploreInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case FlipSocialSubmenuLoggedInIndexCompose:
        free_all(true, true, context);
        if (!alloc_submenu(FlipSocialViewLoggedInCompose))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        break;
    case FlipSocialSubmenuLoggedInIndexSettings:
        free_all(true, true, context);
        if (!alloc_submenu(FlipSocialViewLoggedInSettings))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        break;
    case FlipSocialSubmenuLoggedInIndexAbout:
        free_all(true, false, context);
        if (!alloc_about_widget(true))
        {
            FURI_LOG_E(TAG, "Failed to allocate about widget");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInSettingsAbout);
        break;
    case FlipSocialSubmenuLoggedInIndexWifiSettings:
        free_all(true, false, context);
        if (!alloc_variable_item_list(FlipSocialViewLoggedInSettingsWifi))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInIndexUserSettings:
        free_all(true, false, context);
        if (!alloc_variable_item_list(FlipSocialViewLoggedInSettingsUser))
        {
            FURI_LOG_E(TAG, "Failed to allocate variable item list");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
        break;
    case FlipSocialSubmenuLoggedInSignOutButton:
        free_all(true, true, context);
        app->is_logged_in = "false";

        save_settings(app->wifi_ssid_logged_out, app->wifi_password_logged_out, app->login_username_logged_out, app->login_username_logged_in, app->login_password_logged_out, app->change_password_logged_in, app->change_bio_logged_in, app->is_logged_in);

        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutSubmenu);
        break;
    case FlipSocialSubmenuComposeIndexAddPreSave:
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInComposeAddPreSaveInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    default:
        // Handle the pre-saved message selection (has a max of 25 items)
        if (index >= FlipSocialSubemnuComposeIndexStartIndex && index < FlipSocialSubemnuComposeIndexStartIndex + MAX_PRE_SAVED_MESSAGES)
        {
            app->pre_saved_messages.index = index - FlipSocialSubemnuComposeIndexStartIndex;
            snprintf(selected_message, MAX_MESSAGE_LENGTH, "%s", app->pre_saved_messages.messages[app->pre_saved_messages.index]);
            if (!selected_message)
            {
                FURI_LOG_E(TAG, "Selected message is NULL");
                return;
            }
            free_compose_dialog();
            if (!app->dialog_compose)
            {
                if (!easy_flipper_set_dialog_ex(
                        &app->dialog_compose,
                        FlipSocialViewComposeDialog,
                        "New Feed Post",
                        0,
                        0,
                        alloc_format_message(selected_message),
                        0,
                        10,
                        "Delete",
                        "Post",
                        NULL,
                        callback_compose_dialog,
                        callback_to_compose_logged_in,
                        &app->view_dispatcher,
                        app))
                {
                    return;
                }
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewComposeDialog);
        }

        // Handle the explore selection
        else if (index >= FlipSocialSubmenuExploreIndexStartIndex && index < FlipSocialSubmenuExploreIndexStartIndex + MAX_EXPLORE_USERS)
        {
            if (!flip_social_explore)
            {
                FURI_LOG_E(TAG, "FlipSocialExplore is NULL");
                return;
            }
            flip_social_explore->index = index - FlipSocialSubmenuExploreIndexStartIndex;
            // loading task to get the user info
            if (app->explore_user_bio)
            {
                free(app->explore_user_bio);
                app->explore_user_bio = NULL;
            }
            if (!easy_flipper_set_buffer(&app->explore_user_bio, MAX_MESSAGE_LENGTH))
            {
                return;
            }
            free_flipper_http();
            if (!alloc_flipper_http())
            {
                return;
            }
            if (callback_request_await(app->fhttp, callback_get_user_info, callback_parse_user_info))
            {
                free_explore_dialog();
                if (!app->dialog_explore)
                {
                    if (!easy_flipper_set_dialog_ex(
                            &app->dialog_explore,
                            FlipSocialViewExploreDialog,
                            flip_social_explore->usernames[flip_social_explore->index],
                            0,
                            0,
                            alloc_format_message(app->explore_user_bio),
                            0,
                            10,
                            "Remove", // remove if user is a friend (future update)
                            "Add",    // add if user is not a friend (future update)
                            NULL,
                            explore_dialog_callback,
                            callback_to_explore_logged_in,
                            &app->view_dispatcher,
                            app))
                    {
                        return;
                    }
                }
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewExploreDialog);
            }
            else
            {
                free_explore_dialog();
                if (!app->dialog_explore)
                {
                    if (!easy_flipper_set_dialog_ex(
                            &app->dialog_explore,
                            FlipSocialViewExploreDialog,
                            flip_social_explore->usernames[flip_social_explore->index],
                            0,
                            0,
                            "",
                            0,
                            10,
                            "Remove", // remove if user is a friend (future update)
                            "Add",    // add if user is not a friend (future update)
                            NULL,
                            explore_dialog_callback,
                            callback_to_explore_logged_in,
                            &app->view_dispatcher,
                            app))
                    {
                        return;
                    }
                }
                view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewExploreDialog);
            }
        }

        // handle the friends selection
        else if (index >= FlipSocialSubmenuLoggedInIndexFriendsStart && index < FlipSocialSubmenuLoggedInIndexFriendsStart + MAX_FRIENDS)
        {
            if (!flip_social_friends)
            {
                FURI_LOG_E(TAG, "FlipSocialFriends is NULL");
                return;
            }
            flip_social_friends->index = index - FlipSocialSubmenuLoggedInIndexFriendsStart;
            free_friends_dialog();
            if (!app->dialog_friends)
            {
                if (!easy_flipper_set_dialog_ex(
                        &app->dialog_friends,
                        FlipSocialViewFriendsDialog,
                        "Friend Options",
                        0,
                        0,
                        flip_social_friends->usernames[flip_social_friends->index],
                        0,
                        10,
                        "Remove",
                        "",
                        NULL,
                        friends_dialog_callback,
                        callback_to_friends_logged_in,
                        &app->view_dispatcher,
                        app))
                {
                    return;
                }
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewFriendsDialog);
        }

        // handle the messages selection
        else if (index >= FlipSocialSubmenuLoggedInIndexMessagesUsersStart && index < FlipSocialSubmenuLoggedInIndexMessagesUsersStart + MAX_MESSAGE_USERS)
        {
            if (!flip_social_message_users)
            {
                FURI_LOG_E(TAG, "FlipSocialMessageUsers is NULL");
                return;
            }
            flip_social_message_users->index = index - FlipSocialSubmenuLoggedInIndexMessagesUsersStart;
            free_flipper_http();
            if (!alloc_flipper_http())
            {
                return;
            }
            callback_loading_task(
                app->fhttp,
                messages_get_messages_with_user,       // get the messages with the selected user
                messages_parse_json_messages,          // parse the messages
                FlipSocialViewMessagesDialog,          // switch to the messages process if successful
                FlipSocialViewLoggedInMessagesSubmenu, // switch back to the messages submenu if failed
                &app->view_dispatcher,                 // view dispatcher
                true);
            free_flipper_http();
        }

        // handle the messages user choices selection
        else if (index >= FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart && index < FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart + MAX_EXPLORE_USERS)
        {
            if (!flip_social_explore)
            {
                FURI_LOG_E(TAG, "FlipSocialExplore is NULL");
                return;
            }
            flip_social_explore->index = index - FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart;
            free_text_input();
            if (!alloc_text_input(FlipSocialViewLoggedInMessagesNewMessageUserChoicesInput))
            {
                FURI_LOG_E(TAG, "Failed to allocate text input");
                return;
            } // switch to the text input view
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        }
        else
        {
            FURI_LOG_E(TAG, "Unknown submenu index");
        }

        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_wifi_settings_ssid_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->wifi_ssid_logged_out, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size);

    // Store the entered name in the logged in name field
    strncpy(app->wifi_ssid_logged_in, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size);
    strncpy(app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->wifi_ssid_logged_out[app->wifi_ssid_logged_out_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_out_wifi_settings_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_wifi_settings_ssid, app->wifi_ssid_logged_out);
    }

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_out) > 0 && strlen(app->wifi_password_logged_out) > 0)
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        if (!flipper_http_save_wifi(app->fhttp, app->wifi_ssid_logged_out, app->wifi_password_logged_out))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
        furi_delay_ms(500);
        free_flipper_http();
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_wifi_settings_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered WiFi password
    strncpy(app->wifi_password_logged_out, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);

    // Store the entered WiFi password in the logged in password field
    strncpy(app->wifi_password_logged_in, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);
    strncpy(app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_out_temp_buffer, app->wifi_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->wifi_password_logged_out[app->wifi_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_wifi_settings_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_wifi_settings_password, app->wifi_password_logged_out);
    }

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_out) > 0 && strlen(app->wifi_password_logged_out) > 0)
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        if (!flipper_http_save_wifi(app->fhttp, app->wifi_ssid_logged_out, app->wifi_password_logged_out))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
        furi_delay_ms(500);
        free_flipper_http();
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_out_wifi_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input SSID
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsSSIDInput);
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedOutWifiSettingsSSIDInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutWifiSettingsPasswordInput);
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedOutWifiSettingsPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their username on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_login_username_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->login_username_logged_out, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size);

    // Store the entered name in the logged in username field
    strncpy(app->login_username_logged_in, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size);
    strncpy(app->login_username_logged_in_temp_buffer, app->login_username_logged_out_temp_buffer, app->login_username_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->login_username_logged_out[app->login_username_logged_out_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_out_login_username)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_login_username, app->login_username_logged_out);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */

void callback_logged_out_login_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->login_password_logged_out, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size);

    // Store the entered password in the change password field
    strncpy(app->change_password_logged_in, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size);
    strncpy(app->change_password_logged_in_temp_buffer, app->login_password_logged_out_temp_buffer, app->login_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->login_password_logged_out[app->login_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_login_password)
    {
        // dont show the password on the screen (version 0.2)
        // variable_item_set_current_value_text(app->variable_item_logged_out_login_password, app->login_password_logged_out);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_out_login_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input Username
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginUsernameInput);
        free_all(false, true, app);
        if (!alloc_text_input(FlipSocialViewLoggedOutLoginUsernameInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        // view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedOutLoginPasswordInput);
        free_all(false, true, app);
        if (!alloc_text_input(FlipSocialViewLoggedOutLoginPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 2: // Login Button
        free_all(false, true, app);
        if (!alloc_flipper_http())
        {
            FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
            return;
        }

        callback_loading_task(
            app->fhttp,
            callback_login_fetch,
            callback_login_parse,
            FlipSocialViewLoggedInSubmenu,
            FlipSocialViewLoggedOutSubmenu,
            &app->view_dispatcher,
            false);

        // we cannot do this here because we get a freeze
        // free_flipper_http();
        // it will be freed later anyways
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their username on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_register_username_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered name
    strncpy(app->register_username_logged_out, app->register_username_logged_out_temp_buffer, app->register_username_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->register_username_logged_out[app->register_username_logged_out_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_out_register_username)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_register_username, app->register_username_logged_out);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_register_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->register_password_logged_out, app->register_password_logged_out_temp_buffer, app->register_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->register_password_logged_out[app->register_password_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_register_password)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_register_password, app->register_password_logged_out);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password 2 on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_register_password_2_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->register_password_2_logged_out, app->register_password_2_logged_out_temp_buffer, app->register_password_2_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->register_password_2_logged_out[app->register_password_2_logged_out_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_out_register_password_2)
    {
        variable_item_set_current_value_text(app->variable_item_logged_out_register_password_2, app->register_password_2_logged_out);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_out_register_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input Username
        free_all(false, true, app);
        if (!alloc_text_input(FlipSocialViewLoggedOutRegisterUsernameInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        free_all(false, true, app);
        if (!alloc_text_input(FlipSocialViewLoggedOutRegisterPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 2: // Input Password 2
        free_all(false, true, app);
        if (!alloc_text_input(FlipSocialViewLoggedOutRegisterPassword2Input))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 3: // Register button
        free_all(false, true, app);
        if (!alloc_flipper_http())
        {
            FURI_LOG_E(TAG, "Failed to allocate FlipperHTTP");
            return;
        }

        callback_loading_task(
            app->fhttp,
            callback_register_fetch,
            callback_register_parse,
            FlipSocialViewLoggedInSubmenu,
            FlipSocialViewLoggedOutSubmenu,
            &app->view_dispatcher,
            false);

        // we cannot do this here because we get a freeze
        // free_flipper_http();
        // it will be freed later anyways
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_wifi_settings_ssid_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered SSID
    strncpy(app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer, app->wifi_ssid_logged_in_temp_buffer_size);

    // Store the entered SSID in the logged out SSID
    strncpy(app->wifi_ssid_logged_out, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size);
    strncpy(app->wifi_ssid_logged_out_temp_buffer, app->wifi_ssid_logged_in, app->wifi_ssid_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->wifi_ssid_logged_in[app->wifi_ssid_logged_in_temp_buffer_size - 1] = '\0';

    // Update the name item text
    if (app->variable_item_logged_in_wifi_settings_ssid)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_ssid, app->wifi_ssid_logged_in);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_in, app_instance->wifi_password_logged_in, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_password_logged_in) > 0)
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        if (!flipper_http_save_wifi(app->fhttp, app->wifi_ssid_logged_in, app->wifi_password_logged_in))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
        furi_delay_ms(500);
        free_flipper_http();
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_wifi_settings_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Store the entered password
    strncpy(app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer, app->wifi_password_logged_in_temp_buffer_size);

    // Store the entered password in the logged out password
    strncpy(app->login_password_logged_out, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size);
    strncpy(app->login_password_logged_out_temp_buffer, app->wifi_password_logged_in, app->wifi_password_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->wifi_password_logged_in[app->wifi_password_logged_in_temp_buffer_size - 1] = '\0';

    // Update the password item text
    if (app->variable_item_logged_in_wifi_settings_password)
    {
        // dont show the password on the screen (version 0.2)
        // variable_item_set_current_value_text(app->variable_item_logged_in_wifi_settings_password, app->wifi_password_logged_in);
    }

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_in, app_instance->wifi_password_logged_in, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    // update the wifi settings
    if (strlen(app->wifi_ssid_logged_in) > 0 && strlen(app->wifi_password_logged_in) > 0)
    {
        free_flipper_http();
        if (!alloc_flipper_http())
        {
            return;
        }
        if (!flipper_http_save_wifi(app->fhttp, app->wifi_ssid_logged_in, app->wifi_password_logged_in))
        {
            FURI_LOG_E(TAG, "Failed to save wifi settings via UART");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
        }
        furi_delay_ms(500);
        free_flipper_http();
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewVariableItemList);
}

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_in_wifi_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Input SSID
        free_all(false, false, app);
        if (!alloc_text_input(FlipSocialViewLoggedInWifiSettingsSSIDInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for SSID");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 1: // Input Password
        free_all(false, false, app);
        if (!alloc_text_input(FlipSocialViewLoggedInWifiSettingsPasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for Password");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

void callback_logged_in_user_settings_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // Switch to the appropriate view
    switch (index)
    {
    case 0: // Feed Type
        break;
    case 1: // Notifications
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their message on the compose (logged in) screen for Add Text
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_compose_pre_save_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // Check if the message is empty or if adding the message would exceed MAX_PRE_SAVED_MESSAGES
    if (app->compose_pre_save_logged_in_temp_buffer_size == 0 || app->pre_saved_messages.count >= MAX_PRE_SAVED_MESSAGES)
    {
        FURI_LOG_E(TAG, "Message is empty or would exceed the maximum number of pre-saved messages");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
        return;
    }

    // Copy the entered message into the next available slot
    strncpy(
        app->pre_saved_messages.messages[app->pre_saved_messages.count],
        app->compose_pre_save_logged_in_temp_buffer,
        MAX_MESSAGE_LENGTH - 1);

    // Ensure null-termination
    app->pre_saved_messages.messages[app->pre_saved_messages.count][MAX_MESSAGE_LENGTH - 1] = '\0';

    // Increment the count
    app->pre_saved_messages.count++;

    // Rebuild the submenu
    submenu_reset(app->submenu);
    submenu_add_item(
        app->submenu,
        "Add Pre-Save",
        FlipSocialSubmenuComposeIndexAddPreSave,
        callback_submenu_choices,
        app);

    for (size_t i = 0; i < app->pre_saved_messages.count; i++)
    {
        submenu_add_item(
            app->submenu,
            app->pre_saved_messages.messages[i],
            FlipSocialSubemnuComposeIndexStartIndex + i,
            callback_submenu_choices,
            app);
    }

    // Save the updated playlist
    save_playlist(&app->pre_saved_messages);

    // Switch back to the compose view
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
}

/**
 * @brief Text input callback for when the user finishes entering their message on the profile (logged in) screen for change password
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_profile_change_password_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    // Correct type: old_pass should be a pointer to a string (char *)
    const char *old_password = app->login_password_logged_out;

    // Store the entered message
    strncpy(app->change_password_logged_in, app->change_password_logged_in_temp_buffer, app->change_password_logged_in_temp_buffer_size);

    // store the entered password in the logged out password
    strncpy(app->login_password_logged_out, app->change_password_logged_in, app->login_password_logged_out_temp_buffer_size);
    strncpy(app->login_password_logged_out_temp_buffer, app->change_password_logged_in, app->login_password_logged_out_temp_buffer_size);

    // Ensure null-termination
    app->change_password_logged_in[app->change_password_logged_in_temp_buffer_size - 1] = '\0';

    // Update the message item text
    if (app->variable_item_logged_in_profile_change_password)
    {
        // dont show the password on the screen (version 0.2)
        // variable_item_set_current_value_text(app->variable_item_logged_in_profile_change_password, app->change_password_logged_in);
    }

    // send post request to change password
    free_flipper_http();
    if (!alloc_flipper_http())
    {
        return;
    }
    alloc_headers();
    char payload[256];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"old_password\":\"%s\",\"new_password\":\"%s\"}", app->login_username_logged_out, old_password, app->change_password_logged_in);
    if (!flipper_http_request(app->fhttp, POST, "https://www.jblanked.com/flipper/api/user/change-password/", auth_headers, payload))
    {
        FURI_LOG_E(TAG, "Failed to send post request to change password");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }
    free_flipper_http();

    // Save the settings
    save_settings(app_instance->wifi_ssid_logged_out, app_instance->wifi_password_logged_out, app_instance->login_username_logged_out, app_instance->login_username_logged_in, app_instance->login_password_logged_out, app_instance->change_password_logged_in, app_instance->change_bio_logged_in, app_instance->is_logged_in);

    // instead of going to a view, just show a success message
    easy_flipper_dialog("Success", "Password updated successfully\n\n\nPress BACK to return :D");
}

void callback_logged_in_profile_change_bio_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // Store the entered message
    strncpy(app->change_bio_logged_in, app->change_bio_logged_in_temp_buffer, app->change_bio_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->change_bio_logged_in[app->change_bio_logged_in_temp_buffer_size - 1] = '\0';

    // Update the message item text
    if (app->variable_item_logged_in_profile_change_bio)
    {
        variable_item_set_current_value_text(app->variable_item_logged_in_profile_change_bio, app->change_bio_logged_in);
    }

    // send post request to change bio
    free_flipper_http();
    if (!alloc_flipper_http())
    {
        return;
    }
    alloc_headers();
    char payload[256];
    snprintf(payload, sizeof(payload), "{\"username\":\"%s\",\"bio\":\"%s\"}", app->login_username_logged_out, app->change_bio_logged_in);
    if (!flipper_http_request(app->fhttp, POST, "https://www.jblanked.com/flipper/api/user/change-bio/", auth_headers, payload))
    {
        FURI_LOG_E(TAG, "Failed to send post request to change bio");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }
    furi_delay_ms(500);
    free_flipper_http();

    // Save the settings
    save_settings(app->wifi_ssid_logged_out, app->wifi_password_logged_out, app->login_username_logged_out, app->login_username_logged_in, app->login_password_logged_out, app->change_password_logged_in, app->change_bio_logged_in, app->is_logged_in);

    // instead of going to a view, just show a success message
    easy_flipper_dialog("Success", "Bio updated successfully\n\n\nPress BACK to return :D");
}

/**
 * @brief Callback when a user selects a menu item in the profile (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_in_profile_item_selected(void *context, uint32_t index)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }
    switch (index)
    {
    case 0: // Change Username
        // do nothing since username cannot be changed
        break;
    case 1: // Change Password
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInChangePasswordInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for change password");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 2: // Change Bio
        free_text_input();
        if (!alloc_text_input(FlipSocialViewLoggedInChangeBioInput))
        {
            FURI_LOG_E(TAG, "Failed to allocate text input for change bio");
            return;
        }
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        break;
    case 3: // Friends
        free_all(false, true, app);
        if (!alloc_submenu(FlipSocialViewLoggedInFriendsSubmenu))
        {
            FURI_LOG_E(TAG, "Failed to allocate submenu for friends");
            return;
        }
        if (!alloc_flipper_http())
        {
            return;
        }

        callback_loading_task(
            app->fhttp,
            friends_fetch,
            friends_parse_json,
            FlipSocialViewSubmenu,
            FlipSocialViewVariableItemList,
            &app->view_dispatcher,
            true);

        free_flipper_http();
        break;
    default:
        FURI_LOG_E(TAG, "Unknown configuration item index");
        break;
    }
}

/**
 * @brief Text input callback for when the user finishes entering their message to send to the selected user choice (user choice messages view)
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_messages_user_choice_message_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (app->message_user_choice_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewTextInput);
        return;
    }

    // Store the entered message
    strncpy(app->message_user_choice_logged_in, app->message_user_choice_logged_in_temp_buffer, app->message_user_choice_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->message_user_choice_logged_in[app->message_user_choice_logged_in_temp_buffer_size - 1] = '\0';

    // send post request to send message
    free_flipper_http();
    if (!alloc_flipper_http())
    {
        return;
    }
    alloc_headers();
    char url[128];
    char payload[256];
    snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/messages/%s/post/", app->login_username_logged_in);
    snprintf(payload, sizeof(payload), "{\"receiver\":\"%s\",\"content\":\"%s\"}", flip_social_explore->usernames[flip_social_explore->index], app->message_user_choice_logged_in);

    if (!flipper_http_request(app->fhttp, POST, url, auth_headers, payload)) // start the async request
    {
        FURI_LOG_E(TAG, "Failed to send post request to send message");
        FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
    }
    furi_delay_ms(1000);
    free_flipper_http();
    // add user to the top of the list if not already there
    for (int i = 0; i < flip_social_message_users->count; i++)
    {
        if (strcmp(flip_social_message_users->usernames[i], flip_social_explore->usernames[flip_social_explore->index]) == 0)
        {
            // remove the user from the list
            for (int j = i; j < flip_social_message_users->count - 1; j++)
            {
                strncpy(flip_social_message_users->usernames[j], flip_social_message_users->usernames[j + 1], strlen(flip_social_message_users->usernames[j + 1]));
            }
            flip_social_message_users->count--;
            break;
        }
    }
    // add the user to the top of the list
    for (int i = flip_social_message_users->count; i > 0; i--)
    {
        strncpy(flip_social_message_users->usernames[i], flip_social_message_users->usernames[i - 1], strlen(flip_social_message_users->usernames[i - 1]));
    }
    strncpy(flip_social_message_users->usernames[0], flip_social_explore->usernames[flip_social_explore->index], strlen(flip_social_explore->usernames[flip_social_explore->index]));
    flip_social_message_users->count++;

    // redraw submenu
    messages_submenu_update();
    view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
}

/**
 * @brief Text input callback for when the user finishes entering their message to the selected user (messages view)
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_messages_new_message_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (app->messages_new_message_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewLoggedInMessagesNewMessageInput);
        return;
    }

    // Store the entered message
    strncpy(app->messages_new_message_logged_in, app->messages_new_message_logged_in_temp_buffer, app->messages_new_message_logged_in_temp_buffer_size);

    // Ensure null-termination
    app->messages_new_message_logged_in[app->messages_new_message_logged_in_temp_buffer_size - 1] = '\0';

    free_flipper_http();
    if (!alloc_flipper_http())
    {
        return;
    }

    bool send_message_to_user()
    {
        // send post request to send message
        alloc_headers();
        char url[128];
        char payload[256];
        snprintf(url, sizeof(url), "https://www.jblanked.com/flipper/api/messages/%s/post/", app->login_username_logged_in);
        snprintf(payload, sizeof(payload), "{\"receiver\":\"%s\",\"content\":\"%s\"}", flip_social_message_users->usernames[flip_social_message_users->index], app->messages_new_message_logged_in);
        if (!flipper_http_request(app->fhttp, POST, url, auth_headers, payload))
        {
            FURI_LOG_E(TAG, "Failed to send post request to send message");
            FURI_LOG_E(TAG, "Make sure the Flipper is connected to the Wifi Dev Board");
            easy_flipper_dialog("Error", "Failed to send message\n\n\nPress BACK to return :D");
            view_dispatcher_switch_to_view(app->view_dispatcher, FlipSocialViewSubmenu);
            return false;
        }
        app->fhttp->state = RECEIVING;
        return true;
    }
    bool parse_message_to_user()
    {
        while (app->fhttp->state != IDLE)
        {
            furi_delay_ms(10);
        }
        return true;
    }

    // well, we got a freeze here, so let's use the loading task to switch views and force refresh
    flipper_http_loading_task(
        app->fhttp,
        send_message_to_user,
        parse_message_to_user,
        FlipSocialViewSubmenu,
        FlipSocialViewLoggedInMessagesNewMessageInput,
        &app->view_dispatcher);

    free_flipper_http();
}

void callback_logged_in_explore_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (app->explore_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        strncpy(app->explore_logged_in, "a", 2);
    }
    else
    {
        // Store the entered message
        strncpy(app->explore_logged_in, app->explore_logged_in_temp_buffer, app->explore_logged_in_temp_buffer_size);
    }

    // Ensure null-termination
    app->explore_logged_in[app->explore_logged_in_temp_buffer_size - 1] = '\0';
    free_submenu();

    if (!alloc_submenu(FlipSocialViewLoggedInExploreSubmenu))
    {
        FURI_LOG_E(TAG, "Failed to allocate submenu for explore");
        return;
    }
    if (!alloc_flipper_http())
    {
        return;
    }
    callback_loading_task(
        app->fhttp,
        explore_fetch,                 // get the explore users
        explore_parse_json,            // parse the explore users
        FlipSocialViewSubmenu,         // switch to the explore submenu if successful
        FlipSocialViewLoggedInSubmenu, // switch back to the main submenu if failed
        &app->view_dispatcher, true);  // view dispatcher
    free_flipper_http();
}

void callback_logged_in_message_users_updated(void *context)
{
    FlipSocialApp *app = (FlipSocialApp *)context;
    if (!app)
    {
        FURI_LOG_E(TAG, "FlipSocialApp is NULL");
        return;
    }

    // check if the message is empty
    if (!app->message_users_logged_in_temp_buffer || app->message_users_logged_in_temp_buffer_size == 0)
    {
        FURI_LOG_E(TAG, "Message is empty");
        strncpy(app->message_users_logged_in, "a", 2);
    }
    else
    {
        // Store the entered message
        strncpy(app->message_users_logged_in, app->message_users_logged_in_temp_buffer, app->message_users_logged_in_temp_buffer_size);
    }

    // Ensure null-termination
    app->message_users_logged_in[app->message_users_logged_in_temp_buffer_size - 1] = '\0';

    free_submenu();

    if (!alloc_submenu(FlipSocialViewLoggedInExploreSubmenu))
    {
        FURI_LOG_E(TAG, "Failed to allocate submenu for explore");
        return;
    }

    // get users
    free_flipper_http();
    if (!alloc_flipper_http())
    {
        return;
    }
    callback_loading_task(
        app->fhttp,
        explore_fetch_2,                          // get the explore users
        messages_parse_json_message_user_choices, // parse the explore users
        FlipSocialViewSubmenu,                    // switch to the explore submenu if successful
        FlipSocialViewLoggedInSubmenu,            // switch back to the main submenu if failed
        &app->view_dispatcher, true);             // view dispatcher
    free_flipper_http();
}

bool callback_get_home_notification(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    // Create the directory for saving settings
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);
    furi_record_close(RECORD_STORAGE);
    alloc_headers();

    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data/notification.json");

    fhttp->save_received_data = true;
    return flipper_http_request(fhttp, GET, "https://www.jblanked.com/flipper/api/flip-social-notifications/", auth_headers, NULL);
}
bool callback_parse_home_notification(FlipperHTTP *fhttp)
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "app_instance is NULL");
        return false;
    }
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }

    // Load the notification from the file
    FuriString *notification = flipper_http_load_from_file(fhttp->file_path);
    if (notification == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load notification from file");
        return false;
    }

    // Check if announcement and analytics key exists
    FuriString *announcement_json = get_json_value_furi("announcement", notification);
    FuriString *analytics_json = get_json_value_furi("analytics", notification);
    if (announcement_json == NULL || analytics_json == NULL)
    {
        FURI_LOG_E(TAG, "Failed to get announcement or analytics from notification");
        if (announcement_json)
        {
            furi_string_free(announcement_json);
        }
        if (analytics_json)
        {
            furi_string_free(analytics_json);
        }
        furi_string_free(notification);
        return false;
    }

    // Extract values from JSON
    FuriString *announcement_value = get_json_value_furi("content", announcement_json);
    FuriString *announcement_time = get_json_value_furi("date_created", announcement_json);
    FuriString *analytics_value = get_json_value_furi("count", analytics_json);
    FuriString *analytics_time = get_json_value_furi("time", analytics_json);
    if (!announcement_value || !announcement_time || !analytics_value || !analytics_time)
    {
        FURI_LOG_E(TAG, "Failed to get announcement or analytics value from notification");
        if (announcement_value)
        {
            furi_string_free(announcement_value);
        }
        if (announcement_time)
        {
            furi_string_free(announcement_time);
        }
        if (analytics_value)
        {
            furi_string_free(analytics_value);
        }
        if (analytics_time)
        {
            furi_string_free(analytics_time);
        }
        furi_string_free(announcement_json);
        furi_string_free(analytics_json);
        furi_string_free(notification);
        return false;
    }

    // Load previous announcement and analytics times
    char past_analytics_time[32] = {0};
    char past_announcement_time[32] = {0};
    bool analytics_time_loaded = load_char("analytics_time", past_analytics_time, sizeof(past_analytics_time));
    bool announcement_time_loaded = load_char("announcement_time", past_announcement_time, sizeof(past_announcement_time));

    bool new_announcement = false;
    bool new_analytics = false;

    // Check for new announcement
    if (!announcement_time_loaded || strcmp(furi_string_get_cstr(announcement_time), past_announcement_time) != 0)
    {
        new_announcement = true;
    }

    // Check for new analytics
    if (!analytics_time_loaded || strcmp(furi_string_get_cstr(analytics_time), past_analytics_time) != 0)
    {
        new_analytics = true;
    }

    // If no new announcement and no new analytics, exit early
    if (!new_announcement && !new_analytics)
    {
        FURI_LOG_D(TAG, "No new announcement or analytics");
        furi_string_free(announcement_value);
        furi_string_free(announcement_time);
        furi_string_free(analytics_value);
        furi_string_free(analytics_time);
        furi_string_free(announcement_json);
        furi_string_free(analytics_json);
        furi_string_free(notification);
        return true;
    }

    // Save the new announcement and analytics times if they are new
    if (new_announcement)
    {
        save_char("announcement_time", furi_string_get_cstr(announcement_time));
    }

    if (new_analytics)
    {
        save_char("analytics_time", furi_string_get_cstr(analytics_time));
    }

    // Prepare and show dialogs based on what is new
    if (new_announcement)
    {
        easy_flipper_dialog("Announcement", (char *)furi_string_get_cstr(announcement_value));
    }

    if (new_analytics)
    {
        char analytics_text[128] = {0};
        // Determine the new posts count
        if (atoi(furi_string_get_cstr(analytics_value)) > 0)
        {
            char past_analytics_value[32] = {0};
            int new_posts = 0;
            if (load_char("analytics_value", past_analytics_value, sizeof(past_analytics_value)))
            {
                int past_posts = atoi(past_analytics_value);
                int current_posts = atoi(furi_string_get_cstr(analytics_value));
                new_posts = current_posts - past_posts;
                snprintf(analytics_text, sizeof(analytics_text), "%d new posts", new_posts);
            }
            else
            {
                snprintf(analytics_text, sizeof(analytics_text), "%s feed posts", furi_string_get_cstr(analytics_value));
            }
            save_char("analytics_value", furi_string_get_cstr(analytics_value));
        }
        else
        {
            snprintf(analytics_text, sizeof(analytics_text), "%s feed posts", furi_string_get_cstr(analytics_value));
        }

        easy_flipper_dialog("Notifications", analytics_text);
    }

    // Free allocated resources
    furi_string_free(announcement_value);
    furi_string_free(announcement_time);
    furi_string_free(analytics_value);
    furi_string_free(analytics_time);
    furi_string_free(announcement_json);
    furi_string_free(analytics_json);
    furi_string_free(notification);

    return true;
}

// home notification
bool callback_home_notification(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }

    // Make the request
    if (!callback_get_home_notification(fhttp)) // start the async request
    {
        FURI_LOG_E(TAG, "Failed to send get request to home notification");
        return false;
    }

    furi_timer_start(fhttp->get_timeout_timer, TIMEOUT_DURATION_TICKS);
    fhttp->state = RECEIVING;

    while (fhttp->state == RECEIVING && furi_timer_is_running(fhttp->get_timeout_timer) > 0)
    {
        // Wait for the request to be received
        furi_delay_ms(100);
    }

    furi_timer_stop(fhttp->get_timeout_timer);

    if (!callback_parse_home_notification(fhttp)) // parse the JSON before switching to the view (synchonous)
    {
        FURI_LOG_E(HTTP_TAG, "Failed to parse the home notification...");
        return false;
    }

    return true;
}