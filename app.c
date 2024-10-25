// app.c
#include <jsmn.h>                // Include cJSON
#include <uart_text_input.h>     // Include the text input widget
#include <flip_social_e.h>       // Include the FlipSocialApp structure
#include <flip_social_storage.h> // Include the storage functions
#include "flip_social_draw.h"
#include "flip_social_feed.h"
#include "flip_social_explore.h"
#include "flip_social_friends.h"
#include <flip_social_callback.h> // Include the callback functions
#include <flip_social_i.h>        // Include the initialization functions
#include <flip_social_free.h>     // Include the cleanup functions

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
    FlipSocialApp *app = flip_social_app_alloc();
    if (!app)
    {
        // Allocation failed
        return -1; // Indicate failure
    }

    // send settings and connect wifi
    if (!flipper_http_connect_wifi())
    {
        FURI_LOG_E(TAG, "Failed to connect to WiFi");
        return -1;
    }

    if (!flipper_http_ping())
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        return -1;
    }

    // Run the view dispatcher
    view_dispatcher_run(app->view_dispatcher);

    // Free the resources used by the Hello World application
    flip_social_app_free(app);

    // Return 0 to indicate success
    return 0;
}
