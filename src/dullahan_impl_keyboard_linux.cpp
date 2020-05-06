#include "dullahan_impl.h"
#include <stdint.h>

uint32_t SDL1_to_Win[ 320 ] = {
  0,   0,   0,   0,   0,   0,   0,   0,   8,   9,   0,   0,  12,  13,   0,   0,   0,   0,   0,  19,   0,   0,   0,   0,   0,   0,   0,  27,   0,   0,   0,   0,
  32,   0,   0,   0,   0,   0,   0, 222,   0,   0,   0,   0, 188, 189, 190, 191,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,   0, 186, 226, 187,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 219, 220, 221,   0,   0,
  223,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,   0,   0,   0,   0,  46,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 110, 111, 106, 109, 107,   0,   0,  38,  40,  39,  37,  45,  36,  35,  33,  34, 112, 113, 114, 115, 116, 117,
  118, 119, 120, 121, 122, 123, 124, 125, 126,   0,   0,   0, 144,  20, 145, 161, 160, 163, 162, 165, 164,   0,   0,  91,  92,   0,   0,  47,  44,   0,   3,  93
};

// See SDL_keysym.h (avoiding the dependency on SDL for just one header here)
enum SDL_KeyPad {
  SDLK_KP0        = 256,
  SDLK_KP1        = 257,
  SDLK_KP2        = 258,
  SDLK_KP3        = 259,
  SDLK_KP4        = 260,
  SDLK_KP5        = 261,
  SDLK_KP6        = 262,
  SDLK_KP7        = 263,
  SDLK_KP8        = 264,
  SDLK_KP9        = 265,
  SDLK_KP_PERIOD      = 266,
  SDLK_KP_DIVIDE      = 267,
  SDLK_KP_MULTIPLY    = 268,
  SDLK_KP_MINUS       = 269,
  SDLK_KP_PLUS        = 270,
  SDLK_KP_ENTER       = 271,
  SDLK_KP_EQUALS      = 272
};

void dullahan_impl::nativeKeyboardEvent( dullahan::EKeyEvent key_event, uint32_t native_scan_code, uint32_t native_virtual_key, uint32_t native_modifiers )
{
	if( !mBrowser || !mBrowser->GetHost())
		return;

	if( native_scan_code < sizeof( SDL1_to_Win ) / sizeof( uint32_t ) )
		native_scan_code = SDL1_to_Win[ native_scan_code ];
	else
		native_scan_code = 0;

	CefKeyEvent event = {};
	event.is_system_key = false;
	event.native_key_code = native_virtual_key;
	event.character = native_virtual_key;
	event.unmodified_character = native_virtual_key;
	event.modifiers = native_modifiers;

	if (native_modifiers & EVENTFLAG_ALT_DOWN)
    {
		event.modifiers &= ~EVENTFLAG_ALT_DOWN;
		event.is_system_key = true;
    }

	if( native_scan_code >= SDLK_KP0 && native_scan_code <= SDLK_KP_EQUALS )
		event.modifiers |= EVENTFLAG_IS_KEY_PAD;

	event.windows_key_code = native_scan_code;

	if (key_event == dullahan::KE_KEY_DOWN)
    {
		event.type = KEYEVENT_RAWKEYDOWN;
		mBrowser->GetHost()->SendKeyEvent(event);
		if (event.character)
        {
			event.type = KEYEVENT_CHAR;
			mBrowser->GetHost()->SendKeyEvent(event);
        }
    }
	else
    {
		if (key_event == dullahan::KE_KEY_UP)
        {
			event.native_key_code |= 0xC0000000;
			event.type = KEYEVENT_KEYUP;
			mBrowser->GetHost()->SendKeyEvent(event);
        }
    }
}

// SDL2

enum SDL_Keymod
{
    KMOD_NONE = 0x0000,
    KMOD_LSHIFT = 0x0001,
    KMOD_RSHIFT = 0x0002,
    KMOD_LCTRL = 0x0040,
    KMOD_RCTRL = 0x0080,
    KMOD_LALT = 0x0100,
    KMOD_RALT = 0x0200,
    KMOD_LGUI = 0x0400,
    KMOD_RGUI = 0x0800,
    KMOD_NUM = 0x1000,
    KMOD_CAPS = 0x2000,
    KMOD_MODE = 0x4000,
    KMOD_RESERVED = 0x8000
};

bool isAltPressed( uint32_t mod )
{
    return 0 != ( mod & (KMOD_LALT|KMOD_RALT));
}

bool isShiftPressed( uint32_t mod )
{
    return 0 != ( mod & (KMOD_LSHIFT|KMOD_RSHIFT));
}

bool isControlPressed( uint32_t mod )
{
    return 0 != ( mod & (KMOD_LCTRL|KMOD_RCTRL));
}

void dullahan_impl::nativeKeyboardEventSDL2( dullahan::EKeyEvent key_event, uint32_t key_data, uint32_t key_modifiers, bool keypad_input )
{
    if( !mBrowser || !mBrowser->GetHost())
        return;

    CefKeyEvent event = {};
    event.is_system_key = false;
    event.modifiers = key_modifiers;

    if( keypad_input )
        event.modifiers |= EVENTFLAG_IS_KEY_PAD;

    if ( isAltPressed( key_modifiers ) )
    {
        event.modifiers &= ~EVENTFLAG_ALT_DOWN;
        event.is_system_key = true;
    }

    if ( isShiftPressed( key_modifiers ) )
        event.modifiers &= ~EVENTFLAG_SHIFT_DOWN;

    if ( isControlPressed( key_modifiers ) )
        event.modifiers &= ~EVENTFLAG_CONTROL_DOWN;

    if( key_event == dullahan::KE_KEY_CHAR )
    {
        event.character = key_data;
        event.type = KEYEVENT_CHAR;
        mBrowser->GetHost()->SendKeyEvent(event);
    }
    else
    {
        event.windows_key_code = key_data;
        
        if (key_event == dullahan::KE_KEY_DOWN)
        {
            event.type = KEYEVENT_RAWKEYDOWN;
            mBrowser->GetHost()->SendKeyEvent(event);
        }
        else if(key_event == dullahan::KE_KEY_UP)
        {
            event.native_key_code |= 0xC0000000;
            event.type = KEYEVENT_KEYUP;
            mBrowser->GetHost()->SendKeyEvent(event);
        }
    }
}
