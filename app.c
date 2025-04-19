// app.c
#include <flip_social.h>             // Include the FlipSocialApp structure
#include <alloc/flip_social_alloc.h> // Include the allocation functions

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
    app_instance = flip_social_app_alloc();
    if (!app_instance)
    {
        // Allocation failed
        FURI_LOG_E(TAG, "Failed to allocate FlipSocialApp");
        return -1; // Indicate failure
    }

    // check if board is connected (Derek Jamison)
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (!fhttp)
    {
        easy_flipper_dialog("FlipperHTTP Error", "The UART is likely busy.\nEnsure you have the correct\nflash for your board then\nrestart your Flipper Zero.");
        return -1;
    }

    if (!flipper_http_send_command(fhttp, HTTP_CMD_PING))
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        flipper_http_free(fhttp);
        return -1;
    }

    // Try to wait for pong response.
    uint32_t counter = 10;
    while (fhttp->state == INACTIVE && --counter > 0)
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
            flip_social_home_notification();
        }
    }

    flipper_http_free(fhttp);

    // Run the view dispatcher
    view_dispatcher_run(app_instance->view_dispatcher);

    // Free the resources used by the Hello World application
    flip_social_app_free(app_instance);

    // Return 0 to indicate success
    return 0;
}
