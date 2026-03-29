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

#ifndef _DULLAHAN_PLATFORM_UTILS
#define _DULLAHAN_PLATFORM_UTILS

#ifdef WIN32
#include <windows.h>
#include <tlhelp32.h>

// Retrieves a handle to the parent process with the specified access rights.
// Taken from http://magpcss.org/ceforum/viewtopic.php?f=6&t=15817&start=10#p37820
// Works around a CEF issue where the host process is not destroyed
// after CEF exits in some cases on Windows 7.
inline HANDLE GetParentProcess(DWORD desired_access = SYNCHRONIZE)
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

    return OpenProcess(desired_access, FALSE, ProcessEntry.th32ParentProcessID);
}
#endif // WIN32

#endif // _DULLAHAN_PLATFORM_UTILS
