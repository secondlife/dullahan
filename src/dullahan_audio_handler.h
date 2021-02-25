#ifndef _DULLAHAN_AUDIO_HANDLER
#define _DULLAHAN_AUDIO_HANDLER

#include "cef_audio_handler.h"

class dullahan_impl;

class dullahan_audio_handler :
    public CefAudioHandler
{
    public:
        dullahan_audio_handler(dullahan_impl* parent);
        ~dullahan_audio_handler();

        // CefAudioHandler interface
        bool GetAudioParameters(CefRefPtr<CefBrowser> browser, CefAudioParameters& params) override;
        void OnAudioStreamStarted(CefRefPtr<CefBrowser> browser, const CefAudioParameters& params, int channels) override;
        void OnAudioStreamPacket(CefRefPtr<CefBrowser> browser, const float** data, int frames, int64 pts) override;
        void OnAudioStreamStopped(CefRefPtr<CefBrowser> browser) override;
        void OnAudioStreamError(CefRefPtr<CefBrowser> browser, const CefString& message) override;

        IMPLEMENT_REFCOUNTING(dullahan_audio_handler);

    private:

        dullahan_impl* mParent;
};

#endif // _DULLAHAN_AUDIO_HANDLER
