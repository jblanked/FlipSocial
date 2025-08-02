#include "keyboard.hpp"
#include "font/font.h"
#include <string.h>

const char Keyboard::keyboard_lowercase[3][11] = {
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\0'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '-', '\0'},
    {'z', 'x', 'c', 'v', 'b', 'n', 'm', '.', '_', '/', '\0'}};

const char Keyboard::keyboard_uppercase[3][11] = {
    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '\0'},
    {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '-', '\0'},
    {'Z', 'X', 'C', 'V', 'B', 'N', 'M', '.', '_', '/', '\0'}};

const char Keyboard::keyboard_numbers[3][11] = {
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\0'},
    {'@', '#', '$', '%', '&', '*', '+', '=', '?', '!', '\0'},
    {'(', ')', '[', ']', '{', '}', '<', '>', '|', '\\', '\0'}};

Keyboard::Keyboard()
{
    reset();
    resetText();
}

Keyboard::~Keyboard()
{
    // Destructor - text buffer is automatically cleaned up
}

void Keyboard::reset()
{
    cursor_x = 0;
    cursor_y = 0;
    mode = KEYBOARD_LOWERCASE;
    caps_lock = false;
}

void Keyboard::resetText()
{
    text_buffer[0] = '\0';
}

const char (*Keyboard::getCurrentKeyboard())[11]
{
    switch (mode)
    {
    case KEYBOARD_UPPERCASE:
        return keyboard_uppercase;
    case KEYBOARD_NUMBERS:
        return keyboard_numbers;
    default:
        return keyboard_lowercase;
    }
}

const char *Keyboard::getModeName()
{
    switch (mode)
    {
    case KEYBOARD_UPPERCASE:
        return caps_lock ? "CAPS" : "ABC";
    case KEYBOARD_NUMBERS:
        return "123";
    default:
        return "abc";
    }
}

void Keyboard::clampCursorToValidPosition()
{
    const char (*keyboard)[11] = getCurrentKeyboard();

    if (cursor_y < KEYBOARD_ROWS)
    {
        // Clamp to valid position in keyboard grid
        if (cursor_x > KEYBOARD_COLS - 1)
            cursor_x = KEYBOARD_COLS - 1;

        while (cursor_x > 0 && keyboard[cursor_y][cursor_x] == '\0')
        {
            cursor_x--;
        }
    }
}

size_t Keyboard::getStringLength(const char *str, size_t max_size)
{
    size_t len = 0;
    while (len < max_size - 1 && str[len] != '\0')
    {
        len++;
    }
    return len;
}

bool Keyboard::handleInput(uint8_t key)
{
    return handleInput(key, text_buffer, MAX_TEXT_SIZE);
}

