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

#ifndef _DULLAHAN
#define _DULLAHAN

#define NOMINMAX

#include <memory>
#include <string>
#include <functional>
#include <vector>

class dullahan_impl;

class dullahan
{
    public:
        ////////// keyboard constants //////////
        typedef enum e_key_event
        {
            KE_KEY_DOWN,
            KE_KEY_REPEAT,
            KE_KEY_UP,
            KE_KEY_CHAR,
        } EKeyEvent;

        typedef enum e_keyboard_modifier
        {
            KM_MODIFIER_NONE = 0x00,
            KM_MODIFIER_SHIFT = 0x01,
            KM_MODIFIER_CONTROL = 0x02,
            KM_MODIFIER_ALT = 0x04,
            KM_MODIFIER_META = 0x08
        } EKeyboardModifier;

        ////////// mouse constants //////////
        typedef enum e_mouse_event
        {
            ME_MOUSE_MOVE,
            ME_MOUSE_DOWN,
            ME_MOUSE_UP,
            ME_MOUSE_DOUBLE_CLICK
        } EMouseEvent;

        typedef enum e_mouse_button
        {
            MB_MOUSE_BUTTON_LEFT,
            MB_MOUSE_BUTTON_RIGHT,
            MB_MOUSE_BUTTON_MIDDLE
        } EMouseButton;

        ////////// cursor type //////////
        typedef enum e_cursor_type
        {
            CT_POINTER = 0,
            CT_CROSS,
            CT_HAND,
            CT_IBEAM,
            CT_WAIT,
            CT_HELP,
            CT_EASTRESIZE,
            CT_NORTHRESIZE,
            CT_NORTHEASTRESIZE,
            CT_NORTHWESTRESIZE,
            CT_SOUTHRESIZE,
            CT_SOUTHEASTRESIZE,
            CT_SOUTHWESTRESIZE,
            CT_WESTRESIZE,
            CT_NORTHSOUTHRESIZE,
            CT_EASTWESTRESIZE,
            CT_NORTHEASTSOUTHWESTRESIZE,
            CT_NORTHWESTSOUTHEASTRESIZE,
            CT_COLUMNRESIZE,
            CT_ROWRESIZE,
            CT_MIDDLEPANNING,
            CT_EASTPANNING,
            CT_NORTHPANNING,
            CT_NORTHEASTPANNING,
            CT_NORTHWESTPANNING,
            CT_SOUTHPANNING,
            CT_SOUTHEASTPANNING,
            CT_SOUTHWESTPANNING,
            CT_WESTPANNING,
            CT_MOVE,
            CT_VERTICALTEXT,
            CT_CELL,
            CT_CONTEXTMENU,
            CT_ALIAS,
            CT_PROGRESS,
            CT_NODROP,
            CT_COPY,
            CT_NONE,
            CT_NOTALLOWED,
            CT_ZOOMIN,
            CT_ZOOMOUT,
            CT_GRAB,
            CT_GRABBING,
            CT_CUSTOM,
        } ECursorType;

        typedef enum e_file_dialog
        {
            FD_UNKNOWN,
            FD_OPEN_FILE,
            FD_OPEN_FOLDER,
            FD_OPEN_MULTIPLE_FILES,
            FD_SAVE_FILE,
        } EFileDialogType;

    public:
        //////////// initialization settings ////////////
        struct dullahan_settings
        {
            // initial dimensions of the browser window
            unsigned int initial_width = 512;
            unsigned int initial_height = 512;

            // host process name (for Windows - read only for API consumers)
            const std::string host_process_filename = "dullahan_host.exe";

            // host process path (Not required for macOS)
            std::string host_process_path = std::string();

            // substring inserted into existing user agent string
            // leave it blank by default otherwise "Chrome xx.x" part is removed
            std::string user_agent_substring = std::string();

            // default frame rate
            int frame_rate = 60;

