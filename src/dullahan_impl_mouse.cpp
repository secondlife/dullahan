/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
*/

#include "dullahan_impl.h"

void dullahan_impl::mouseButton(dullahan::EMouseButton mouse_button,
                                dullahan::EMouseEvent mouse_event, int x, int y)
{
    // send to CEF
    if (mBrowser && mBrowser->GetHost())
    {
        // set click location
        CefMouseEvent cef_mouse_event;
        cef_mouse_event.x = x;
        cef_mouse_event.y = y;
        cef_mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;

        // set button
        CefBrowserHost::MouseButtonType btnType = MBT_LEFT;
        if (mouse_button == dullahan::MB_MOUSE_BUTTON_RIGHT)
        {
            btnType = MBT_RIGHT;
        }
        if (mouse_button == dullahan::MB_MOUSE_BUTTON_MIDDLE)
        {
            btnType = MBT_MIDDLE;
        }

        // set click type
        bool is_down = false;
        int last_click_count = 1;
        if (mouse_event == dullahan::ME_MOUSE_DOWN)
        {
            is_down = true;
        }
        else if (mouse_event == dullahan::ME_MOUSE_UP)
        {
            is_down = false;
        }
        else if (mouse_event == dullahan::ME_MOUSE_DOUBLE_CLICK)
        {
            last_click_count = 2;
            is_down = true;
        }

        mBrowser->GetHost()->SendMouseClickEvent(cef_mouse_event, btnType,
                is_down ? false : true, last_click_count);
    }
};

void dullahan_impl::mouseMove(int x, int y)
{
    if (mBrowser && mBrowser->GetHost())
    {
        CefMouseEvent mouse_event;
        mouse_event.x = x;
        mouse_event.y = y;
        mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;

        bool mouse_leave = false;
        mBrowser->GetHost()->SendMouseMoveEvent(mouse_event, mouse_leave);
    }
};

void dullahan_impl::mouseWheel(int deltaX, int deltaY)
{
    if (mBrowser && mBrowser->GetHost())
    {
        CefMouseEvent mouse_event;
        mouse_event.modifiers = 0;
        mBrowser->GetHost()->SendMouseWheelEvent(mouse_event, 0, deltaY);
    }
}
