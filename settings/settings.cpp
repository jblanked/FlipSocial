#include "settings.hpp"
#include "app.hpp"

HelloWorldSettings::HelloWorldSettings(ViewDispatcher **view_dispatcher, void *appContext) : appContext(appContext), view_dispatcher_ref(view_dispatcher)
{
    if (!easy_flipper_set_variable_item_list(&variable_item_list, HelloWorldViewSettings,
                                             settingsItemSelectedCallback, callbackToSubmenu, view_dispatcher, this))
    {
        return;
    }

    variable_item_wifi_ssid = variable_item_list_add(variable_item_list, "WiFi SSID", 1, nullptr, nullptr);
    variable_item_wifi_pass = variable_item_list_add(variable_item_list, "WiFi Password", 1, nullptr, nullptr);
    variable_item_connect = variable_item_list_add(variable_item_list, "Connect", 1, nullptr, nullptr);

    char loaded_ssid[64];
    char loaded_pass[64];
    HelloWorldApp *app = static_cast<HelloWorldApp *>(appContext);
    if (app->loadChar("wifi_ssid", loaded_ssid, sizeof(loaded_ssid)))
    {
        variable_item_set_current_value_text(variable_item_wifi_ssid, loaded_ssid);
    }
    else
    {
        variable_item_set_current_value_text(variable_item_wifi_ssid, "");
    }
    if (app->loadChar("wifi_pass", loaded_pass, sizeof(loaded_pass)))
    {
        variable_item_set_current_value_text(variable_item_wifi_pass, "*****");
    }
    else
    {
        variable_item_set_current_value_text(variable_item_wifi_pass, "");
    }
    variable_item_set_current_value_text(variable_item_connect, "");
}

HelloWorldSettings::~HelloWorldSettings()
{
    // Free text input first
    freeTextInput();

    if (variable_item_list && view_dispatcher_ref && *view_dispatcher_ref)
    {
        view_dispatcher_remove_view(*view_dispatcher_ref, HelloWorldViewSettings);
        variable_item_list_free(variable_item_list);
        variable_item_list = nullptr;
        variable_item_wifi_ssid = nullptr;
        variable_item_wifi_pass = nullptr;
    }
}

uint32_t HelloWorldSettings::callbackToSettings(void *context)
{
    UNUSED(context);
    return HelloWorldViewSettings;
}

uint32_t HelloWorldSettings::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return HelloWorldViewSubmenu;
}

void HelloWorldSettings::freeTextInput()
{
    if (text_input && view_dispatcher_ref && *view_dispatcher_ref)
    {
        view_dispatcher_remove_view(*view_dispatcher_ref, HelloWorldViewTextInput);
#ifndef FW_ORIGIN_Momentum
        uart_text_input_free(text_input);
#else
        text_input_free(text_input);
#endif
        text_input = nullptr;
    }
    text_input_buffer.reset();
    text_input_temp_buffer.reset();
}

bool HelloWorldSettings::initTextInput(uint32_t view)
{
    // check if already initialized
    if (text_input_buffer || text_input_temp_buffer)
    {
        FURI_LOG_E(TAG, "initTextInput: already initialized");
        return false;
    }

    // init buffers
    text_input_buffer_size = 128;
    if (!easy_flipper_set_buffer(reinterpret_cast<char **>(&text_input_buffer), text_input_buffer_size))
    {
        return false;
    }
    if (!easy_flipper_set_buffer(reinterpret_cast<char **>(&text_input_temp_buffer), text_input_buffer_size))
    {
        return false;
    }

    // app context
    HelloWorldApp *app = static_cast<HelloWorldApp *>(appContext);
    char loaded[256];

    if (view == SettingsViewSSID)
    {
        if (app->loadChar("wifi_ssid", loaded, sizeof(loaded)))
        {
            strncpy(text_input_temp_buffer.get(), loaded, text_input_buffer_size);
        }
        else
        {
            text_input_temp_buffer[0] = '\0'; // Ensure empty if not loaded
        }
        text_input_temp_buffer[text_input_buffer_size - 1] = '\0'; // Ensure null-termination
#ifndef FW_ORIGIN_Momentum
        return easy_flipper_set_uart_text_input(&text_input, HelloWorldViewTextInput,
                                                "Enter SSID", text_input_temp_buffer.get(), text_input_buffer_size,
                                                textUpdatedSsidCallback, callbackToSettings, view_dispatcher_ref, this);
#else
        return easy_flipper_set_text_input(&text_input, HelloWorldViewTextInput,
                                           "Enter SSID", text_input_temp_buffer.get(), text_input_buffer_size,
                                           textUpdatedSsidCallback, callbackToSettings, view_dispatcher_ref, this);
#endif
    }
    else if (view == SettingsViewPassword)
    {
        if (app->loadChar("wifi_pass", loaded, sizeof(loaded)))
        {
            strncpy(text_input_temp_buffer.get(), loaded, text_input_buffer_size);
        }
        else
        {
            text_input_temp_buffer[0] = '\0'; // Ensure empty if not loaded
        }
        text_input_temp_buffer[text_input_buffer_size - 1] = '\0'; // Ensure null-termination
#ifndef FW_ORIGIN_Momentum
        return easy_flipper_set_uart_text_input(&text_input, HelloWorldViewTextInput,
                                                "Enter Password", text_input_temp_buffer.get(), text_input_buffer_size,
                                                textUpdatedPassCallback, callbackToSettings, view_dispatcher_ref, this);
#else
        return easy_flipper_set_text_input(&text_input, HelloWorldViewTextInput,
                                           "Enter Password", text_input_temp_buffer.get(), text_input_buffer_size,
                                           textUpdatedPassCallback, callbackToSettings, view_dispatcher_ref, this);
#endif
    }
    return false;
}

