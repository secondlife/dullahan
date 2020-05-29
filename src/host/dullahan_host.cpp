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

#ifdef __linux__
int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);
    return CefExecuteProcess(main_args, NULL, NULL);
}
#endif
#ifdef WIN32
#include <windows.h>

#define HOST_PROCESS_REAPER

#ifdef HOST_PROCESS_REAPER
// Ignore c:\program files (x86)\microsoft visual studio 12.0\vc\include\thr\xthread(196): warning C4702: unreachable code
#pragma warning( disable : 4702)
#include <thread>
#include <tlhelp32.h>

/*
  Nasty hack to stop flash from displaying a popup with "NO SANDBOX"
  Flashplayer will try to spawn a cmd.exe and echo this message into it, we
  use a process group to limit the number of processes allowed to 1, thus preventing
  popup.

  Limitation: NeedsWindows 8 or higher, the viewer already does put SLPlugin (and with that
  all sub processes) into a job, so all plugin instances get killed when the viewer does exit.
  Anything before Windows 8 will not allow a process being part of more than one job.

  Using the sandbox would fix this problem, but for using the sandbox the same executable
  must be used  for browser and all sub processes (see cef_sandbox_win.h); but the viewer
  uses slplugin.exe and llceflib_host.exe.
*/
void enablePPAPIFlashHack(LPSTR lpCmdLine)
{
    if (!lpCmdLine)
    {
        return;
    }

    std::string strCmdLine = lpCmdLine;

    std::string strType = "--type=ppapi";
    std::string::size_type i = strCmdLine.find(strType);

    if (i == std::string::npos)
    {
        return;
    }

    HANDLE hJob = CreateJobObject(nullptr, nullptr);
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ::GetCurrentProcessId());

    if (!AssignProcessToJobObject(hJob, hProc))
    {
        ::CloseHandle(hProc);
        ::CloseHandle(hJob);
        return;
    }

    JOBOBJECT_BASIC_LIMIT_INFORMATION baseLimits = {};
    baseLimits.LimitFlags = JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
    baseLimits.ActiveProcessLimit = 1;

    SetInformationJobObject(hJob, JobObjectBasicLimitInformation, &baseLimits, sizeof(baseLimits));

    ::CloseHandle(hProc);
    ::CloseHandle(hJob);
}

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

    enablePPAPIFlashHack(lpCmdLine);

    return CefExecuteProcess(args, nullptr, nullptr);
}
#endif

// OS X Helper executable, we can probably share this between Win & Mac
#ifdef __APPLE__
#include "include/wrapper/cef_library_loader.h"

// Entry point function for sub-processes.
int main(int argc, char* argv[])
{
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper())
    {
        return 1;
    }

    // Provide CEF with command-line arguments.
    CefMainArgs args(argc, argv);

    // Execute the sub-process.
    return CefExecuteProcess(args, nullptr, nullptr);
}
#endif
