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
uint8_t flip_social_notification_type_index = 1;

bool went_to_friends = false;
Loading *loading_global = NULL;

bool flip_social_subfolder_mkdir(char *folder_name)
{
    if (!folder_name || strlen(folder_name) == 0)
    {
        FURI_LOG_E(TAG, "Folder name is NULL/empty.");
        return false;
    }
    Storage *storage = furi_record_open(RECORD_STORAGE);
    char directory[128];
    snprintf(directory, sizeof(directory), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social");
    storage_common_mkdir(storage, directory);
    snprintf(directory, sizeof(directory), STORAGE_EXT_PATH_PREFIX "/apps_data/flip_social/%s", folder_name);
    storage_common_mkdir(storage, directory);
    furi_record_close(RECORD_STORAGE);
    return true;
}