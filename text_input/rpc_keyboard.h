#pragma once

#include <core/common_defines.h>
#include <core/mutex.h>
#include <core/pubsub.h>

#define RECORD_RPC_KEYBOARD "rpckeyboard"

#define RPC_KEYBOARD_KEY_RIGHT '\x13'
#define RPC_KEYBOARD_KEY_LEFT '\x14'
#define RPC_KEYBOARD_KEY_ENTER '\x0D'
#define RPC_KEYBOARD_KEY_BACKSPACE '\x08'

typedef enum
{
    // Unknown error occurred
    RpcKeyboardChatpadStatusError,
    // The chatpad worker is stopped
    RpcKeyboardChatpadStatusStopped,
    // The chatpad worker is started, but not ready
    RpcKeyboardChatpadStatusStarted,
    // The chatpad worker is ready and got response from chatpad
    RpcKeyboardChatpadStatusReady,
} RpcKeyboardChatpadStatus;

typedef struct RpcKeyboard RpcKeyboard;

typedef enum
{
    // Replacement text was provided by the user
    RpcKeyboardEventTypeTextEntered,
    // A single character was provided by the user
    RpcKeyboardEventTypeCharEntered,
    // A macro was entered by the user
    RpcKeyboardEventTypeMacroEntered,
} RpcKeyboardEventType;

typedef struct
{
    // The mutex to protect the data, call furi_mutex_acquire/furi_mutex_release.
    FuriMutex *mutex;
    // The text message, macro or character.
    char message[256];
    // The length of the message.
    uint16_t length;
    // The newline enabled flag, allow newline to submit text.
    bool newline_enabled;
} RpcKeyboardEventData;

typedef struct
{
    RpcKeyboardEventType type;
    RpcKeyboardEventData data;
} RpcKeyboardEvent;

typedef FuriPubSub *(*RpcKeyboardGetPubsub)(RpcKeyboard *rpc_keyboard);
typedef void (*RpcKeyboardNewlineEnable)(RpcKeyboard *rpc_keyboard, bool enable);
typedef void (*RpcKeyboardPublishCharFn)(RpcKeyboard *keyboard, char character);
typedef void (*RpcKeyboardPublishMacroFn)(RpcKeyboard *rpc_keyboard, char macro);
typedef char *(*RpcKeyboardGetMacroFn)(RpcKeyboard *rpc_keyboard, char macro);
typedef void (*RpcKeyboardSetMacroFn)(RpcKeyboard *rpc_keyboard, char macro, char *value);
typedef void (*RpcKeyboardChatpadStartFn)(RpcKeyboard *rpc_keyboard);
typedef void (*RpcKeyboardChatpadStopFn)(RpcKeyboard *rpc_keyboard);
typedef RpcKeyboardChatpadStatus (*RpcKeyboardChatpadStatusFn)(RpcKeyboard *rpc_keyboard);

typedef struct RpcKeyboardFunctions RpcKeyboardFunctions;
struct RpcKeyboardFunctions
{
    uint16_t major;
    uint16_t minor;
    RpcKeyboardGetPubsub fn_get_pubsub;
    RpcKeyboardNewlineEnable fn_newline_enable;
    RpcKeyboardPublishCharFn fn_publish_char;
    RpcKeyboardPublishMacroFn fn_publish_macro;
    RpcKeyboardGetMacroFn fn_get_macro;
    RpcKeyboardSetMacroFn fn_set_macro;
    RpcKeyboardChatpadStartFn fn_chatpad_start;
    RpcKeyboardChatpadStopFn fn_chatpad_stop;
    RpcKeyboardChatpadStatusFn fn_chatpad_status;
};

/**
 * @brief STARTUP - Register the remote keyboard.
 */
void rpc_keyboard_register(void);

/**
 * @brief UNUSED - Unregister the remote keyboard.
 */
void rpc_keyboard_release(void);

/**
 * @brief Get the pubsub object for the remote keyboard.
 * @details This function returns the pubsub object, use to subscribe to keyboard events.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @return FuriPubSub* pointer to the pubsub object.
 */
FuriPubSub *rpc_keyboard_get_pubsub(RpcKeyboard *rpc_keyboard);

/**
 * @brief Enable or disable newline character submitting the text.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] enable true to enable, false to disable.
 */
void rpc_keyboard_newline_enable(RpcKeyboard *rpc_keyboard, bool enable);

/**
 * @brief Publish the replacement text to the remote keyboard.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] bytes pointer to the text buffer.
 * @param[in] buffer_size size of the text buffer.
 */
void rpc_keyboard_publish_text(RpcKeyboard *rpc_keyboard, uint8_t *bytes, uint32_t buffer_size);

/**
 * @brief Publish a single key pressed on the remote keyboard.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] character the character that was pressed.
 */
void rpc_keyboard_publish_char(RpcKeyboard *rpc_keyboard, char character);

/**
 * @brief Publish a macro key pressed on the remote keyboard.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] character the macro key that was pressed.
 */
void rpc_keyboard_publish_macro(RpcKeyboard *rpc_keyboard, char macro);

/**
 * @brief Get the macro text associated with a macro key.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] macro the macro key.
 * @return char* pointer to the macro text. NULL if the macro key is not set. User must free the memory.
 */
char *rpc_keyboard_get_macro(RpcKeyboard *rpc_keyboard, char macro);

/**
 * @brief Set the macro text associated with a macro key.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @param[in] macro the macro key.
 * @param[in] value the macro text.
 */
void rpc_keyboard_set_macro(RpcKeyboard *rpc_keyboard, char macro, char *value);

/**
 * @brief Initializes the chatpad and starts listening for keypresses.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 */
void rpc_keyboard_chatpad_start(RpcKeyboard *rpc_keyboard);

/**
 * @brief Stops the chatpad & frees resources.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 */
void rpc_keyboard_chatpad_stop(RpcKeyboard *rpc_keyboard);

/**
 * @brief Get the status of the chatpad.
 * @param[in] rpc_keyboard pointer to the RECORD_RPC_KEYBOARD.
 * @return RpcKeyboardChatpadStatus the status of the chatpad.
 */
RpcKeyboardChatpadStatus rpc_keyboard_chatpad_status(RpcKeyboard *rpc_keyboard);
