#include <messages/messages.h>
#include <alloc/alloc.h>

bool messages_submenu_update()
{
    if (!app_instance)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (app_instance->submenu == NULL)
    {
        FURI_LOG_E(TAG, "Submenu is NULL");
        return false;
    }
    if (flip_social_message_users == NULL)
    {
        FURI_LOG_E(TAG, "Message users model is NULL");
        return false;
    }
    submenu_reset(app_instance->submenu);
    submenu_set_header(app_instance->submenu, "Messages");
    submenu_add_item(app_instance->submenu, "[New Message]", FlipSocialSubmenuLoggedInIndexMessagesNewMessage, callback_submenu_choices, app_instance);
    for (int i = 0; i < flip_social_message_users->count; i++)
    {
        submenu_add_item(app_instance->submenu, flip_social_message_users->usernames[i], FlipSocialSubmenuLoggedInIndexMessagesUsersStart + i, callback_submenu_choices, app_instance);
    }
    return true;
}

bool messages_update_submenu_user_choices()
{
    if (app_instance == NULL)
    {
        FURI_LOG_E(TAG, "App instance is NULL");
        return false;
    }
    if (app_instance->submenu == NULL)
    {
        FURI_LOG_E(TAG, "Submenu is NULL");
        return false;
    }
    if (flip_social_explore == NULL)
    {
        FURI_LOG_E(TAG, "Explore model is NULL");
        return false;
    }
    submenu_reset(app_instance->submenu);
    submenu_set_header(app_instance->submenu, "Users");
    for (int i = 0; i < flip_social_explore->count; i++)
    {
        submenu_add_item(app_instance->submenu, flip_social_explore->usernames[i], FlipSocialSubmenuLoggedInIndexMessagesUserChoicesIndexStart + i, callback_submenu_choices, app_instance);
    }
    return true;
}

// Get all the users that have sent messages to the logged in user
bool messages_get_message_users(FlipperHTTP *fhttp)
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
    if (app_instance->login_username_logged_out == NULL)
    {
        FURI_LOG_E(TAG, "Username is NULL");
        return false;
    }
    // create the messages directory
    if (!flip_social_subfolder_mkdir("messages"))
    {
        FURI_LOG_E(TAG, "Failed to create messages directory");
        return false;
    }

    char command[128];
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/messages/message_users.json");

    fhttp->save_received_data = true;
    alloc_headers();
    snprintf(command, 128, "https://www.jblanked.com/flipper/api/messages/%s/get/list/%d/", app_instance->login_username_logged_out, MAX_MESSAGE_USERS);
    if (!flipper_http_request(fhttp, GET, command, auth_headers, NULL))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for messages");
        fhttp->state = ISSUE;
        return false;
    }
    fhttp->state = RECEIVING;
    return true;
}

// Get all the messages between the logged in user and the selected user
bool messages_get_messages_with_user(FlipperHTTP *fhttp)
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
    if (app_instance->login_username_logged_out == NULL)
    {
        FURI_LOG_E(TAG, "Username is NULL");
        return false;
    }
    if (strlen(flip_social_message_users->usernames[flip_social_message_users->index]) == 0)
    {
        FURI_LOG_E(TAG, "Username is NULL");
        return false;
    }

    // create the messages directory
    if (!flip_social_subfolder_mkdir("messages"))
    {
        FURI_LOG_E(TAG, "Failed to create messages directory");
        return false;
    }

    char command[256];
    snprintf(
        fhttp->file_path,
        sizeof(fhttp->file_path),
        STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/messages/%s_messages.json",
        flip_social_message_users->usernames[flip_social_message_users->index]);

    fhttp->save_received_data = true;
    alloc_headers();
    snprintf(command, sizeof(command), "https://www.jblanked.com/flipper/api/messages/%s/get/%s/%d/", app_instance->login_username_logged_out, flip_social_message_users->usernames[flip_social_message_users->index], MAX_MESSAGES);
    if (!flipper_http_request(fhttp, GET, command, auth_headers, NULL))
    {
        FURI_LOG_E(TAG, "Failed to send HTTP request for messages");
        fhttp->state = ISSUE;
        return false;
    }
    fhttp->state = RECEIVING;
    return true;
}