            // enable/disable features - most obvious but listed for completeness
            bool begin_frame_scheduling = false;        // fixes issue when onPaint not called
            bool cache_enabled = true;                  // local cache
            bool cookies_enabled = true;                // cookies
            bool disable_gpu = true;                    // disable GPU and GPU compositing
            bool file_access_from_file_urls = false;    // allow access files from local file system
            bool disable_web_security = false;          // like adding --disable-web-security to Chrome command line
            bool disable_network_service = false;       // like adding --disable-features=NetworkService to Chrome command line
            bool use_mock_keychain = false;             // like adding --use-mock-keychain to Chrome command line
            bool autoplay_without_gesture = false;      // like adding --autoplay-policy=???? to Chrome command line
            bool fake_ui_for_media_stream = false;      // like adding --fake-ui-for-media-stream to Chrome command line
            bool flash_enabled = true;                  // system flash plugin
            bool force_wave_audio = false;              // forces Windows WaveOut/In audio
            bool image_shrink_standalone_to_fit = true; // scale standalone images larger than browser size to fit
            bool java_enabled = false;                  // java
            bool javascript_enabled = true;             // javascript
            bool media_stream_enabled = false;          // web cams etc. (caution)
            bool plugins_enabled = true;                // all plugins
            bool webgl_enabled = true;                  // webgl

            // explicitly set the path to the locales folder since defaults no longer work on some systems
            std::string locales_dir_path = std::string();

            // The root directory that all cache_path and context_cache_path values
            // must have in common.
            // If this value is empty and cache_path is non-empty then this value
            // will default to the cache_path value.
            std::string root_cache_path = std::string();

            // path to browser cache - cookies (if enabled) are also stored here as of Chrome 75
            // This will be used for global context
            std::string cache_path = std::string();

            // As of version 75 cef doesn't allow storing cookies separately from cache, but context
            // requests with individual cache path can be used to separate cookies.
            // Context's cache always should be a child to root cache path, simultaneous contexts with
            // same path do not share sessions.
            std::string context_cache_path = std::string();

            // list of language locale codes used to configure the Accept-Language HTTP header value
            // and change the default language of the browser
            std::string accept_language_list = "en-us";

            // host name:port to use as a web proxy
            std::string proxy_host_port = std::string();

            // background color displayed before first page loaded (RRGGBB)
            unsigned int background_color = 0xffffff;

            // flip pixel buffer in Y direction
            bool flip_pixels_y = false;

            // flip mouse input in Y direction
            bool flip_mouse_y = false;

            // location, name of CEF log file
            std::string log_file = "cef_log.txt";

            // whether to log verbosely (true) or not (false)
            int log_verbose = false;

            // allow Chrome (or other CEF windoW) to debug via http://localhost::PORT_NUMBER
            int remote_debugging_port = 1964;
        };

    public:
        //////////// the API itself ////////////
        dullahan();
        ~dullahan();

        // initialize everything - call before anything else
        bool init(dullahan_settings& user_settings);

        // close down CEF - call just before you exit
        void shutdown();

        // indicate to CEF you want to exit - after you call this,
        // wait for onRequestExit() callback before calling shutdown()
        void requestExit();

        // accessors for size of virtual window
        void getSize(int& width, int& height);
        void setSize(int width, int height);
        int getDepth();

        // run CEF in it's own message loop - doesn't exit until requestExit()
        // and shutdown() calls triggered
        // Note: complimentary to update();
        void run();

        // do some work in CEF - call regularly in your own message loop
        // Note: complimentary to run();
        void update();

        // transport control
        bool canGoBack();
        void goBack();
        bool canGoForward();
        void goForward();
        bool isLoading();
        void reload(const bool ignore_cache);
        void stop();

        // versions of CEF, Chrome and this library and one with everything
        const std::string dullahan_cef_version(bool show_bitwidth);
        const std::string dullahan_chrome_version(bool show_bitwidth);
        const std::string dullahan_version(bool show_bitwidth);
        const std::string composite_version();

        // returns a user agent string based off of passed in string that
        // is "more" compatible with sites that look for a specific string
        std::string makeCompatibleUserAgentString(const std::string base);

        // mouse input
        void mouseButton(EMouseButton mouse_button,
                         EMouseEvent mouse_event,
                         int x, int y);
        void mouseMove(int x, int y);
        void mouseWheel(int x, int y, int delta_x, int delta_y);

        // keyboard input
#ifndef __linux__
        void nativeKeyboardEventWin(uint32_t msg, uint32_t wparam, uint64_t lparam);
        void nativeKeyboardEventOSX(void* event);
        void nativeKeyboardEventOSX(dullahan::EKeyEvent event_type, uint32_t event_modifiers, uint32_t event_keycode,
                                    uint32_t event_chars, uint32_t event_umodchars, bool event_isrepeat);
#else
        void nativeKeyboardEvent(dullahan::EKeyEvent key_event, uint32_t native_scan_code, uint32_t native_virtual_key, uint32_t native_modifiers);
        void nativeKeyboardEventSDL2(dullahan::EKeyEvent key_event, uint32_t key_data, uint32_t key_modifiers, bool keypad_input);
#endif

