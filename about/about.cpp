#include "about/about.hpp"

FlipSocialAbout::FlipSocialAbout(ViewDispatcher **viewDispatcher) : widget(nullptr), viewDispatcherRef(viewDispatcher)
{
    easy_flipper_set_widget(&widget, FlipSocialViewAbout, "Simple C++ Flipper app\n\n\n\n\nwww.github.com/jblanked", callbackToSubmenu, viewDispatcherRef);
}

FlipSocialAbout::~FlipSocialAbout()
{
    if (widget && viewDispatcherRef && *viewDispatcherRef)
    {
        view_dispatcher_remove_view(*viewDispatcherRef, FlipSocialViewAbout);
        widget_free(widget);
        widget = nullptr;
    }
}

uint32_t FlipSocialAbout::callbackToSubmenu(void *context)
{
    UNUSED(context);
    return FlipSocialViewSubmenu;
}
