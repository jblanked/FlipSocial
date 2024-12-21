#include "flip_social_storage.h"

// Function to save the playlist
void save_playlist(const PreSavedPlaylist *playlist)
{
    if (!playlist)
    {
        FURI_LOG_E(TAG, "Playlist is NULL");
        return;
    }

    // Create the directory for saving settings
    char directory_path[128];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);

    // Open the settings file
    File *file = storage_file_alloc(storage);
    if (!storage_file_open(file, PRE_SAVED_MESSAGES_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS))
    {
        FURI_LOG_E(TAG, "Failed to open settings file for writing: %s", PRE_SAVED_MESSAGES_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return;
    }

    // Write each playlist message on a separate line
    for (size_t i = 0; i < playlist->count; ++i)
    {
        // Write the message
        if (storage_file_write(file, playlist->messages[i], strlen(playlist->messages[i])) != strlen(playlist->messages[i]))
        {
            FURI_LOG_E(TAG, "Failed to write playlist message %zu", i);
        }

        // Write a newline after each message
        if (storage_file_write(file, "\n", 1) != 1)
        {
            FURI_LOG_E(TAG, "Failed to write newline after message %zu", i);
        }
    }

    // Close the file and storage
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool load_playlist(PreSavedPlaylist *playlist)
{
    if (!playlist)
    {
        FURI_LOG_E(TAG, "Playlist is NULL");
        return false;
    }

    // Clear existing data in the playlist
    memset(playlist->messages, 0, sizeof(playlist->messages));
    playlist->count = 0;

    // Open the storage
    Storage *storage = furi_record_open(RECORD_STORAGE);
    File *file = storage_file_alloc(storage);

    if (!storage_file_open(file, PRE_SAVED_MESSAGES_PATH, FSAM_READ, FSOM_OPEN_EXISTING))
    {
        FURI_LOG_E(TAG, "Failed to open pre-saved messages file for reading: %s", PRE_SAVED_MESSAGES_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    // Read the file line by line
    char line[MAX_MESSAGE_LENGTH] = {0};
    size_t line_pos = 0;
    char ch;

    while (storage_file_read(file, &ch, 1) == 1)
    {
        if (ch == '\n' || line_pos >= (MAX_MESSAGE_LENGTH - 1)) // End of line or max length reached
        {
            line[line_pos] = '\0'; // Null-terminate the line
            strncpy(playlist->messages[playlist->count], line, MAX_MESSAGE_LENGTH);
            playlist->count++;
            line_pos = 0;

            // Ensure playlist count does not exceed maximum allowed
            if (playlist->count >= MAX_PRE_SAVED_MESSAGES)
            {
                FURI_LOG_W(TAG, "Reached maximum playlist messages");
                break;
            }
        }
        else
        {
            line[line_pos++] = ch;
        }
    }

    // Handle the last line if it does not end with a newline
    if (line_pos > 0)
    {
        line[line_pos] = '\0'; // Null-terminate the last line
        strncpy(playlist->messages[playlist->count], line, MAX_MESSAGE_LENGTH);
        playlist->count++;
    }

    // Close the file and storage
    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return true;
}

void save_settings(
    const char *ssid,
    const char *password,
    const char *login_username_logged_out,
    const char *login_username_logged_in,
    const char *login_password_logged_out,
    const char *change_password_logged_in,
    const char *change_bio_logged_in,
    const char *is_logged_in)
{
    // Create the directory for saving settings
    char directory_path[128];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);

    // Open the settings file
    File *file = storage_file_alloc(storage);
    if (!storage_file_open(file, SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS))
    {
        FURI_LOG_E(TAG, "Failed to open settings file for writing: %s", SETTINGS_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return;
    }

    // Save the ssid length and data
    size_t ssid_length = strlen(ssid) + 1; // Include null terminator
    if (storage_file_write(file, &ssid_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, ssid, ssid_length) != ssid_length)
    {
        FURI_LOG_E(TAG, "Failed to write SSID");
    }

    // Save the password length and data
    size_t password_length = strlen(password) + 1; // Include null terminator
    if (storage_file_write(file, &password_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, password, password_length) != password_length)
    {
        FURI_LOG_E(TAG, "Failed to write password");
    }

    // Save the login_username_logged_out length and data
    size_t username_out_length = strlen(login_username_logged_out) + 1; // Include null terminator
    if (storage_file_write(file, &username_out_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, login_username_logged_out, username_out_length) != username_out_length)
    {
        FURI_LOG_E(TAG, "Failed to write login_username_logged_out");
    }

    // Save the login_username_logged_in length and data
    size_t username_in_length = strlen(login_username_logged_in) + 1; // Include null terminator
    if (storage_file_write(file, &username_in_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, login_username_logged_in, username_in_length) != username_in_length)
    {
        FURI_LOG_E(TAG, "Failed to write login_username_logged_in");
    }

    // Save the login_password_logged_out length and data
    size_t password_out_length = strlen(login_password_logged_out) + 1; // Include null terminator
    if (storage_file_write(file, &password_out_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, login_password_logged_out, password_out_length) != password_out_length)
    {
        FURI_LOG_E(TAG, "Failed to write login_password_logged_out");
    }

    // Save the change_password_logged_in length and data
    size_t change_password_length = strlen(change_password_logged_in) + 1; // Include null terminator
    if (storage_file_write(file, &change_password_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, change_password_logged_in, change_password_length) != change_password_length)
    {
        FURI_LOG_E(TAG, "Failed to write change_password_logged_in");
    }

    // Save the is_logged_in length and data
    size_t is_logged_in_length = strlen(is_logged_in) + 1; // Include null terminator
    if (storage_file_write(file, &is_logged_in_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, is_logged_in, is_logged_in_length) != is_logged_in_length)
    {
        FURI_LOG_E(TAG, "Failed to write is_logged_in");
    }

    // Save the change_bio_logged_in length and data
    size_t change_bio_length = strlen(change_bio_logged_in) + 1; // Include null terminator
    if (storage_file_write(file, &change_bio_length, sizeof(size_t)) != sizeof(size_t) ||
        storage_file_write(file, change_bio_logged_in, change_bio_length) != change_bio_length)
    {
        FURI_LOG_E(TAG, "Failed to write change_bio_logged_in");
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

bool load_settings(
    char *ssid,
    size_t ssid_size,
    char *password,
    size_t password_size,
    char *login_username_logged_out,
    size_t username_out_size,
    char *login_username_logged_in,
    size_t username_in_size,
    char *login_password_logged_out,
    size_t password_out_size,
    char *change_password_logged_in,
    size_t change_password_size,
    char *change_bio_logged_in,
    size_t change_bio_size,
    char *is_logged_in,
    size_t is_logged_in_size)
{
    Storage *storage = furi_record_open(RECORD_STORAGE);
    File *file = storage_file_alloc(storage);

    if (!storage_file_open(file, SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING))
    {
        FURI_LOG_E(TAG, "Failed to open settings file for reading: %s", SETTINGS_PATH);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false; // Return false if the file does not exist
    }

    // Load the ssid
    size_t ssid_length;
    if (storage_file_read(file, &ssid_length, sizeof(size_t)) != sizeof(size_t) || ssid_length > ssid_size ||
        storage_file_read(file, ssid, ssid_length) != ssid_length)
    {
        FURI_LOG_E(TAG, "Failed to read SSID");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    else
    {
        ssid[ssid_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the password
    size_t password_length;
    if (storage_file_read(file, &password_length, sizeof(size_t)) != sizeof(size_t) || password_length > password_size ||
        storage_file_read(file, password, password_length) != password_length)
    {
        FURI_LOG_E(TAG, "Failed to read password");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }
    else
    {
        password[password_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the login_username_logged_out
    size_t username_out_length;
    if (storage_file_read(file, &username_out_length, sizeof(size_t)) != sizeof(size_t) || username_out_length > username_out_size ||
        storage_file_read(file, login_username_logged_out, username_out_length) != username_out_length)
    {
        FURI_LOG_E(TAG, "Failed to read login_username_logged_out");
        // storage_file_close(file);
        // storage_file_free(file);
        // furi_record_close(RECORD_STORAGE);
        // return false;
    }
    else
    {
        login_username_logged_out[username_out_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the login_username_logged_in
    size_t username_in_length;
    if (storage_file_read(file, &username_in_length, sizeof(size_t)) != sizeof(size_t) || username_in_length > username_in_size ||
        storage_file_read(file, login_username_logged_in, username_in_length) != username_in_length)
    {
        FURI_LOG_E(TAG, "Failed to read login_username_logged_in");
        // storage_file_close(file);
        // storage_file_free(file);
        // furi_record_close(RECORD_STORAGE);
        // return false;
    }
    else
    {
        login_username_logged_in[username_in_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the login_password_logged_out
    size_t password_out_length;
    if (storage_file_read(file, &password_out_length, sizeof(size_t)) != sizeof(size_t) || password_out_length > password_out_size ||
        storage_file_read(file, login_password_logged_out, password_out_length) != password_out_length)
    {
        FURI_LOG_E(TAG, "Failed to read login_password_logged_out");
        // storage_file_close(file);
        // storage_file_free(file);
        // furi_record_close(RECORD_STORAGE);
        // return false;
    }
    else
    {
        login_password_logged_out[password_out_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the change_password_logged_in
    size_t change_password_length;
    if (storage_file_read(file, &change_password_length, sizeof(size_t)) != sizeof(size_t) || change_password_length > change_password_size ||
        storage_file_read(file, change_password_logged_in, change_password_length) != change_password_length)
    {
        FURI_LOG_E(TAG, "Failed to read change_password_logged_in");
        // storage_file_close(file);
        // storage_file_free(file);
        // furi_record_close(RECORD_STORAGE);
        //  return false;
    }
    else
    {
        change_password_logged_in[change_password_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the is_logged_in
    size_t is_logged_in_length;
    if (storage_file_read(file, &is_logged_in_length, sizeof(size_t)) != sizeof(size_t) || is_logged_in_length > is_logged_in_size ||
        storage_file_read(file, is_logged_in, is_logged_in_length) != is_logged_in_length)
    {
        FURI_LOG_E(TAG, "Failed to read is_logged_in");
        // storage_file_close(file);
        // storage_file_free(file);
        // furi_record_close(RECORD_STORAGE);
        //  return false;
    }
    else
    {
        is_logged_in[is_logged_in_length - 1] = '\0'; // Ensure null-termination
    }

    // Load the change_bio_logged_in
    size_t change_bio_length;
    if (storage_file_read(file, &change_bio_length, sizeof(size_t)) != sizeof(size_t) || change_bio_length > change_bio_size ||
        storage_file_read(file, change_bio_logged_in, change_bio_length) != change_bio_length)
    {
        FURI_LOG_E(TAG, "Failed to read change_bio_logged_in");
        // storage_file_close(file);
        // storage_file_free(file);
        // furi_record_close(RECORD_STORAGE);
        //  return false;
    }
    else
    {
        change_bio_logged_in[change_bio_length - 1] = '\0'; // Ensure null-termination
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return true;
}

bool flip_social_save_post(const char *post_id, const char *json_feed_data)
{
    Storage *storage = furi_record_open(RECORD_STORAGE);
    File *file = storage_file_alloc(storage);

    // Create the directory for saving the feed
    char directory_path[128];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/feed");
    storage_common_mkdir(storage, directory_path);

    char file_path[128];
    snprintf(file_path, sizeof(file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/feed/feed_post_%s.json", post_id);

    if (!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS))
    {
        FURI_LOG_E(TAG, "Failed to open file for writing: %s", file_path);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    if (storage_file_write(file, json_feed_data, strlen(json_feed_data)) != strlen(json_feed_data))
    {
        FURI_LOG_E(TAG, "Failed to write feed post data");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return true;
}
//
bool save_char(
    const char *path_name, const char *value)
{
    if (!value)
    {
        return false;
    }
    // Create the directory for saving settings
    char directory_path[256];
    snprintf(directory_path, sizeof(directory_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data");

    // Create the directory
    Storage *storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(storage, directory_path);

    // Open the settings file
    File *file = storage_file_alloc(storage);
    char file_path[256];
    snprintf(file_path, sizeof(file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data/%s.txt", path_name);

    // Open the file in write mode
    if (!storage_file_open(file, file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS))
    {
        FURI_LOG_E(HTTP_TAG, "Failed to open file for writing: %s", file_path);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    // Write the data to the file
    size_t data_size = strlen(value) + 1; // Include null terminator
    if (storage_file_write(file, value, data_size) != data_size)
    {
        FURI_LOG_E(HTTP_TAG, "Failed to append data to file");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return true;
}

bool load_char(
    const char *path_name,
    char *value,
    size_t value_size)
{
    if (!value)
    {
        return false;
    }
    Storage *storage = furi_record_open(RECORD_STORAGE);
    File *file = storage_file_alloc(storage);

    char file_path[256];
    snprintf(file_path, sizeof(file_path), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/data/%s.txt", path_name);

    // Open the file for reading
    if (!storage_file_open(file, file_path, FSAM_READ, FSOM_OPEN_EXISTING))
    {
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return NULL; // Return false if the file does not exist
    }

    // Read data into the buffer
    size_t read_count = storage_file_read(file, value, value_size);
    if (storage_file_get_error(file) != FSE_OK)
    {
        FURI_LOG_E(HTTP_TAG, "Error reading from file.");
        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
        return false;
    }

    // Ensure null-termination
    value[read_count - 1] = '\0';

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return true;
}