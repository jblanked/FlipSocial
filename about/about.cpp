#include "about/about.hpp"

HelloWorldAbout::HelloWorldAbout(ViewDispatcher **viewDispatcher) : widget(nullptr), viewDispatcherRef(viewDispatcher)
{
    easy_flipper_set_widget(&widget, HelloWorldViewAbout, "Simple C++ Flipper app\n\n\n\n\nwww.github.com/jblanked", callbackToSubmenu, viewDispatcherRef);
}

HelloWorldAbout::~HelloWorldAbout()
{
    if (widget && viewDispatcherRef && *viewDispatcherRef)
    {
        view_dispatcher_remove_view(*viewDispatcherRef, HelloWorldViewAbout);
        widget_free(widget);
        widget = nullptr;
    }
}

uint32_t HelloWorldAbout::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return HelloWorldViewSubmenu;
}
