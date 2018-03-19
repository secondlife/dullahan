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

#include "cef_app.h"

#ifdef WIN32
#include <windows.h>

//#define HOST_PROCESS_REAPER

#ifdef HOST_PROCESS_REAPER
// Ignore c:\program files (x86)\microsoft visual studio 12.0\vc\include\thr\xthread(196): warning C4702: unreachable code
#pragma warning( disable : 4702)
#include <thread>
#include <tlhelp32.h>

// taken from http://magpcss.org/ceforum/viewtopic.php?f=6&t=15817&start=10#p37820
// works around a CEF issue (yet to be filed) where the host process is not destroyed
// after CEF exits in some case on Windows 7
// Making it switchable for now while I investigate it a bit
HANDLE GetParentProcess()
{
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32 ProcessEntry = {};
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(Snapshot, &ProcessEntry))
    {
        DWORD CurrentProcessId = GetCurrentProcessId();

        do
        {
            if (ProcessEntry.th32ProcessID == CurrentProcessId)
            {
                break;
            }
        }
        while (Process32Next(Snapshot, &ProcessEntry));
    }

    CloseHandle(Snapshot);

    return OpenProcess(SYNCHRONIZE, FALSE, ProcessEntry.th32ParentProcessID);
}
#endif

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
#ifdef HOST_PROCESS_REAPER
    HANDLE ParentProcess = GetParentProcess();

    std::thread([ParentProcess]()
    {
        WaitForSingleObject(ParentProcess, INFINITE);
        ExitProcess(0);
    }).detach();
#endif

    CefMainArgs args(GetModuleHandle(nullptr));

    return CefExecuteProcess(args, nullptr, nullptr);
}
#endif

// OS X Helper executable, we can probably share this between Win & Mac
#ifdef __APPLE__

// Entry point function for sub-processes.
int main(int argc, char* argv[])
{
    // Provide CEF with command-line arguments.
    CefMainArgs args(argc, argv);

    // Execute the sub-process.
    return CefExecuteProcess(args, nullptr, nullptr);
}
#endif
