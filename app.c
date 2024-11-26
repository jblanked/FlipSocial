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
        return -1; // Indicate failure
    }

    if (!flipper_http_ping())
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        return -1;
    }

    // Thanks to Derek Jamison for the following edits
    if (app_instance->wifi_ssid_logged_out != NULL &&
        app_instance->wifi_password_logged_out != NULL)
    {
        // Try to wait for pong response.
        uint8_t counter = 10;
        while (fhttp.state == INACTIVE && --counter > 0)
        {
            FURI_LOG_D(TAG, "Waiting for PONG");
            furi_delay_ms(100);
        }

        if (counter == 0)
        {
            DialogsApp *dialogs = furi_record_open(RECORD_DIALOGS);
            DialogMessage *message = dialog_message_alloc();
            dialog_message_set_header(
                message, "[FlipperHTTP Error]", 64, 0, AlignCenter, AlignTop);
            dialog_message_set_text(
                message,
                "Ensure your WiFi Developer\nBoard or Pico W is connected\nand the latest FlipperHTTP\nfirmware is installed.",
                0,
                63,
                AlignLeft,
                AlignBottom);
            dialog_message_show(dialogs, message);
            dialog_message_free(message);
            furi_record_close(RECORD_DIALOGS);
        }
    }

    // Run the view dispatcher
    view_dispatcher_run(app_instance->view_dispatcher);

    // Free the resources used by the Hello World application
    flip_social_app_free(app_instance);

    // Return 0 to indicate success
    return 0;
}
