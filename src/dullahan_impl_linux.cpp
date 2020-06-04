#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <pwd.h>
#include <string>
#include <fstream>
#include <dirent.h>

namespace
{
    std::string getExeCwd()
    {
        char path[ 4096 ];
        int len = readlink("/proc/self/exe", path, sizeof(path));
        if (len != -1)
        {
            path[len] = 0;
            return dirname(path) ;
        }
        return "";
    }
}

void dullahan_impl::platormInitWidevine(std::string cachePath)
{
}

void dullahan_impl::platformAddCommandLines(CefRefPtr<CefCommandLine> command_line)
{
    // <ND> For Linux autodetection does not work, we need to pass ppapi-flash-path/version.
    // We're getting this via environment variables from either the parent process or the user
    std::string strPlugin, strVersion;

    if (getenv("FS_FLASH_PLUGIN") && getenv("FS_FLASH_VERSION"))
    {
        strPlugin = getenv("FS_FLASH_PLUGIN");
        strVersion = getenv("FS_FLASH_VERSION");
    }

    if (strPlugin.size() && strVersion.size() && mSystemFlashEnabled)
    {
        command_line->AppendSwitchWithValue("ppapi-flash-path", strPlugin);
        command_line->AppendSwitchWithValue("ppapi-flash-version", strVersion);
    }
}

