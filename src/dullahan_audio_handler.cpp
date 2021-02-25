#include "wrapper/cef_helpers.h"

#include "dullahan_audio_handler.h"

#include "dullahan_impl.h"
#include "dullahan_callback_manager.h"

dullahan_audio_handler::dullahan_audio_handler(dullahan_impl* parent) :
    mParent(parent)
{
}

dullahan_audio_handler::~dullahan_audio_handler()
{
}

bool dullahan_audio_handler::GetAudioParameters(CefRefPtr<CefBrowser> browser, CefAudioParameters& params)
{
    params.channel_layout = CEF_CHANNEL_LAYOUT_STEREO;
    params.sample_rate = 48000;

    return true;
}

void dullahan_audio_handler::OnAudioStreamStarted(CefRefPtr<CefBrowser> browser, const CefAudioParameters& params, int channels)
{
    mParent->getCallbackManager()->onAudioStreamStarted(channels, params.sample_rate, params.frames_per_buffer);
}

void dullahan_audio_handler::OnAudioStreamPacket(CefRefPtr<CefBrowser> browser, const float** data, int frames, int64 pts)
{
    mParent->getCallbackManager()->onAudioStreamPacket(data, frames, pts);
    DLNOUT("Audio packet received: frames = " << frames)
}

void dullahan_audio_handler::OnAudioStreamStopped(CefRefPtr<CefBrowser> browser)
{
    mParent->getCallbackManager()->onAudioStreamStopped();
}

void dullahan_audio_handler::OnAudioStreamError(CefRefPtr<CefBrowser> browser, const CefString& message)
{
    mParent->getCallbackManager()->onAudioStreamError(message);
}