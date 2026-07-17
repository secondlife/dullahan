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
#include "cef_scheme.h"

#include <string>

namespace
{
    // CEF requires the same registration in every process before initialisation.
    // Must match dullahan_impl::OnRegisterCustomSchemes exactly
    const char* kEmbedScheme = "embed";
    const char* kEmbedSchemePrefix = "embed://";

    bool url_is_embed(const std::string& url)
    {
        static const std::string prefix(kEmbedSchemePrefix);
        if (url.size() < prefix.size()) return false;
        // Scheme comparison is case-insensitive.
        for (size_t i = 0; i < prefix.size(); ++i)
        {
            if (tolower(static_cast<unsigned char>(url[i])) != prefix[i]) return false;
        }
        return true;
    }
}

// Shared by Windows and Mac sub-process entry points
class JSONtoCPPHandler : public CefV8Handler
{
    public:
        JSONtoCPPHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) :
            // Save the browser + frame references from OnContextCreated()
            // for later use in IPC communication. The frame reference is used
            // so the browser process can identify which document originated
            // each message (for embed:// origin checks).
            mBrowser(browser),
            mFrame(frame)
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
            std::string frame_url = mFrame ? std::string(mFrame->GetURL()) : std::string();
            args->SetString(2, frame_url);
            mBrowser->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);

            // Acknowledge receipt of the JSON string
            return "{\"ACK\": true}";
        }

        CefRefPtr<CefBrowser> mBrowser;
        CefRefPtr<CefFrame> mFrame;
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

    // CefApp override. Runs in every process. Must register the embed://
    // scheme with the same flags as the browser process so Chromium's renderer
    // treats embed:// documents as a proper (secure) origin.
    void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override
    {
        registrar->AddCustomScheme(kEmbedScheme,
                                   CEF_SCHEME_OPTION_STANDARD |
                                   CEF_SCHEME_OPTION_SECURE |
                                   CEF_SCHEME_OPTION_CORS_ENABLED |
                                   CEF_SCHEME_OPTION_FETCH_ENABLED |
                                   CEF_SCHEME_OPTION_LOCAL);
    }

    void OnContextCreated(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override
    {
        // Only inject the window.JSONtoCPP bridge on trusted embed:// frames.
        std::string url = frame ? std::string(frame->GetURL()) : std::string();
        if (!url_is_embed(url))
        {
            return;
        }

        CefRefPtr<CefV8Value> global = context->GetGlobal();
        CefRefPtr<CefV8Handler> handler = new JSONtoCPPHandler(browser, frame);
        CefRefPtr<CefV8Value> func = CefV8Value::CreateFunction("JSONtoCPP", handler);
        global->SetValue("JSONtoCPP", func, V8_PROPERTY_ATTRIBUTE_NONE);

        // Friendly greeting to the browser process to confirm the context was created
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JSONtoCPP_MSG");
        CefRefPtr<CefListValue> args = msg->GetArgumentList();
        args->SetString(0, "INFO");
        args->SetString(1, "Hello from the OnContextCreated in the sub-process!");
        args->SetString(2, url);
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
    const CefRefPtr<MyApp> app = new MyApp();
    return CefExecuteProcess(main_args, app, nullptr);
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

    // Create the CefApp instance in the main function to ensure it is available
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
