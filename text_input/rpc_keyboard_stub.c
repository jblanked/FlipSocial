#include "rpc_keyboard.h"

#include <furi.h>

static bool rpc_keyboard_functions_check_version(RpcKeyboardFunctions *stub)
{
    furi_check(stub);
    if (stub->major == 1 && stub->minor > 2)
    {
        return true;
    }
    FURI_LOG_D("RpcKeyboard", "Unsupported version %d.%d", stub->major, stub->minor);
    return false;
}

/**
 * @brief Get the pubsub object for the remote keyboard.
 * @details This function returns the pubsub object, use to subscribe to keyboard events.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @return FuriPubSub* pointer to the pubsub object.
 */
FuriPubSub *rpc_keyboard_get_pubsub(RpcKeyboard *rpc_keyboard)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return NULL;
    }
    return stub->fn_get_pubsub((RpcKeyboard *)rpc_keyboard);
}

/**
 * @brief Enable or disable newline character submitting the text.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] enable true to enable, false to disable.
 */
void rpc_keyboard_newline_enable(RpcKeyboard *rpc_keyboard, bool enable)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return;
    }
    stub->fn_newline_enable((RpcKeyboard *)rpc_keyboard, enable);
}

/**
 * @brief Publish a single key pressed on the remote keyboard.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] character the character that was pressed.
 */
void rpc_keyboard_publish_char(RpcKeyboard *rpc_keyboard, char character)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return;
    }
    stub->fn_publish_char((RpcKeyboard *)rpc_keyboard, character);
}

/**
 * @brief Publish a macro key pressed on the remote keyboard.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] character the macro key that was pressed.
 */
void rpc_keyboard_publish_macro(RpcKeyboard *rpc_keyboard, char macro)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return;
    }
    stub->fn_publish_macro((RpcKeyboard *)rpc_keyboard, macro);
}

/**
 * @brief Get the macro text associated with a macro key.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] macro the macro key.
 * @return char* pointer to the macro text. NULL if the macro key is not set. User must free the memory.
 */
char *rpc_keyboard_get_macro(RpcKeyboard *rpc_keyboard, char macro)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return NULL;
    }
    return stub->fn_get_macro((RpcKeyboard *)rpc_keyboard, macro);
}

/**
 * @brief Set the macro text associated with a macro key.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] macro the macro key.
 * @param[in] value the macro text.
 */
void rpc_keyboard_set_macro(RpcKeyboard *rpc_keyboard, char macro, char *value)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return;
    }
    stub->fn_set_macro((RpcKeyboard *)rpc_keyboard, macro, value);
}

/**
 * @brief Initializes the chatpad and starts listening for keypresses.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 */
void rpc_keyboard_chatpad_start(RpcKeyboard *rpc_keyboard)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return;
    }
    stub->fn_chatpad_start((RpcKeyboard *)rpc_keyboard);
}

/**
 * @brief Stops the chatpad & frees resources.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 */
void rpc_keyboard_chatpad_stop(RpcKeyboard *rpc_keyboard)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return;
    }
    stub->fn_chatpad_stop((RpcKeyboard *)rpc_keyboard);
}

/**
 * @brief Get the status of the chatpad.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @return RpcKeyboardChatpadStatus the status of the chatpad.
 */
RpcKeyboardChatpadStatus rpc_keyboard_chatpad_status(RpcKeyboard *rpc_keyboard)
{
    RpcKeyboardFunctions *stub = (RpcKeyboardFunctions *)rpc_keyboard;
    if (!rpc_keyboard_functions_check_version(stub))
    {
        return RpcKeyboardChatpadStatusError;
    }
    return stub->fn_chatpad_status((RpcKeyboard *)rpc_keyboard);
}
