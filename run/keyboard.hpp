#pragma once
#include <stdint.h>
#include <stddef.h>
#include <gui/gui.h>

enum KeyboardMode
{
    KEYBOARD_LOWERCASE = 0,
    KEYBOARD_UPPERCASE = 1,
    KEYBOARD_NUMBERS = 2
};

enum FunctionKey
{
    FUNC_KEY_SPACE = 0,
    FUNC_KEY_BACKSPACE = 1,
    FUNC_KEY_MODE_SWITCH = 2,
    FUNC_KEY_CAPS_LOCK = 3,
    FUNC_KEY_DONE = 4
};

class Keyboard
{
private:
    // Keyboard layouts
    static const char keyboard_lowercase[3][11];
    static const char keyboard_uppercase[3][11];
    static const char keyboard_numbers[3][11];

    // Cursor position
    uint8_t cursor_x;
    uint8_t cursor_y;

    // Keyboard state
    KeyboardMode mode;
    bool caps_lock;

    // Internal text buffer
    static const size_t MAX_TEXT_SIZE = 256;
    char text_buffer[MAX_TEXT_SIZE];

    // Helper methods
    const char (*getCurrentKeyboard())[11];
    const char *getModeName();
    void clampCursorToValidPosition();
    size_t getStringLength(const char *str, size_t max_size);

public:
    Keyboard();
    ~Keyboard();

    // Initialize/reset keyboard state
    void reset();
    void resetText();

    // Input handling
    bool handleInput(uint8_t key);
    bool handleInput(uint8_t key, char *target_buffer, size_t target_size);

    // Text management
    const char *getText() const;
    void setText(const char *text);
    void clearText();
    size_t getTextLength() const;

    // Drawing
    void draw(Canvas *canvas, const char *title);
    void draw(Canvas *canvas, const char *title, const char *current_text);

    // State getters
    uint8_t getCursorX() const { return cursor_x; }
    uint8_t getCursorY() const { return cursor_y; }
    KeyboardMode getMode() const { return mode; }
    bool getCapsLock() const { return caps_lock; }

    // Constants
    static const int KEYBOARD_ROWS = 3;
    static const int KEYBOARD_COLS = 10;
    static const int FUNCTION_ROW = 3;
};