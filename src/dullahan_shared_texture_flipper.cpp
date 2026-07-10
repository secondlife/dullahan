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

#ifdef WIN32

#include "dullahan_shared_texture_flipper.h"

#include "dullahan_debug.h"

#include <cstring>

#include <dxgi1_2.h>
#include <d3dcompiler.h>

// Key used for the destination keyed mutex. Producer (this class) and consumer
// both acquire/release key 0 - a single key is enough for a strict
// produce-then-consume handoff of one frame.
static const UINT64 DEST_MUTEX_KEY = 0;

// How long the consumer-facing side is willing to wait on the source keyed
// mutex before giving up on this frame (matches the value the sample consumer
// uses on its side).
static const DWORD SOURCE_MUTEX_TIMEOUT_MS = 16;

// Fullscreen-triangle vertex shader. Generates three vertices from the vertex
// id (no vertex/index buffer needed) that cover the whole render target, and
// emits texture coordinates whose V axis is inverted so the sampled image lands
// upside-down in the destination.
static const char* kFlipVertexShader =
    "struct VSOut { float4 pos : SV_Position; float2 uv : TEXCOORD0; };       \n"
    "VSOut main(uint id : SV_VertexID)                                        \n"
    "{                                                                        \n"
    "    VSOut o;                                                             \n"
    "    float2 uv = float2((id << 1) & 2, id & 2);                          \n"
    "    o.pos = float4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, 0.0, 1.0);       \n"
    "    o.uv = float2(uv.x, 1.0 - uv.y);   /* flip vertically */            \n"
    "    return o;                                                            \n"
    "}                                                                        \n";

static const char* kFlipPixelShader =
    "Texture2D tex : register(t0);                                           \n"
    "SamplerState smp : register(s0);                                        \n"
    "float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
    "{                                                                        \n"
    "    return tex.Sample(smp, uv);                                         \n"
    "}                                                                        \n";

dullahan_shared_texture_flipper::dullahan_shared_texture_flipper() :
    mDestSharedHandle(nullptr),
    mDestWidth(0),
    mDestHeight(0),
    mDestFormat(DXGI_FORMAT_UNKNOWN),
    mInitialized(false)
{
}

dullahan_shared_texture_flipper::~dullahan_shared_texture_flipper()
{
    releaseDestination();
}

bool dullahan_shared_texture_flipper::init(bool use_luid, uint32_t luid_low, int32_t luid_high)
{
    if (mInitialized)
    {
        return true;
    }

    // If we were told which adapter CEF is using, create our device on the same
    // one so the shared texture can be opened directly (no cross-adapter copy).
    ComPtr<IDXGIAdapter> adapter;
    if (use_luid)
    {
        ComPtr<IDXGIFactory1> factory;
        if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)factory.GetAddressOf())))
        {
            ComPtr<IDXGIAdapter1> candidate;
            for (UINT i = 0; factory->EnumAdapters1(i, candidate.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i)
            {
                DXGI_ADAPTER_DESC1 desc;
                if (SUCCEEDED(candidate->GetDesc1(&desc)) &&
                    desc.AdapterLuid.LowPart == luid_low &&
                    desc.AdapterLuid.HighPart == luid_high)
                {
                    candidate.As(&adapter);
                    break;
                }
            }
        }
    }

    // When an explicit adapter is supplied the driver type MUST be UNKNOWN.
    const D3D_DRIVER_TYPE driver_type = adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    HRESULT hr = D3D11CreateDevice(adapter.Get(), driver_type, nullptr,
                                   D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                   nullptr, 0, D3D11_SDK_VERSION,
                                   device.GetAddressOf(), nullptr, context.GetAddressOf());
    if (FAILED(hr))
    {
        DLNOUT("shared texture flipper: D3D11CreateDevice failed 0x" << std::hex << hr);
        return false;
    }

    // We need ID3D11Device1 for OpenSharedResource1 (NT-handle shared textures).
    if (FAILED(device.As(&mDevice)))
    {
        DLNOUT("shared texture flipper: ID3D11Device1 not available");
        return false;
    }
    mContext = context;

    // Compile the flip pipeline once.
    ComPtr<ID3DBlob> vs_blob, ps_blob, errors;
    hr = D3DCompile(kFlipVertexShader, strlen(kFlipVertexShader), nullptr, nullptr, nullptr,
                    "main", "vs_4_0", 0, 0, vs_blob.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(hr))
    {
        DLNOUT("shared texture flipper: vertex shader compile failed");
        return false;
    }

    hr = D3DCompile(kFlipPixelShader, strlen(kFlipPixelShader), nullptr, nullptr, nullptr,
                    "main", "ps_4_0", 0, 0, ps_blob.ReleaseAndGetAddressOf(), errors.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        DLNOUT("shared texture flipper: pixel shader compile failed");
        return false;
    }

    if (FAILED(mDevice->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
                                           nullptr, mVertexShader.GetAddressOf())) ||
        FAILED(mDevice->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(),
                                          nullptr, mPixelShader.GetAddressOf())))
    {
        DLNOUT("shared texture flipper: shader creation failed");
        return false;
    }

    D3D11_SAMPLER_DESC samp = {};
    samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samp.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(mDevice->CreateSamplerState(&samp, mSampler.GetAddressOf())))
    {
        DLNOUT("shared texture flipper: sampler creation failed");
        return false;
    }

    mInitialized = true;
    return true;
}

