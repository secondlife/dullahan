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

    bool testFileOrDir(std::string path, uint32_t mode)
    {
        if (path.empty())
        {
            return false;
        }

        struct stat st;

        if (!stat(path.c_str(), &st))
        {
            return (st.st_mode & mode) != 0;
        }

        return false;
    }

    bool isFile(std::string path)
    {
        return testFileOrDir(path, S_IFREG);
    }

    bool isDir(std::string path)
    {
        return testFileOrDir(path, S_IFDIR);
    }

    void copyFile(std::string source, std::string dest)
    {
        // std::filesystem would be neat here ...
        // sadly not available till pretty recent GCC versions.
        std::ifstream fSource(source, std::ios::binary);
        std::ofstream fDest(dest, std::ios::binary);

        fDest << fSource.rdbuf();
    }

    std::string strManifest = { R"(
    {
      "manifest_version": 2,
      "name": "WidevineCdm",
      "description": "Widevine Content Decryption Module",
      "version": "4.10.1679.0",
      "x-cdm-module-versions": "4",
      "x-cdm-interface-versions": "10",
      "x-cdm-host-versions": "10",
      "x-cdm-codecs": "vp8,vp09,avc1,av01",
      "x-cdm-persistent-license-support": false,
      "x-cdm-supported-encryption-schemes": ["cenc","cbcs"],
      "arch": "x64",
      "os": "linux"
     })" 
    };


    void appendSlash(std::string& dir)
    {
        if (dir.empty())
        {
            return;
        }

        if (dir[ dir.length() - 1 ] != '/')
        {
            dir += "/";
        }
    }

    std::string getHomedir()
    {
        char const* pHome = getenv("HOME");
        if (pHome == nullptr)
        {
            pHome = getpwuid(getuid())->pw_dir;
        }

        if (!pHome)
        {
            return "";
        }

        std::string home{ pHome };

        appendSlash(home);
        return home;
    }

    struct WidevineEntry
    {
        std::string path;
        std::string version;
    };

    std::string getFirefoxWidevine()
    {
        std::string pathWidevine;
        std::vector< WidevineEntry > foundVersions;
        std::string home { getHomedir() };

        if (home.empty())
        {
            return pathWidevine;
        }

        home += ".mozilla/firefox/";
        if (!isDir(home))
        {
            return pathWidevine;
        }

        DIR* pDir{ opendir(home.c_str()) };
        if (pDir)
        {
            while (dirent* pEntry = readdir(pDir))
            {
                std::string dirName { home };

                if (pEntry->d_name[ 0 ] == 0 ||
                    std::string(pEntry->d_name) == "." ||
                    std::string(pEntry->d_name) == "..")
                {
                    continue;
                }

                dirName += pEntry->d_name;
                if (!isDir(dirName))
                {
                    continue;
                }

                appendSlash(dirName);

                dirName += "gmp-widevinecdm/";
                if (!isDir(dirName))
                {
                    continue;
                }

                DIR* pSubdir{ opendir(dirName.c_str()) };
                if (pSubdir)
                {
                    while (dirent* pSubentry = readdir(pSubdir))
                    {
                        std::string subDir { dirName };
                        if (pEntry->d_name[ 0 ] == 0 ||
                            std::string(pEntry->d_name) == "." ||
                            std::string(pEntry->d_name) == "..")
                        {
                            continue;
                        }

                        subDir += pSubentry->d_name;
                        if (!isDir(subDir))
                        {
                            continue;
                        }

                        appendSlash(subDir);
                        if (isFile(subDir + "manifest.json") &&
                            isFile(subDir + "libwidevinecdm.so"))
                            foundVersions.push_back({ subDir, pSubentry->d_name });
                    }
                    closedir(pSubdir);
                }
            }

            closedir(pDir);
        }

        // Todo? check versions if multiple are found?

        if (foundVersions.size())
        {
            pathWidevine = foundVersions[0].path;
        }

        return pathWidevine;
    }

    std::string getChromeWidevine(std::string cachePath)
    {
        std::string strGooglePath {"/opt/google/chrome/WidevineCdm/" };
        if (!isDir(strGooglePath))
        {
            return "";
        }

        if (!isFile(strGooglePath + "_platform_specific/linux_x64/libwidevinecdm.so"))
        {
            return "";
        }

        if (!isDir(cachePath))
        {
            return "";
        }

        if (cachePath[ cachePath.length() - 1 ] != '/')
        {
            cachePath += "/widevine/";
        }
        else
        {
            cachePath += "widevine/";
        }

        if (!isDir(cachePath))
        {
            mkdir(cachePath.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }

        copyFile(strGooglePath + "_platform_specific/linux_x64/libwidevinecdm.so", cachePath + "libwidevinecdm.so");
        std::ofstream fManifest(cachePath + "manifest.json", std::ios::binary);
        fManifest << strManifest;

        return cachePath;

    }

}

void dullahan_impl::platormInitWidevine(std::string cachePath)
{
    std::string strWDVPath { getFirefoxWidevine() };
    if (strWDVPath.size())
    {
        CefRegisterWidevineCdm(strWDVPath.c_str(), nullptr);
        return;
    }

    strWDVPath = getChromeWidevine(cachePath);
    if (strWDVPath.size())
    {
        CefRegisterWidevineCdm(strWDVPath.c_str(), nullptr);
    }
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