// Parse the users that have sent messages to the logged-in user
bool messages_parse_json_message_users(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }
    // load the received data from the saved file
    FuriString *message_data = flipper_http_load_from_file(fhttp->file_path);
    if (message_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    // Allocate memory for each username only if not already allocated
    flip_social_message_users = alloc_messages();
    if (flip_social_message_users == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for message users.");
        furi_string_free(message_data);
        return false;
    }

    // Initialize message users count
    flip_social_message_users->count = 0;

    for (int i = 0; i < MAX_MESSAGE_USERS; i++)
    {
        FuriString *user = get_json_array_value_furi("users", i, message_data);
        if (user == NULL)
        {
            break;
        }
        snprintf(flip_social_message_users->usernames[i], MAX_USER_LENGTH, "%s", furi_string_get_cstr(user));
        flip_social_message_users->count++;
        furi_string_free(user);
    }

    // Add submenu items for the users
    messages_submenu_update();

    // Free the JSON data
    furi_string_free(message_data);
    return true;
}

// Parse the users that the logged in user can message
bool messages_parse_json_message_user_choices(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }

    // load the received data from the saved file
    FuriString *user_data = flipper_http_load_from_file(fhttp->file_path);
    if (user_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    // Allocate memory for each username only if not already allocated
    flip_social_explore = alloc_explore();
    if (flip_social_explore == NULL)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for explore usernames.");
        furi_string_free(user_data);
        return false;
    }

    // Initialize explore count
    flip_social_explore->count = 0;

    for (int i = 0; i < MAX_MESSAGE_USERS; i++)
    {
        FuriString *user = get_json_array_value_furi("users", i, user_data);
        if (user == NULL)
        {
            break;
        }
        snprintf(flip_social_explore->usernames[i], MAX_USER_LENGTH, "%s", furi_string_get_cstr(user));
        flip_social_explore->count++;
        furi_string_free(user);
    }

    // Add submenu items for the users
    messages_update_submenu_user_choices();

    // Free the JSON data
    furi_string_free(user_data);
    return flip_social_explore->count > 0;
}

// parse messages between the logged in user and the selected user
bool messages_parse_json_messages(FlipperHTTP *fhttp)
{
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "FlipperHTTP is NULL");
        return false;
    }

    // load the received data from the saved file
    FuriString *message_data = flipper_http_load_from_file(fhttp->file_path);
    if (message_data == NULL)
    {
        FURI_LOG_E(TAG, "Failed to load received data from file.");
        return false;
    }

    // Allocate memory for each message only if not already allocated
    flip_social_messages = alloc_user_messages();
    if (!flip_social_messages)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for messages.");
        furi_string_free(message_data);
        return false;
    }

    // Initialize messages count
    flip_social_messages->count = 0;

    // Iterate through the messages array
    for (int i = 0; i < MAX_MESSAGES; i++)
    {
        // Parse each item in the array
        FuriString *item = get_json_array_value_furi("conversations", i, message_data);
        if (item == NULL)
        {
            break;
        }

        // Extract individual fields from the JSON object
        FuriString *sender = get_json_value_furi("sender", item);
        FuriString *content = get_json_value_furi("content", item);

        if (sender == NULL || content == NULL)
        {
            FURI_LOG_E(TAG, "Failed to parse item fields.");
            if (sender)
                furi_string_free(sender);
            if (content)
                furi_string_free(content);
            furi_string_free(item);
            continue;
        }

        // Store parsed values in pre-allocated memory
        snprintf(flip_social_messages->usernames[i], MAX_USER_LENGTH, "%s", furi_string_get_cstr(sender));
        snprintf(flip_social_messages->messages[i], MAX_MESSAGE_LENGTH, "%s", furi_string_get_cstr(content));
        flip_social_messages->count++;

        furi_string_free(item);
        furi_string_free(sender);
        furi_string_free(content);
    }
    if (!allow_messages_dialog(true))
    {
        FURI_LOG_E(TAG, "Failed to allocate and set messages dialog.");
        furi_string_free(message_data);
        return false;
    }
    furi_string_free(message_data);
    return true;
}