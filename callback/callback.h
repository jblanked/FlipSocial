#ifndef FLIP_SOCIAL_CALLBACK_H
#define FLIP_SOCIAL_CALLBACK_H
#include <flip_social.h>

// replica of flipper_http_load_task but allows FlipperHTTP to be passed in
typedef bool (*LoadingCallback)(FlipperHTTP *);

void callback_loading_task(FlipperHTTP *fhttp,
                           LoadingCallback http_request,
                           LoadingCallback parse_response,
                           uint32_t success_view_id,
                           uint32_t failure_view_id,
                           ViewDispatcher **view_dispatcher);

bool callback_request_await(FlipperHTTP *fhttp, LoadingCallback http_request, LoadingCallback parse_response);

/**
 * @brief Navigation callback to go back to the submenu Logged out.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutSubmenu)
 */
uint32_t callback_to_submenu_logged_out(void *context);

/**
 * @brief Navigation callback to go back to the submenu Logged in.
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSubmenu)
 */
uint32_t callback_to_submenu_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Login screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutLogin)
 */
uint32_t callback_to_login_logged_out(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Register screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutRegister)
 */
uint32_t callback_to_register_logged_out(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged out) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedOutWifiSettings)
 */
uint32_t callback_to_wifi_settings_logged_out(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Wifi Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t callback_to_wifi_settings_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Settings screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInSettingsWifi)
 */
uint32_t callback_to_settings_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Compose screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInCompose)
 */
uint32_t callback_to_compose_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the (Logged in) Profile screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInProfile)
 */
uint32_t callback_to_profile_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the Explore submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInExploreSubmenu)
 */
uint32_t callback_to_explore_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the Friends submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInFriendsSubmenu)
 */
uint32_t callback_to_friends_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the Messages submenu
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesSubmenu)
 */
uint32_t callback_to_messages_logged_in(void *context);

/**
 * @brief Navigation callback to bring the user back to the User Choices screen
 * @param context The context - unused
 * @return next view id (FlipSocialViewLoggedInMessagesUserChoices)
 */
uint32_t callback_to_messages_user_choices(void *context);

/**
 * @brief Navigation callback for exiting the application
 * @param context The context - unused
 * @return next view id (VIEW_NONE to exit the app)
 */
uint32_t callback_exit_app(void *context);

void callback_feed_draw(Canvas *canvas, void *model);
bool callback_feed_input(InputEvent *event, void *context);

/**
 * @brief Handle ALL submenu item selections.
 * @param context The context - FlipSocialApp object.
 * @param index The FlipSocialSubmenuIndex item that was clicked.
 * @return void
 */
void callback_submenu_choices(void *context, uint32_t index);

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_wifi_settings_ssid_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_wifi_settings_password_updated(void *context);

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_out_wifi_settings_item_selected(void *context, uint32_t index);

/**
 * @brief Text input callback for when the user finishes entering their username on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_login_username_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their password on the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_login_password_updated(void *context);

/**
 * @brief Callback when the user selects a menu item in the login (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_out_login_item_selected(void *context, uint32_t index);

/**
 * @brief Text input callback for when the user finishes entering their username on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_register_username_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their password on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_register_password_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their password 2 on the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_out_register_password_2_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their SSID on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_wifi_settings_ssid_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their password on the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_wifi_settings_password_updated(void *context);

/**
 * @brief Callback when the user selects a menu item in the wifi settings (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_in_wifi_settings_item_selected(void *context, uint32_t index);

/**
 * @brief Text input callback for when the user finishes entering their message on the compose (logged in) screen for Add Text
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_compose_pre_save_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their message on the profile (logged in) screen for change password
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_profile_change_password_updated(void *context);

/**
 * @brief Callback when a user selects a menu item in the profile (logged in) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_in_profile_change_bio_updated(void *context);
void callback_logged_in_profile_item_selected(void *context, uint32_t index);

/**
 * @brief Text input callback for when the user finishes entering their message to send to the selected user choice (user choice messages view)
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_messages_user_choice_message_updated(void *context);

/**
 * @brief Text input callback for when the user finishes entering their message to the selected user (messages view)
 * @param context The context - FlipSocialApp object.
 * @return void
 */
void callback_logged_in_messages_new_message_updated(void *context);

/**
 * @brief Callback when the user selects a menu item in the register (logged out) screen.
 * @param context The context - FlipSocialApp object.
 * @param index The index of the selected item.
 * @return void
 */
void callback_logged_out_register_item_selected(void *context, uint32_t index);
void callback_logged_in_user_settings_item_selected(void *context, uint32_t index);
void callback_logged_in_explore_updated(void *context);
void callback_logged_in_message_users_updated(void *context);

void callback_message_dialog(DialogExResult result, void *context);
//
bool callback_home_notification(FlipperHTTP *fhttp);
#endif