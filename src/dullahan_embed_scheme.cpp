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

#include "dullahan_embed_scheme.h"
#include "dullahan_impl.h"

#include "cef_parser.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
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

    // Read an entire file into a byte vector;
    bool read_file_bytes(const std::string& disk_path, std::vector<unsigned char>& out)
    {
        std::ifstream in(disk_path, std::ios::binary);
        if (!in) return false;
        in.seekg(0, std::ios::end);
        std::streamsize size = in.tellg();
        if (size < 0) return false;
        in.seekg(0, std::ios::beg);
        out.resize(static_cast<size_t>(size));
        if (size > 0 && !in.read(reinterpret_cast<char*>(out.data()), size)) return false;
        return true;
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

    // Resource handler that serves a pre-loaded byte buffer with a given MIME type.
    // Uses CefStreamResourceHandler via an in-memory CefStreamReader, or a simple
    // hand-rolled CefResourceHandler if we need finer control.
    class embed_resource_handler : public CefResourceHandler
    {
        public:
            embed_resource_handler(std::vector<unsigned char>&& data,
                                   const std::string& mime,
                                   int status_code,
                                   const std::string& status_text) :
                mData(std::move(data)),
                mMime(mime),
                mStatus(status_code),
                mStatusText(status_text),
                mOffset(0)
            {
            }

            bool Open(CefRefPtr<CefRequest> request, bool& handle_request,
                      CefRefPtr<CefCallback> callback) override
            {
                handle_request = true;
                return true;
            }

            void GetResponseHeaders(CefRefPtr<CefResponse> response,
                                    int64_t& response_length,
                                    CefString& redirectUrl) override
            {
                response->SetMimeType(mMime);
                response->SetStatus(mStatus);
                if (!mStatusText.empty())
                {
                    response->SetStatusText(mStatusText);
                }
                CefResponse::HeaderMap headers;
                headers.insert(std::make_pair("Cache-Control", "no-store"));
                headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));
                response->SetHeaderMap(headers);
                response_length = static_cast<int64_t>(mData.size());
            }

            bool Read(void* data_out, int bytes_to_read, int& bytes_read,
                      CefRefPtr<CefResourceReadCallback> callback) override
            {
                bytes_read = 0;
                if (mOffset >= mData.size()) return false;
                size_t remaining = mData.size() - mOffset;
                int to_copy = static_cast<int>(std::min<size_t>(remaining,
                                                                 static_cast<size_t>(bytes_to_read)));
                std::memcpy(data_out, mData.data() + mOffset, to_copy);
                mOffset += to_copy;
                bytes_read = to_copy;
                return true;
            }

            void Cancel() override {}

        private:
            std::vector<unsigned char> mData;
            std::string mMime;
            int mStatus;
            std::string mStatusText;
            size_t mOffset;

            IMPLEMENT_REFCOUNTING(embed_resource_handler);
            DISALLOW_COPY_AND_ASSIGN(embed_resource_handler);
    };

    CefRefPtr<CefResourceHandler> make_error(int status, const std::string& reason)
    {
        std::string body = "embed:// " + reason;
        std::vector<unsigned char> bytes(body.begin(), body.end());
        return new embed_resource_handler(std::move(bytes), "text/plain", status, reason);
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

    std::vector<unsigned char> bytes;
    if (!read_file_bytes(disk_path, bytes))
    {
        return make_error(404, "file not found");
    }

    return new embed_resource_handler(std::move(bytes), mime_for_path(relpath), 200, "OK");
}
