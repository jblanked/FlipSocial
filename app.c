#include <flip_social.h>
#include <alloc/alloc.h>
#include <flip_storage/flip_social_storage.h>

/**
 * @brief Entry point for the Hello World application.
 * @details Initializes the app, runs the view dispatcher, and cleans up upon exit.
 * @param p Input parameter - unused
 * @return 0 to indicate success, -1 on failure
 */
int32_t main_flip_social(void *p)
{
    UNUSED(p);

    // Initialize the Hello World application
    app_instance = alloc_flip_social_app();
    if (!app_instance)
    {
        // Allocation failed
        FURI_LOG_E(TAG, "Failed to allocate FlipSocialApp");
        return -1; // Indicate failure
    }

    // check if board is connected (Derek Jamison)
    if (!alloc_flipper_http())
    {
        easy_flipper_dialog("FlipperHTTP Error", "The UART is likely busy.\nEnsure you have the correct\nflash for your board then\nrestart your Flipper Zero.");
        return -1;
    }

    if (!flipper_http_send_command(app_instance->fhttp, HTTP_CMD_PING))
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        free_flipper_http();
        return -1;
    }

    // Try to wait for pong response.
    uint32_t counter = 10;
    while (app_instance->fhttp->state == INACTIVE && --counter > 0)
    {
        FURI_LOG_D(TAG, "Waiting for PONG");
        furi_delay_ms(100);
    }
    // save app version
    save_char("app_version", VERSION);

    if (counter == 0)
    {
        easy_flipper_dialog("FlipperHTTP Error", "Ensure your WiFi Developer\nBoard or Pico W is connected\nand the latest FlipperHTTP\nfirmware is installed.");
    }
    else
    {
        char is_connected[5];
        char is_logged_in[5];
        char is_notifications[5];
        load_char("is_connected", is_connected, 5);
        load_char("is_logged_in", is_logged_in, 5);
        load_char("user_notifications", is_notifications, 5);

        if (strcmp(is_connected, "true") == 0 &&
            strcmp(is_notifications, "on") == 0 &&
            strcmp(is_logged_in, "true") == 0)
        {
            callback_home_notification(app_instance->fhttp);
        }

        // if (update_is_ready(app_instance->fhttp, true))
        // {
        //     easy_flipper_dialog("Update Status", "Complete.\nRestart your Flipper Zero.");
        // }
    }

    free_flipper_http();

    // Run the view dispatcher
    view_dispatcher_run(app_instance->view_dispatcher);

    // Free the resources used by the Hello World application
    free_flip_social_app(app_instance);

    // Return 0 to indicate success
    return 0;
}
