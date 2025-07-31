#pragma once
#include "easy_flipper/easy_flipper.h"

class HelloWorldApp;

class HelloWorldRun
{
    void *appContext;        // reference to the app context
    bool shouldReturnToMenu; // Flag to signal return to menu
public:
    HelloWorldRun(void *appContext);
    ~HelloWorldRun();
    //
    bool isActive() const { return shouldReturnToMenu == false; } // Check if the run is active
    void updateDraw(Canvas *canvas);                              // update and draw the run
    void updateInput(InputEvent *event);                          // update input for the run
};
