#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <pwd.h>
#include <string>
#include <fstream>
#include <dirent.h>
#include <iostream>

namespace {
    std::string getExeCwd()
    {
        char path[ 4096 ];
        int len = readlink("/proc/self/exe", path, sizeof(path));
        if (len == -1)
        {
            return "";
        }

        path[len] = 0;
        return dirname(path) ;
    }
}

void dullahan_impl::platormInitWidevine(std::string cachePath)
{
}

void dullahan_impl::platformAddCommandLines(CefRefPtr<CefCommandLine> command_line)
{
    auto *pDisplay = getenv("DISPLAY");
    auto *pSessionType = getenv("XDG_SESSION_TYPE");
    auto *pWaylandDisplay = getenv("WAYLAND_DISPAY");
    auto *pSDLVideoDriver = getenv("SDL_VIDEODRIVER");

    // XWayland disabled
    // pDisplay == nullptr;
    // pSessionType == "wayland"
    // pWaylandDispay == "wayland-%d"

    // XWayland enabled
    // pDisplay == ":%d"
    // pSessionType == "wayland"
    // pWaylandDispay == "wayland-%d"

    bool use_wayland = false;

    if (pDisplay == nullptr || strlen(pDisplay) == 0)
    {
        use_wayland = true;
    }

    if (pSDLVideoDriver && strcmp(pSDLVideoDriver, "wayland") == 0)
    {
        use_wayland = true;
    }

    std::cerr << "Using wayland: " << use_wayland << std::endl;

    if (use_wayland)
    {
        command_line->AppendSwitchWithValue("ozone-platform", "wayland");
    }
}