void dullahan_shared_texture_flipper::releaseDestination()
{
    if (mDestSharedHandle)
    {
        ::CloseHandle(mDestSharedHandle);
        mDestSharedHandle = nullptr;
    }
    mDestMutex.Reset();
    mDestRTV.Reset();
    mDestTexture.Reset();
    mDestWidth = 0;
    mDestHeight = 0;
    mDestFormat = DXGI_FORMAT_UNKNOWN;
}

bool dullahan_shared_texture_flipper::ensureDestination(uint32_t width, uint32_t height, DXGI_FORMAT format)
{
    // Reuse the existing destination if nothing about the surface changed.
    if (mDestTexture && width == mDestWidth && height == mDestHeight && format == mDestFormat)
    {
        return true;
    }

    releaseDestination();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    // NT handle so the consumer can DuplicateHandle/OpenSharedResource1; keyed
    // mutex so producer and consumer serialize access to the one frame.
    desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

    if (FAILED(mDevice->CreateTexture2D(&desc, nullptr, mDestTexture.GetAddressOf())))
    {
        DLNOUT("shared texture flipper: destination CreateTexture2D failed");
        return false;
    }

    if (FAILED(mDevice->CreateRenderTargetView(mDestTexture.Get(), nullptr, mDestRTV.GetAddressOf())))
    {
        DLNOUT("shared texture flipper: destination RTV creation failed");
        releaseDestination();
        return false;
    }

    if (FAILED(mDestTexture.As(&mDestMutex)))
    {
        DLNOUT("shared texture flipper: destination keyed mutex query failed");
        releaseDestination();
        return false;
    }

    ComPtr<IDXGIResource1> dxgi_res;
    if (FAILED(mDestTexture.As(&dxgi_res)) ||
        FAILED(dxgi_res->CreateSharedHandle(nullptr,
                                            DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
                                            nullptr, &mDestSharedHandle)))
    {
        DLNOUT("shared texture flipper: destination CreateSharedHandle failed");
        releaseDestination();
        return false;
    }

    mDestWidth = width;
    mDestHeight = height;
    mDestFormat = format;
    return true;
}

void* dullahan_shared_texture_flipper::flip(void* cef_handle)
{
    if (!mInitialized || !cef_handle)
    {
        return nullptr;
    }

    // Open CEF's shared texture. It is consumed synchronously here, within the
    // OnAcceleratedPaint callback, so we do not need to duplicate the handle.
    ComPtr<ID3D11Texture2D> source;
    if (FAILED(mDevice->OpenSharedResource1((HANDLE)cef_handle, __uuidof(ID3D11Texture2D),
                                            (void**)source.GetAddressOf())))
    {
        // Transient - leave the device/pipeline intact and try again next frame.
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC src_desc;
    source->GetDesc(&src_desc);

    if (!ensureDestination(src_desc.Width, src_desc.Height, src_desc.Format))
    {
        return nullptr;
    }

    ComPtr<ID3D11ShaderResourceView> srv;
    if (FAILED(mDevice->CreateShaderResourceView(source.Get(), nullptr, srv.GetAddressOf())))
    {
        return nullptr;
    }

    // CEF documents its texture as having no keyed mutex, but older/other
    // configurations may still provide one - acquire it if it is there.
    ComPtr<IDXGIKeyedMutex> src_mutex;
    bool src_locked = false;
    if (SUCCEEDED(source.As(&src_mutex)))
    {
        if (SUCCEEDED(src_mutex->AcquireSync(DEST_MUTEX_KEY, SOURCE_MUTEX_TIMEOUT_MS)))
        {
            src_locked = true;
        }
        else
        {
            // Could not get the frame in time - skip it, keep everything alive.
            return nullptr;
        }
    }

    // Take the destination for writing.
    if (FAILED(mDestMutex->AcquireSync(DEST_MUTEX_KEY, SOURCE_MUTEX_TIMEOUT_MS)))
    {
        if (src_locked)
        {
            src_mutex->ReleaseSync(DEST_MUTEX_KEY);
        }
        return nullptr;
    }

    // Draw the source into the destination, flipped in Y by the vertex shader.
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)mDestWidth;
    vp.Height = (float)mDestHeight;
    vp.MaxDepth = 1.0f;

    ID3D11RenderTargetView* rtv = mDestRTV.Get();
    ID3D11ShaderResourceView* srv_raw = srv.Get();
    ID3D11SamplerState* samp_raw = mSampler.Get();

    mContext->OMSetRenderTargets(1, &rtv, nullptr);
    mContext->RSSetViewports(1, &vp);
    mContext->IASetInputLayout(nullptr);
    mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    mContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
    mContext->PSSetShader(mPixelShader.Get(), nullptr, 0);
    mContext->PSSetShaderResources(0, 1, &srv_raw);
    mContext->PSSetSamplers(0, 1, &samp_raw);
    mContext->Draw(3, 0);

    // Unbind the source SRV before we release the mutexes so the runtime does
    // not hold a reference to a resource we are about to hand back.
    ID3D11ShaderResourceView* null_srv = nullptr;
    mContext->PSSetShaderResources(0, 1, &null_srv);
    ID3D11RenderTargetView* null_rtv = nullptr;
    mContext->OMSetRenderTargets(1, &null_rtv, nullptr);

    mContext->Flush();

    mDestMutex->ReleaseSync(DEST_MUTEX_KEY);
    if (src_locked)
    {
        src_mutex->ReleaseSync(DEST_MUTEX_KEY);
    }

    return mDestSharedHandle;
}

#endif // WIN32
