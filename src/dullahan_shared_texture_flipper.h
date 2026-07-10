/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework
    @author Roxie Linden 2026

    Copyright (c) 2026, Linden Research, Inc.

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

#ifndef _DULLAHAN_SHARED_TEXTURE_FLIPPER
#define _DULLAHAN_SHARED_TEXTURE_FLIPPER

// This helper only exists on Windows where accelerated paint hands us a
// D3D11 shared texture. On other platforms the accelerated handle is passed
// through unchanged (see dullahan_render_handler::OnAcceleratedPaint).
#ifdef WIN32

#include <cstdint>

#include <d3d11_1.h>
#include <wrl/client.h>

// Vertically flips the D3D11 shared texture that CEF emits from
// OnAcceleratedPaint and republishes the result as a new shared texture whose
// origin is bottom-left (matching what OpenGL-style consumers expect when
// flip_pixels_y is set).
//
// The GPU device, the flip pipeline (shaders + sampler) and the destination
// texture are created ONCE and reused for every frame. The destination is only
// reallocated when the incoming texture's width, height or format changes. A
// transient per-frame failure (e.g. a keyed-mutex timeout) returns null but
// leaves the device and pipeline intact so the next frame can try again.
class dullahan_shared_texture_flipper
{
    public:
        dullahan_shared_texture_flipper();
        ~dullahan_shared_texture_flipper();

        // Create the device and fixed pipeline. When use_luid is true the
        // device is created on the adapter identified by (luid_low, luid_high)
        // so it matches the adapter CEF renders on and the shared texture can
        // be opened without a cross-adapter copy. Returns false on failure.
        bool init(bool use_luid, uint32_t luid_low, int32_t luid_high);

        // Flip the CEF shared texture referenced by cef_handle and return a
        // shareable NT handle to the flipped copy, or nullptr on transient
        // failure. The returned handle is owned by this object and remains
        // valid until the next flip() that resizes the destination or until
        // this object is destroyed.
        void* flip(void* cef_handle);

    private:
        template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

        // (re)create the destination texture, its render target view and its
        // shared handle when the requested size/format differs from the cache.
        bool ensureDestination(uint32_t width, uint32_t height, DXGI_FORMAT format);
        void releaseDestination();

        ComPtr<ID3D11Device1> mDevice;
        ComPtr<ID3D11DeviceContext> mContext;
        ComPtr<ID3D11VertexShader> mVertexShader;
        ComPtr<ID3D11PixelShader> mPixelShader;
        ComPtr<ID3D11SamplerState> mSampler;

        // reused destination - owned by us and shared with the consumer
        ComPtr<ID3D11Texture2D> mDestTexture;
        ComPtr<ID3D11RenderTargetView> mDestRTV;
        ComPtr<IDXGIKeyedMutex> mDestMutex;
        HANDLE mDestSharedHandle;
        uint32_t mDestWidth;
        uint32_t mDestHeight;
        DXGI_FORMAT mDestFormat;

        bool mInitialized;
};

#endif // WIN32

#endif // _DULLAHAN_SHARED_TEXTURE_FLIPPER
