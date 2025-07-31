#include "run/run.hpp"
#include "app.hpp"

FlipSocialRun::FlipSocialRun(void *appContext) : appContext(appContext),
                                                 currentMenuIndex(SocialViewFeed), currentView(SocialViewMenu),
                                                 inputHeld(false), lastInput(InputKeyMAX),
                                                 shouldDebounce(false), shouldReturnToMenu(false)
{
}

FlipSocialRun::~FlipSocialRun()
{
    // nothing to do
}

void FlipSocialRun::debounceInput()
{
    static uint8_t debounceCounter = 0;
    if (shouldDebounce)
    {
        lastInput = InputKeyMAX;
        debounceCounter++;
        if (debounceCounter < 2)
        {
            return;
        }
        debounceCounter = 0;
        shouldDebounce = false;
        inputHeld = false;
    }
}

void FlipSocialRun::drawMenu(Canvas *canvas, uint8_t selectedIndex, const char **menuItems, uint8_t menuCount)
{
    canvas_clear(canvas);

    // Draw title
    canvas_set_font_custom(canvas, FONT_SIZE_LARGE);
    const char *title = "FlipSocial";
    int title_width = canvas_string_width(canvas, title);
    int title_x = (128 - title_width) / 2; // Center the title
    canvas_draw_str(canvas, title_x, 12, title);

    // Draw underline for title
    canvas_draw_line(canvas, title_x, 14, title_x + title_width, 14);

    // Draw decorative horizontal pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 18);
    }

    // Menu items - horizontal scrolling, show one at a time
    canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);

    // Display current menu item centered
    const char *currentItem = menuItems[selectedIndex];
    int text_width = canvas_string_width(canvas, currentItem);
    int text_x = (128 - text_width) / 2;
    int menu_y = 40; // Centered vertically

    // Draw main selection box for current item
    canvas_draw_rbox(canvas, 10, menu_y - 8, 108, 16, 4);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_str(canvas, text_x, menu_y + 4, currentItem);
    canvas_set_color(canvas, ColorBlack);

    // Draw navigation arrows
    if (selectedIndex > 0)
    {
        canvas_draw_str(canvas, 2, menu_y + 4, "<");
    }
    if (selectedIndex < (menuCount - 1))
    {
        canvas_draw_str(canvas, 122, menu_y + 4, ">");
    }

    // Draw page indicator dots
    int dots_spacing = 6;
    int dots_start_x = (128 - (menuCount * dots_spacing)) / 2; // Center dots with spacing
    for (int i = 0; i < menuCount; i++)
    {
        int dot_x = dots_start_x + (i * dots_spacing);
        if (i == selectedIndex)
        {
            canvas_draw_box(canvas, dot_x, menu_y + 12, 4, 4); // Filled dot for current
        }
        else
        {
            canvas_draw_frame(canvas, dot_x, menu_y + 12, 4, 4); // Empty dot for others
        }
    }

    // Draw decorative bottom pattern
    for (int i = 0; i < 128; i += 4)
    {
        canvas_draw_dot(canvas, i, 58);
    }
}

void FlipSocialRun::updateDraw(Canvas *canvas)
{
    canvas_clear(canvas);
    switch (currentView)
    {
    case SocialViewMenu:
    {
        const char *menuItems[] = {"Feed", "Messages", "Profile"};
        drawMenu(canvas, (uint8_t)currentMenuIndex, menuItems, 3);
        break;
    }
    default:
        canvas_set_font_custom(canvas, FONT_SIZE_MEDIUM);
        canvas_draw_str(canvas, 10, 30, "View not implemented yet.");
        break;
    };
}

void FlipSocialRun::updateInput(InputEvent *event)
{
    lastInput = event->key;
    debounceInput();
    switch (currentView)
    {
    case SocialViewMenu:
    {
        switch (lastInput)
        {
        case InputKeyBack:
            // return to menu
            shouldReturnToMenu = true;
            break;
        case InputKeyDown:
        case InputKeyLeft:
            if (currentMenuIndex == SocialViewMessages)
            {
                currentMenuIndex = SocialViewFeed;
            }
            else if (currentMenuIndex == SocialViewProfile)
            {
                currentMenuIndex = SocialViewMessages;
                shouldDebounce = true;
            }
            break;
        case InputKeyUp:
        case InputKeyRight:
            if (currentMenuIndex == SocialViewFeed)
            {
                currentMenuIndex = SocialViewMessages;
                shouldDebounce = true;
            }
            else if (currentMenuIndex == SocialViewMessages)
            {
                currentMenuIndex = SocialViewProfile;
            }
            break;
        default:
            break;
        }
    }
    break;
    default:
        // Handle other views or default behavior
        break;
    };
}
