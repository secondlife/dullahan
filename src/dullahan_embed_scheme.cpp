/*
    @brief Dullahan - a headless browser rendering engine
           based around the Chromium Embedded Framework

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

// embed:// custom scheme handler implementation

#define NOMINMAX

#include "dullahan_embed_scheme.h"
#include "dullahan_impl.h"

#include "cef_parser.h"
#include "cef_stream.h"
#include "wrapper/cef_stream_resource_handler.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    // MIME map for the file types the embed scheme uses.
    std::string mime_for_path(const std::string& path)
    {
        auto dot = path.find_last_of('.');
        std::string ext = (dot == std::string::npos) ? std::string() : path.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(),
                       [](char c) { return static_cast<char>(tolower(c)); });

        if (ext == "html" || ext == "htm") return "text/html";
        if (ext == "js")                   return "application/javascript";
        if (ext == "css")                  return "text/css";
        if (ext == "json")                 return "application/json";
        if (ext == "svg")                  return "image/svg+xml";
        if (ext == "png")                  return "image/png";
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "gif")                  return "image/gif";
        return "application/octet-stream";
    }

    // Strip query string / fragment and any leading '/'; also normalise backslashes.
    std::string canonical_relpath(const std::string& host, const std::string& path)
    {
        std::string p = path;
        auto q = p.find_first_of("?#");
        if (q != std::string::npos) p.erase(q);
        while (!p.empty() && p.front() == '/') p.erase(0, 1);
        std::replace(p.begin(), p.end(), '\\', '/');
        std::string relpath = host;
        if (!relpath.empty() && !p.empty()) relpath.push_back('/');
        relpath += p;
        return relpath;
    }

    // Reject any relpath containing "..", empty segments, or non-relative form.
    bool relpath_is_safe(const std::string& relpath)
    {
        if (relpath.empty()) return false;
        if (relpath.front() == '/') return false;
        if (relpath.find("..") != std::string::npos) return false;
        return true;
    }

    // In-memory CefReadHandler backed by a std::string. Used for the short
    // canned bodies of error responses so we can serve them through the same
    // CefStreamResourceHandler that we use for real file bodies.
    class string_read_handler : public CefReadHandler
    {
        public:
            explicit string_read_handler(std::string body) :
                mData(std::move(body)),
                mOffset(0)
            {
            }

            size_t Read(void* ptr, size_t elem_size, size_t n) override
            {
                size_t bytes_wanted = elem_size * n;
                size_t remaining = mData.size() - mOffset;
                size_t to_copy = std::min(bytes_wanted, remaining);
                if (to_copy > 0)
                {
                    std::memcpy(ptr, mData.data() + mOffset, to_copy);
                    mOffset += to_copy;
                }
                return elem_size ? (to_copy / elem_size) : 0;
            }

            int Seek(int64_t /*offset*/, int /*whence*/) override { return -1; }
            int64_t Tell() override { return static_cast<int64_t>(mOffset); }
            int Eof() override { return mOffset >= mData.size() ? 1 : 0; }
            bool MayBlock() override { return false; }

        private:
            std::string mData;
            size_t mOffset;

            IMPLEMENT_REFCOUNTING(string_read_handler);
            DISALLOW_COPY_AND_ASSIGN(string_read_handler);
    };

    CefResponse::HeaderMap default_headers()
    {
        CefResponse::HeaderMap headers;
        headers.insert(std::make_pair("Cache-Control", "no-store"));
        headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));
        return headers;
    }

    CefRefPtr<CefResourceHandler> make_error(int status, const std::string& reason)
    {
        std::string body = "embed:// " + reason;
        CefRefPtr<CefStreamReader> stream =
            CefStreamReader::CreateForHandler(new string_read_handler(std::move(body)));
        return new CefStreamResourceHandler(status, reason, "text/plain", default_headers(), stream);
    }
}

dullahan_embed_scheme_factory::dullahan_embed_scheme_factory(dullahan_impl* parent) :
    mParent(parent)
{
}

CefRefPtr<CefResourceHandler> dullahan_embed_scheme_factory::Create(
    CefRefPtr<CefBrowser> /*browser*/,
    CefRefPtr<CefFrame> /*frame*/,
    const CefString& /*scheme_name*/,
    CefRefPtr<CefRequest> request)
{
    if (!mParent)
    {
        return make_error(500, "no host");
    }

    const std::string& root = mParent->getEmbedSchemeRoot();
    const std::vector<std::string>& registry = mParent->getEmbedRegistry();
    if (root.empty())
    {
        return make_error(500, "embed root not configured");
    }

    // Parse the URL into pieces.
    CefURLParts parts;
    if (!CefParseURL(request->GetURL(), parts))
    {
        return make_error(400, "bad url");
    }
    std::string host = CefString(&parts.host).ToString();
    std::string path = CefString(&parts.path).ToString();
    std::transform(host.begin(), host.end(), host.begin(),
                   [](char c) { return static_cast<char>(tolower(c)); });

    std::string relpath = canonical_relpath(host, path);
    if (!relpath_is_safe(relpath))
    {
        return make_error(400, "invalid path");
    }

    // Registry lookup, relative paths use forward slashes.
    if (std::find(registry.begin(), registry.end(), relpath) == registry.end())
    {
        return make_error(404, "not in registry");
    }

    // Compose on-disk path.
    std::string disk_path = root;
    if (!disk_path.empty() &&
        disk_path.back() != '/' &&
        disk_path.back() != '\\')
    {
        disk_path.push_back('/');
    }
    disk_path += relpath;

    // Delegate file I/O to CEF. CefStreamReader::CreateForFile takes a
    // CefString (UTF-16 internally), so any UTF-8 path works correctly on
    // Windows - unlike std::ifstream, which silently fails on UTF-8 paths.
    CefRefPtr<CefStreamReader> stream = CefStreamReader::CreateForFile(disk_path);
    if (!stream)
    {
        return make_error(404, "file not found");
    }

    return new CefStreamResourceHandler(200, "OK", mime_for_path(relpath), default_headers(), stream);
}
