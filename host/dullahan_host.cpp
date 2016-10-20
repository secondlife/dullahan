/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

    @author Callum Prentice - September 2016

    LICENSE FILE TO GO HERE
*/

#ifndef _DULLAHAN_HOST
#include "cef_app.h"

#include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
    CefMainArgs args(GetModuleHandle(NULL));

    return CefExecuteProcess(args, nullptr, nullptr);
}

#endif // _DULLAHAN_HOST
