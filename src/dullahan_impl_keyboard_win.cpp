/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    Copyright (c) 2016, Linden Research, Inc.

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

bool isKeyDown(int vkey)
{
    return (GetKeyState(vkey) & 0x8000) != 0;
}

int GetCefKeyboardModifiers(WPARAM wparam, LPARAM lparam)
{
    int modifiers = 0;
    if (isKeyDown(VK_SHIFT))
    {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (isKeyDown(VK_CONTROL))
    {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if (isKeyDown(VK_MENU))
    {
        modifiers |= EVENTFLAG_ALT_DOWN;
    }

    // Low bit set from GetKeyState indicates "toggled".
    if (::GetKeyState(VK_NUMLOCK) & 1)
    {
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    }
    if (::GetKeyState(VK_CAPITAL) & 1)
    {
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
    }

    switch (wparam)
    {
        case VK_RETURN:
            if ((lparam >> 16) & KF_EXTENDED)
            {
                modifiers |= EVENTFLAG_IS_KEY_PAD;
            }
            break;
        case VK_INSERT:
        case VK_DELETE:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_UP:
        case VK_DOWN:
        case VK_LEFT:
        case VK_RIGHT:
            if (!((lparam >> 16) & KF_EXTENDED))
            {
                modifiers |= EVENTFLAG_IS_KEY_PAD;
            }
            break;
        case VK_NUMLOCK:
        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
        case VK_DIVIDE:
        case VK_MULTIPLY:
        case VK_SUBTRACT:
        case VK_ADD:
        case VK_DECIMAL:
        case VK_CLEAR:
            modifiers |= EVENTFLAG_IS_KEY_PAD;
            break;
        case VK_SHIFT:
            if (isKeyDown(VK_LSHIFT))
            {
                modifiers |= EVENTFLAG_IS_LEFT;
            }
            else if (isKeyDown(VK_RSHIFT))
            {
                modifiers |= EVENTFLAG_IS_RIGHT;
            }
            break;
        case VK_CONTROL:
            if (isKeyDown(VK_LCONTROL))
            {
                modifiers |= EVENTFLAG_IS_LEFT;
            }
            else if (isKeyDown(VK_RCONTROL))
            {
                modifiers |= EVENTFLAG_IS_RIGHT;
            }
            break;
        case VK_MENU:
            if (isKeyDown(VK_LMENU))
            {
                modifiers |= EVENTFLAG_IS_LEFT;
            }
            else if (isKeyDown(VK_RMENU))
            {
                modifiers |= EVENTFLAG_IS_RIGHT;
            }
            break;
        case VK_LWIN:
            modifiers |= EVENTFLAG_IS_LEFT;
            break;
        case VK_RWIN:
            modifiers |= EVENTFLAG_IS_RIGHT;
            break;
    }
    return modifiers;
}

void dullahan_impl::nativeKeyboardEventWin(uint32_t msg, uint32_t wparam, uint64_t lparam)
{
    if (mBrowser && mBrowser->GetHost())
    {
        CefKeyEvent event;
        event.windows_key_code = wparam;
        event.native_key_code = (int)lparam;
        event.is_system_key = msg == WM_SYSCHAR || msg == WM_SYSKEYDOWN
                              || msg == WM_SYSKEYUP;

        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
        {
            event.type = KEYEVENT_RAWKEYDOWN;
        }
        else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
        {
            event.type = KEYEVENT_KEYUP;
        }
        else
        {
            event.type = KEYEVENT_CHAR;
        }

        event.modifiers = GetCefKeyboardModifiers((WPARAM)wparam, (LPARAM)lparam);

        mBrowser->GetHost()->SendKeyEvent(event);
    }
}

void dullahan_impl::nativeKeyboardEventOSX(void* event)
{
}

void dullahan_impl::nativeKeyboardEventOSX(dullahan::EKeyEvent event_type, uint32_t event_modifiers,
        uint32_t event_keycode, uint32_t event_chars,
        uint32_t event_umodchars, bool event_isrepeat)
{
}
