#pragma once
#include "easy_flipper/easy_flipper.h"

class HelloWorldAbout
{
private:
    Widget *widget;
    ViewDispatcher **viewDispatcherRef;

    static constexpr const uint32_t HelloWorldViewSubmenu = 1; // View ID for submenu
    static constexpr const uint32_t HelloWorldViewAbout = 2;   // View ID for about

    static uint32_t callbackToSubmenu(void *context);

public:
    HelloWorldAbout(ViewDispatcher **viewDispatcher);
    ~HelloWorldAbout();
};
