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

#include "dullahan_impl.h"
#include "dullahan_cookie_manager.h"

#include <atomic>

bool dullahan_cookie_manager::init()
{
    mCookieManager = nullptr;

    class getCookieManagerCallback :
        public CefCompletionCallback
    {
        public:
            getCookieManagerCallback(dullahan_cookie_manager* parent) :
                mParent(parent),
                mCookieManagerReady(false)
            {
            }

            void OnComplete() override
            {
                mCookieManagerReady = true;
            }

            std::atomic<bool> mCookieManagerReady;

        private:
            dullahan_cookie_manager* mParent;
            IMPLEMENT_REFCOUNTING(getCookieManagerCallback);
    };

    // A common pattern used in this source file.  These functions
    // are all asynchronous but we want to call them from a synchronous
    // API so we have to wait for an atomic to be set in their
    // completion functions. There might be a better way to do this.
    // Alternatively, we might reconstruct the Dullahan API to be
    // more asynchronous in nature, in which case we would not need
    // this.  This pattern is used in all of the cookie functions.
    getCookieManagerCallback* cb = new getCookieManagerCallback(this);
    mCookieManager = CefCookieManager::GetGlobalManager(cb);
    if (mCookieManager)
    {
        while (cb->mCookieManagerReady != true)
        {
            // While we are waiting, service CEF
            CefDoMessageLoopWork();
        }

        return true;
    }

    return false;
}

bool dullahan_cookie_manager::setCookie(const std::string url,
                                        const std::string name, const std::string value,
                                        const std::string domain, const std::string path,
                                        bool httponly, bool secure)
{
    if (mCookieManager == nullptr)
    {
        return false;
    }

    CefCookie cookie;
    CefString(&cookie.name) = name;
    CefString(&cookie.value) = value;
    CefString(&cookie.domain) = domain;
    CefString(&cookie.path) = path;
    cookie.httponly = httponly;
    cookie.secure = secure;

    cookie.has_expires = true;
    cookie.expires.year = 2100;
    cookie.expires.month = 1;
    cookie.expires.day_of_week = 1;
    cookie.expires.day_of_month = 1;

    class setCookieCallback :
        public CefSetCookieCallback
    {
        public:
            setCookieCallback(dullahan_cookie_manager* parent) :
                mParent(parent),
                mSetCookieComplete(false),
                mSetCookieResult(false)
            {
            }

            void OnComplete(bool success) override
            {
                mSetCookieResult = success;

                if (success)
                {
                    // TODO: If we pass true here to indicate that
                    // we want to wait for flushCookies() to complete
                    // all kinds of bad this happen - figure out why.
                    bool wait_for_complete = false;
                    mParent->flushCookies(wait_for_complete);
                }
            }
            std::atomic<bool> mSetCookieComplete;
            std::atomic<bool> mSetCookieResult;

        private:
            dullahan_cookie_manager* mParent;
            IMPLEMENT_REFCOUNTING(setCookieCallback);
    };

    setCookieCallback* cb = new setCookieCallback(this);
    bool result = mCookieManager->SetCookie(url, cookie, cb);
    if (result)
    {
        while (cb->mSetCookieComplete != true)
        {
            CefDoMessageLoopWork();
        }

        return cb->mSetCookieResult;
    }

    return false;
}

bool dullahan_cookie_manager::flushCookies(bool wait_for_complete)
{
    if (mCookieManager == nullptr)
    {
        return false;
    }

    class flushCookiesCallback :
        public CefCompletionCallback
    {
        public:
            flushCookiesCallback(dullahan_cookie_manager* parent) :
                mParent(parent),
                mFlushCookiesComplete(false)
            {
            }

            void OnComplete() override
            {
                mFlushCookiesComplete = true;
            }

            std::atomic<bool> mFlushCookiesComplete;

        private:
            dullahan_cookie_manager* mParent;
            IMPLEMENT_REFCOUNTING(flushCookiesCallback);
    };

    flushCookiesCallback* cb = nullptr;
    if (wait_for_complete)
    {
        cb = new flushCookiesCallback(this);
    }

    bool result = mCookieManager->FlushStore(cb);
    if (result)
    {
        if (cb)
        {
            while (cb->mFlushCookiesComplete != true)
            {
                CefDoMessageLoopWork();
            }
        }

        return true;
    }

    return false;
}

const dullahan::dullahan_cookie_list_t dullahan_cookie_manager::getCookies()
{
    dullahan::dullahan_cookie_list_t cookies;

    if (mCookieManager == nullptr)
    {
        return cookies;
    }

    class cookieVisitorCallback : public CefCookieVisitor
    {
        public:
            cookieVisitorCallback(dullahan_cookie_manager* parent, dullahan::dullahan_cookie_list_t& cookies) :
                mParent(parent),
                mCookies(cookies),
                mVisitCookiesComplete(false)
            {
            }

            bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) override
            {
                const std::string name = std::string(CefString(&cookie.name));
                const std::string value = std::string(CefString(&cookie.value));

                mCookies.emplace_back(name, value);

                if (count == total)
                {
                    mVisitCookiesComplete = true;
                }

                deleteCookie = false;
                const bool keep_visiting_cookies = true;
                return keep_visiting_cookies;
            }

            std::atomic<bool> mVisitCookiesComplete;

        private:
            dullahan_cookie_manager* mParent;
            dullahan::dullahan_cookie_list_t& mCookies;
            IMPLEMENT_REFCOUNTING(cookieVisitorCallback);
    };

    cookieVisitorCallback* cb = new cookieVisitorCallback(this, cookies);
    bool result = mCookieManager->VisitAllCookies(cb);
    if (result)
    {
        while (cb->mVisitCookiesComplete != true)
        {
            CefDoMessageLoopWork();
        }

        return cookies;
    }

    return  cookies;
}

bool dullahan_cookie_manager::deleteCookies()
{
    if (mCookieManager == nullptr)
    {
        return false;
    }

    class deleteCookiesCallback :
        public CefDeleteCookiesCallback
    {
        public:
            deleteCookiesCallback(dullahan_cookie_manager* parent) :
                mParent(parent),
                mDeleteCookiesComplete(false)
            {
            }

            void OnComplete(int num_deleted) override
            {
                mDeleteCookiesComplete = true;
            }

            std::atomic<bool> mDeleteCookiesComplete;

        private:
            dullahan_cookie_manager* mParent;
            IMPLEMENT_REFCOUNTING(deleteCookiesCallback);
    };

    const CefString url("");
    const CefString name("");

    deleteCookiesCallback* cb = new deleteCookiesCallback(this);
    bool result = mCookieManager->DeleteCookies(url, name, cb);
    if (result)
    {
        while (cb->mDeleteCookiesComplete != true)
        {
            CefDoMessageLoopWork();
        }

        return true;
    }

    return false;
}