void HelloWorldSettings::settingsItemSelected(uint32_t index)
{
    switch (index)
    {
    case SettingsViewSSID:
    case SettingsViewPassword:
        startTextInput(index);
        break;
    case SettingsViewConnect:
    {
        HelloWorldApp *app = static_cast<HelloWorldApp *>(appContext);
        char loaded_ssid[64];
        char loaded_pass[64];
        if (!app->loadChar("wifi_ssid", loaded_ssid, sizeof(loaded_ssid)) ||
            !app->loadChar("wifi_pass", loaded_pass, sizeof(loaded_pass)))
        {
            FURI_LOG_E(TAG, "WiFi credentials not set");
            easy_flipper_dialog("No WiFi Credentials", "Please set your WiFi SSID\nand Password in Settings.");
        }
        else
        {
            app->sendWiFiCredentials(loaded_ssid, loaded_pass);
        }
    }
    break;
    default:
        break;
    };
}

void HelloWorldSettings::settingsItemSelectedCallback(void *context, uint32_t index)
{
    HelloWorldSettings *settings = (HelloWorldSettings *)context;
    settings->settingsItemSelected(index);
}

bool HelloWorldSettings::startTextInput(uint32_t view)
{
    freeTextInput();
    if (!initTextInput(view))
    {
        FURI_LOG_E(TAG, "Failed to initialize text input for view %lu", view);
        return false;
    }
    if (view_dispatcher_ref && *view_dispatcher_ref)
    {
        view_dispatcher_switch_to_view(*view_dispatcher_ref, HelloWorldViewTextInput);
        return true;
    }
    else
    {
        FURI_LOG_E(TAG, "View dispatcher reference is null or invalid");
        return false;
    }
}

void HelloWorldSettings::textUpdated(uint32_t view)
{
    // store the entered text
    strncpy(text_input_buffer.get(), text_input_temp_buffer.get(), text_input_buffer_size);

    // Ensure null-termination
    text_input_buffer[text_input_buffer_size - 1] = '\0';

    // app context
    HelloWorldApp *app = static_cast<HelloWorldApp *>(appContext);

    switch (view)
    {
    case SettingsViewSSID:
        if (variable_item_wifi_ssid)
        {
            variable_item_set_current_value_text(variable_item_wifi_ssid, text_input_buffer.get());
        }
        app->saveChar("wifi_ssid", text_input_buffer.get());
        break;
    case SettingsViewPassword:
        if (variable_item_wifi_pass)
        {
            variable_item_set_current_value_text(variable_item_wifi_pass, text_input_buffer.get());
        }
        app->saveChar("wifi_pass", text_input_buffer.get());
        break;
    default:
        break;
    }

    // switch to the settings view
    if (view_dispatcher_ref && *view_dispatcher_ref)
    {
        view_dispatcher_switch_to_view(*view_dispatcher_ref, HelloWorldViewSettings);
    }
}

void HelloWorldSettings::textUpdatedSsidCallback(void *context)
{
    HelloWorldSettings *settings = (HelloWorldSettings *)context;
    settings->textUpdated(SettingsViewSSID);
}

void HelloWorldSettings::textUpdatedPassCallback(void *context)
{
    HelloWorldSettings *settings = (HelloWorldSettings *)context;
    settings->textUpdated(SettingsViewPassword);
}