        // navigate to a URL
        void navigate(const std::string url);

        // give focus to virtual browser window
        void setFocus();

        // set the page zoom
        void setPageZoom(const double zoom_val);

        // indicates if there is something available to be copy/cut/pasted
        // (for UI purposes) and if so, provides methods to do so
        bool editCanCopy();
        bool editCanCut();
        bool editCanPaste();
        void editCopy();
        void editCut();
        void editPaste();

        // show/hide the dev tools
        void showDevTools();
        void closeDevTools();

        // print page to PDF
        void printToPDF(const std::string path);

        // cookies
        bool setCookie(const std::string url,
                       const std::string name, const std::string value,
                       const std::string domain, const std::string path,
                       bool httponly, bool secure);
        const std::vector<std::string> getCookies();
        void deleteAllCookies();

        // POST data to a URL
        void postData(const std::string url,
                      const std::string data,
                      const std::string headers);

        // javascript
        bool executeJavaScript(const std::string cmd);

        // display a message page in the browser - e.g. URL cannot be loaded
        void showBrowserMessage(const std::string msg);

        // set/gate the schemes to intercept, halt browsing and trigger callback
        void setCustomSchemes(std::vector<std::string> custom_schemes);
        std::vector<std::string>& getCustomSchemes();

        //////////// callback setters ////////////
        // URL changes - e.g. redirect
        void setOnAddressChangeCallback(std::function<void(const std::string url)> callback);

        // message appears in the JavaScript console
        void setOnConsoleMessageCallback(std::function<void(const std::string message,
                                         const std::string source, int line)> callback);

        // cursor changes - e.g. as passed over hyperlink or entered text field
        void setOnCursorChangedCallback(std::function<void(const ECursorType type)> callback);

        // custom URL scheme link is clicked (see setCustomSchemes(..))
        void setOnCustomSchemeURLCallback(std::function<void(const std::string url,
                                          bool user_gesture, 
                                          bool is_redirect)> callback);

        // HTTP auth request triggered
        void setOnHTTPAuthCallback(std::function<bool(const std::string host,
                                   const std::string realm,
                                   std::string& username, std::string& password)> callback);

        // page finishes loading
        void setOnLoadEndCallback(std::function<void(int status, const std::string url)> callback);

        // page load error - e.g. 404
        void setOnLoadErrorCallback(std::function<void(int status, const std::string error_text, const std::string error_url)> callback);

        // page starts to load
        void setOnLoadStartCallback(std::function<void()> callback);

        // popup opened
        void setOnOpenPopupCallback(std::function<void(const std::string url,
                                    const std::string target)> callback);

        // contents of the pages changes
        void setOnPageChangedCallback(std::function<void(const unsigned char* pixels,
                                      int x, int y,
                                      int width, int height)> callback);

        // exit app requested
        void setOnRequestExitCallback(std::function<void()> callback);

        // browser status message changes
        void setOnStatusMessageCallback(std::function<void(const std::string message)> callback);

        // page title changes
        void setOnTitleChangeCallback(std::function<void(const std::string title)> callback);

        void setOnTooltipCallback(std::function<void(const std::string text)> callback);

        // a call to printToPDF completed
        void setOnPdfPrintFinishedCallback(std::function<void(const std::string path, bool ok)> callback);

        // file download progress
        void setOnFileDownloadProgressCallback(std::function<void(int percent, bool complete)> callback);

        // file picker shown
        void setOnFileDialogCallback(std::function<const std::vector<std::string>(dullahan::EFileDialogType dialog_type, const std::string dialog_title, const std::string default_file, const std::string dialog_accept_filter, bool& use_default)> callback);

        // JS dialog shown (alert)
        void setOnJSDialogCallback(std::function<bool(const std::string origin_url,
                                   const std::string message_text,
                                   const std::string default_prompt_text)> callback);

        // JS before unload callback (alert)
        void setOnJSBeforeUnloadCallback(std::function<bool()> callback);

    private:
        std::unique_ptr <dullahan_impl> mImpl;
};

#endif //  _DULLAHAN
