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

#define NOMINMAX

#include "cef_app.h"

// Shared by Windows and Mac sub-process entry points
class JSONtoCPPHandler : public CefV8Handler
{
    public:
        JSONtoCPPHandler(CefRefPtr<CefBrowser> browser) :
            // Save the browser reference from OnContextCreated()
            // for later use in IPC communication
            mBrowser(browser)
        {
        }

        bool Execute(const CefString& name,
                     CefRefPtr<CefV8Value> object,
                     const CefV8ValueList& arguments,
                     CefRefPtr<CefV8Value>& retval,
                     CefString& exception) override
        {
            if (name == "JSONtoCPP")
            {
                // Check args
                if (arguments.size() != 2 || ! arguments[0]->IsString() || ! arguments[1]->IsString())
                {
                    exception = "JSONtoCPP(json, id) expects two string arguments";
                    return true;
                }

                // Send the JSON string to the browser process and return an acknowledgment
                std::string id = arguments[0]->GetStringValue();
                std::string json = arguments[1]->GetStringValue();
                std::string result = DoJSONtoCPP(id, json);
                retval = CefV8Value::CreateString(result);

                // Indicate we handled the function call, even 
                // if there was an error in DoJSONtoCPP
                return true;
            }

            return false;
        }

    private:
        std::string DoJSONtoCPP(const std::string& id, const std::string& json)
        {
            // Send the JSON string to the browser process via IPC
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JSONtoCPP_MSG");
            CefRefPtr<CefListValue> args = msg->GetArgumentList();
            args->SetString(0, id);
            args->SetString(1, json);
            mBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);

            // Acknowledge receipt of the JSON string
            return "{\"ACK\": true}";
        }

        CefRefPtr<CefBrowser> mBrowser;
        IMPLEMENT_REFCOUNTING(JSONtoCPPHandler);
};

class MyApp : public CefApp,
              public CefRenderProcessHandler
{
public:
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
    {
        return this;
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override
    {
        CefRefPtr<CefV8Value> global = context->GetGlobal();
        CefRefPtr<CefV8Handler> handler = new JSONtoCPPHandler(browser);
        CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("JSONtoCPP", handler);
        global->SetValue("JSONtoCPP", func, V8_PROPERTY_ATTRIBUTE_NONE);

        // Friendly greeting to the browser process to confirm the context was created
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JSONtoCPP_MSG");
        CefRefPtr<CefListValue> args = msg->GetArgumentList();
        args->SetString(0, "INFO");
        args->SetString(1, "Hello from the OnContextCreated in the sub-process!");
        browser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
    }

private:
    IMPLEMENT_REFCOUNTING(MyApp);
};

#ifdef __linux__
#if defined(NO_STACK_PROTECTOR)
NO_STACK_PROTECTOR
#endif
int main(int argc, char* argv[])
{
    CefMainArgs main_args(argc, argv);
    return CefExecuteProcess(main_args, nullptr, nullptr);
}
#endif
#ifdef WIN32
#include <windows.h>

#define HOST_PROCESS_REAPER

#ifdef HOST_PROCESS_REAPER
// Ignore c:\program files (x86)\microsoft visual studio 12.0\vc\include\thr\xthread(196): warning C4702: unreachable code
#pragma warning( disable : 4702)
#include <thread>

#include "dullahan_platform_utils.h"

// GetParentProcess() is provided by dullahan_platform_utils.h (included above).
// It used to be duplicated here, which made the call in WinMain() ambiguous
// against the header's version, so the local copy has been removed.

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
#endif


#if defined(NO_STACK_PROTECTOR)
NO_STACK_PROTECTOR
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

    // Important: Create the CefApp instance in the main function to ensure it is available
    // to CefExecuteProcess() when the sub-process is launched. Creating the CefApp instance
    const CefRefPtr<MyApp> app = new MyApp();

    return CefExecuteProcess(args, app, nullptr);
}
#endif

// OS X Helper executable, we can probably share this between Win & Mac
#ifdef __APPLE__
#include "include/wrapper/cef_library_loader.h"

// Entry point function for sub-processes.
#if defined(NO_STACK_PROTECTOR)
NO_STACK_PROTECTOR
#endif
int main(int argc, char* argv[])
{
    CefScopedLibraryLoader library_loader;
    if (!library_loader.LoadInHelper())
    {
        return 1;
    }

    // Provide CEF with command-line arguments.
    CefMainArgs args(argc, argv);

    // Important: same as Windows, CefApp instance should be created in the main function
    const CefRefPtr<MyApp> app = new MyApp();

    // Execute the sub-process.
    return CefExecuteProcess(args, app, nullptr);
}
#endif
