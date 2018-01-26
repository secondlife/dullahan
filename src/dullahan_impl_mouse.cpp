/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Callum Prentice 2017

    Copyright (c) 2017, Linden Research, Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
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
        cef_mouse_event.y = getFlipMouseY() ? (mViewHeight - y) : y;
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
        bool is_up = true;
        int last_click_count = 1;
        if (mouse_event == dullahan::ME_MOUSE_DOWN)
        {
            is_up = false;
        }
        else if (mouse_event == dullahan::ME_MOUSE_DOUBLE_CLICK)
        {
            last_click_count = 2;
            is_up = false;
        }

        mBrowser->GetHost()->SendMouseClickEvent(cef_mouse_event, btnType, is_up, last_click_count);
    }
};

void dullahan_impl::mouseMove(int x, int y)
{
    if (mBrowser && mBrowser->GetHost())
    {
        CefMouseEvent cef_mouse_event;
        cef_mouse_event.x = x;
        cef_mouse_event.y = getFlipMouseY() ? (mViewHeight - y) : y;
		cef_mouse_event.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;

        bool mouse_leave = false;
        mBrowser->GetHost()->SendMouseMoveEvent(cef_mouse_event, mouse_leave);
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