bool Keyboard::handleInput(uint8_t key, char *target_buffer, size_t target_size)
{
    const char (*keyboard)[11] = getCurrentKeyboard();

    switch (key)
    {
    case InputKeyLeft:
        if (cursor_y < KEYBOARD_ROWS)
        { // Main keyboard area
            if (cursor_x > 0)
            {
                cursor_x--;
            }
            else
            {
                // Wrap to end of row
                cursor_x = KEYBOARD_COLS - 1;
                while (cursor_x > 0 && keyboard[cursor_y][cursor_x] == '\0')
                {
                    cursor_x--;
                }
            }
        }
        else
        { // Function key row
            if (cursor_x > 0)
            {
                cursor_x--;
            }
            else
            {
                // Wrap to last function key
                cursor_x = (mode == KEYBOARD_NUMBERS) ? 3 : 4; // No caps in number mode
            }
        }
        break;

    case InputKeyRight:
        if (cursor_y < KEYBOARD_ROWS)
        { // Main keyboard area
            if (cursor_x < KEYBOARD_COLS - 1 && keyboard[cursor_y][cursor_x + 1] != '\0')
            {
                cursor_x++;
            }
            else
            {
                // Wrap to beginning of row
                cursor_x = 0;
            }
        }
        else
        {                                                          // Function key row
            int max_func_key = (mode == KEYBOARD_NUMBERS) ? 3 : 4; // No caps in number mode
            if (cursor_x < max_func_key)
            {
                cursor_x++;
            }
            else
            {
                cursor_x = 0;
            }
        }
        break;

    case InputKeyUp:
        if (cursor_y > 0)
        {
            cursor_y--;
            if (cursor_y < KEYBOARD_ROWS)
            { // Moving to main keyboard
                clampCursorToValidPosition();
            }
        }
        else
        {
            // Wrap to function key row
            cursor_y = FUNCTION_ROW;
            if (cursor_x > 4)
                cursor_x = 4;
        }
        break;

    case InputKeyDown:
        if (cursor_y < KEYBOARD_ROWS)
        {
            if (cursor_y < KEYBOARD_ROWS - 1)
            { // Can move down within keyboard
                cursor_y++;
                clampCursorToValidPosition();
            }
            else
            { // Move to function keys
                cursor_y = FUNCTION_ROW;
                if (cursor_x > 4)
                    cursor_x = 4;
            }
        }
        else
        {
            // Wrap to top row
            cursor_y = 0;
            if (cursor_x > KEYBOARD_COLS - 1)
                cursor_x = KEYBOARD_COLS - 1;
        }
        break;

    case InputKeyOk:
        if (cursor_y == FUNCTION_ROW)
        { // Function key row
            switch (cursor_x)
            {
            case FUNC_KEY_SPACE:
            {
                size_t len = getStringLength(target_buffer, target_size);
                if (len < target_size - 1)
                {
                    target_buffer[len] = ' ';
                    target_buffer[len + 1] = '\0';
                }
                break;
            }

            case FUNC_KEY_BACKSPACE:
            {
                size_t len = getStringLength(target_buffer, target_size);
                if (len > 0)
                {
                    target_buffer[len - 1] = '\0';
                }
                break;
            }

            case FUNC_KEY_MODE_SWITCH:
                if (mode == KEYBOARD_NUMBERS)
                {
                    mode = caps_lock ? KEYBOARD_UPPERCASE : KEYBOARD_LOWERCASE;
                }
                else
                {
                    mode = KEYBOARD_NUMBERS;
                }
                break;

            case FUNC_KEY_CAPS_LOCK:
                if (mode != KEYBOARD_NUMBERS)
                {
                    caps_lock = !caps_lock;
                    mode = caps_lock ? KEYBOARD_UPPERCASE : KEYBOARD_LOWERCASE;
                }
                break;

            case FUNC_KEY_DONE:
                return true; // Signal that input is complete
            }
        }
        else
        { // Main keyboard area
            char ch = keyboard[cursor_y][cursor_x];
            if (ch != '\0')
            {
                size_t len = getStringLength(target_buffer, target_size);
                if (len < target_size - 1)
                {
                    target_buffer[len] = ch;
                    target_buffer[len + 1] = '\0';

                    // Auto-switch to lowercase after typing a letter in caps mode (not caps lock)
                    if (mode == KEYBOARD_UPPERCASE && !caps_lock &&
                        ((ch >= 'A' && ch <= 'Z')))
                    {
                        mode = KEYBOARD_LOWERCASE;
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    return false; // Input not complete
}

const char *Keyboard::getText() const
{
    return text_buffer;
}

void Keyboard::setText(const char *text)
{
    if (text != nullptr)
    {
        strncpy(text_buffer, text, MAX_TEXT_SIZE - 1);
        text_buffer[MAX_TEXT_SIZE - 1] = '\0';
    }
    else
    {
        text_buffer[0] = '\0';
    }
}

void Keyboard::clearText()
{
    text_buffer[0] = '\0';
}

size_t Keyboard::getTextLength() const
{
    return strlen(text_buffer);
}

void Keyboard::draw(Canvas *canvas, const char *title)
{
    draw(canvas, title, text_buffer);
}

void Keyboard::draw(Canvas *canvas, const char *title, const char *current_text)
{
    canvas_clear(canvas);
    const char (*keyboard)[11] = getCurrentKeyboard();

    // Draw title and current input
    canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
    canvas_draw_str(canvas, 0, 8, title);
    canvas_draw_str(canvas, strlen(title) * 6 + 6, 8, current_text);

    // Draw mode indicator in top right
    canvas_draw_str(canvas, 100, 8, getModeName());

    // Draw compact 3x10 virtual keyboard
    for (int row = 0; row < KEYBOARD_ROWS; row++)
    {
        for (int col = 0; col < KEYBOARD_COLS; col++)
        {
            char ch = keyboard[row][col];
            if (ch == '\0')
                break;

            int x = 3 + col * 12;
            int y = 22 + row * 10;

            // Highlight current cursor position
            if (row == cursor_y && col == cursor_x)
            {
                canvas_draw_rbox(canvas, x - 1, y - 7, 11, 9, 1);
                canvas_set_color(canvas, ColorWhite);
            }

            // Draw character
            char str[2] = {ch, '\0'};
            canvas_draw_str(canvas, x + 1, y, str);
            canvas_set_color(canvas, ColorBlack);
        }
    }

    // Draw function keys below the main keyboard
    int func_y = 55;

    // Space bar (wide button)
    bool space_selected = (cursor_y == FUNCTION_ROW && cursor_x == FUNC_KEY_SPACE);
    if (space_selected)
    {
        canvas_draw_rbox(canvas, 3, func_y - 7, 30, 9, 1);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str(canvas, 10, func_y, "SPACE");
    canvas_set_color(canvas, ColorBlack);

    // Backspace
    bool backspace_selected = (cursor_y == FUNCTION_ROW && cursor_x == FUNC_KEY_BACKSPACE);
    if (backspace_selected)
    {
        canvas_draw_rbox(canvas, 35, func_y - 7, 20, 9, 1);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str(canvas, 38, func_y, "DEL");
    canvas_set_color(canvas, ColorBlack);

    // Shift/Mode
    bool shift_selected = (cursor_y == FUNCTION_ROW && cursor_x == FUNC_KEY_MODE_SWITCH);
    if (shift_selected)
    {
        canvas_draw_rbox(canvas, 57, func_y - 7, 20, 9, 1);
        canvas_set_color(canvas, ColorWhite);
    }
    if (mode == KEYBOARD_NUMBERS)
    {
        canvas_draw_str(canvas, 60, func_y, "ABC");
    }
    else
    {
        canvas_draw_str(canvas, 60, func_y, "123");
    }
    canvas_set_color(canvas, ColorBlack);

    // Caps Lock (only show in letter modes)
    if (mode != KEYBOARD_NUMBERS)
    {
        bool caps_selected = (cursor_y == FUNCTION_ROW && cursor_x == FUNC_KEY_CAPS_LOCK);
        if (caps_selected)
        {
            canvas_draw_rbox(canvas, 79, func_y - 7, 20, 9, 1);
            canvas_set_color(canvas, ColorWhite);
        }
        if (caps_lock)
        {
            canvas_draw_box(canvas, 81, func_y - 5, 16, 5); // Filled when caps lock is on
            canvas_set_color(canvas, ColorWhite);
        }
        canvas_draw_str(canvas, 82, func_y, "CAPS");
        canvas_set_color(canvas, ColorBlack);
    }

    // Done/Enter
    bool done_selected = (cursor_y == FUNCTION_ROW && cursor_x == FUNC_KEY_DONE);
    if (done_selected)
    {
        canvas_draw_rbox(canvas, 101, func_y - 7, 25, 9, 1);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_str(canvas, 105, func_y, "DONE");
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font_custom(canvas, FONT_SIZE_SMALL);
    canvas_draw_str(canvas, 0, 64, "↑↓←→ OK=Select");
}