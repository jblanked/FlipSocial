#include "run/run.hpp"
#include "app.hpp"

FlipSocialRun::FlipSocialRun(void *appContext) : appContext(appContext), shouldReturnToMenu(false)
{
}

FlipSocialRun::~FlipSocialRun()
{
    // nothing to do
}

void FlipSocialRun::updateDraw(Canvas *canvas)
{
    canvas_clear(canvas);
    canvas_draw_str(canvas, 0, 10, "Hello World Run!");
}

void FlipSocialRun::updateInput(InputEvent *event)
{
    if (event->key == InputKeyBack)
    {
        // return to menu
        shouldReturnToMenu = true;
    }
}
