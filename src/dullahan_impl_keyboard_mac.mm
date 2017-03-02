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

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#endif

#include "dullahan_impl.h"

namespace DullahanImplMacAssist
{
    uint32_t modifiersForModifierFlags(uint32_t modifierFlags)
    {
        uint32_t modifers = EVENTFLAG_NONE;
        
        if (modifierFlags & NSAlphaShiftKeyMask)
            modifers |= EVENTFLAG_CAPS_LOCK_ON;
        
        if (modifierFlags & NSShiftKeyMask)
            modifers |= EVENTFLAG_SHIFT_DOWN;
        
        if (modifierFlags & NSControlKeyMask)
            modifers |= EVENTFLAG_CONTROL_DOWN;
        
        if (modifierFlags & NSAlternateKeyMask)
            modifers |= EVENTFLAG_ALT_DOWN;
        
        if (modifierFlags & NSCommandKeyMask)
            modifers |= EVENTFLAG_COMMAND_DOWN;
        
        if (modifierFlags & NSNumericPadKeyMask)
            modifers |= EVENTFLAG_IS_KEY_PAD;
        
        return modifers;
    }
}

void dullahan_impl::nativeKeyboardEventWin(uint32_t msg, uint32_t wparam, uint64_t lparam)
{
}

void dullahan_impl::nativeKeyboardEventOSX(void* event)
{
    if (mBrowser.get())
    {
        if (mBrowser->GetHost())
        {
            static uint32_t lastModifiers = DullahanImplMacAssist::modifiersForModifierFlags([NSEvent modifierFlags]);
            static uint32_t newModifiers = DullahanImplMacAssist::modifiersForModifierFlags([NSEvent modifierFlags]);

            NSEvent* ns_event = (NSEvent*)event;

            lastModifiers = newModifiers;
            newModifiers = DullahanImplMacAssist::modifiersForModifierFlags([ns_event modifierFlags]);

            if (([ns_event type] == NSKeyDown) || ([ns_event type] == NSKeyUp))
            {
                CefKeyEvent keyEvent;
                
                NSString *c = [ns_event characters];
                if ([c length] > 0)
                {
                    keyEvent.character = [c characterAtIndex:0];
                }
                
                NSString *cim = [ns_event charactersIgnoringModifiers];
                if ([cim length] > 0)
                {
                    keyEvent.unmodified_character = [cim characterAtIndex:0];
                }
                
                keyEvent.native_key_code = [ns_event keyCode];
                keyEvent.is_system_key = false;
                keyEvent.modifiers = newModifiers;

                if ([ns_event type] == NSKeyDown)
                {
                    keyEvent.type =  KEYEVENT_KEYDOWN;
                    mBrowser->GetHost()->SendKeyEvent(keyEvent);

                    keyEvent.type =  KEYEVENT_CHAR;
                    mBrowser->GetHost()->SendKeyEvent(keyEvent);
                }
                else
                if ([ns_event type] == NSKeyUp)
                {
                    keyEvent.type =  KEYEVENT_KEYUP;
                }
            }
        }
    }
}
