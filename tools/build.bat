@rem Batch file that makes use of a leaner CMakeLists.txt
@rem to build the CEF DLL wrapper library, the Dullahan
@rem library, the Dullahan host executable and the OpenGL
@rem example application.
@rem
@rem Either pass in a URL to a CEF package archive (via
@rem CEF_PACKAGE_URL) or a folder containing an uncompressed
@rem CEF distribution (CEF_PACKAGE_DIR) into the cmake command
@rem below.
@rem
@rem Note 1: CEF_PACKAGE_URL must be a tar.bz2 file unless
@rem you are using CMake 4.1+, in which case zst might work
@rem Note 2: CEF_PACKAGE_DIR path must use forward slashes
@rem vs backslashes - even on Windows...

@if not exist "tools/" (
    @echo Run this command from the project root directory
    @goto end
)

if exist build rmdir build /s /q
mkdir build
cd build

cmake^
 -G "Visual Studio 17 2022" -A x64 ..^
 -DCEF_RUNTIME_LIBRARY_FLAG=/MD^
 -DUSE_SANDBOX=Off^
 -DCEF_PACKAGE_URL=https://cef-builds.spotifycdn.com/cef_binary_139.0.37%%2Bgb457b0b%%2Bchromium-139.0.7258.139_windows64_minimal.tar.bz2

cmake --build . --target dullahan --config Release
cmake --build . --target dullahan_host --config Release
cmake --build . --target opengl-example --config Release

:end
