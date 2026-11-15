/*
Rodent, a UCI chess playing engine derived from Sungorus 1.4
Copyright (C) 2009-2011 Pablo Vazquez (Sungorus author)
Copyright (C) 2020-2020 Pawel Koziol
Copyright (C) 2020-2020 Bernhard C. Maerz

Rodent is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Rodent is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else    
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#if defined(__APPLE__)
    #include <mach-o/dyld.h>
#endif
#include <cerrno>
#include <cstdlib>
#include <vector>

#include "rodent.h"

// --------------------------------------------------------------------------------
// GetEnv
// --------------------------------------------------------------------------------

std::string GetEnv(const char *envName) {
    std::string retStr;

    // Default value (we don't need difference between 'empty' and 'unset')
    retStr = "";

#if defined(_WIN32) || defined(_WIN64)
    // Simple variant is enough - without autoincrease the limit
    const DWORD buff_size = 255;
    
    wchar_t buff[buff_size];
    const DWORD var_size = GetEnvironmentVariableW(Str2WStr(envName).c_str(), buff, buff_size);

    if (var_size && var_size <= buff_size)
        retStr = WCStr2Str(buff);
#else
    char *envValue;
    envValue = getenv(envName);
    if (envValue)
        retStr = envValue;
#endif
    
    return retStr;
}

// --------------------------------------------------------------------------------
// DirOrFileExists
// --------------------------------------------------------------------------------

bool DirOrFileExists(const char* path) {

#if defined(_WIN32) || defined(_WIN64)
    DWORD ftyp;
    
    ftyp = GetFileAttributesA(path);
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false; //something is wrong with your path!
    // if need any "DirExists", use:
    //     return (ftyp & FILE_ATTRIBUTE_DIRECTORY);
	
#else
    struct stat info;

    if (stat( path, &info ))
        return false; //something is wrong with your path!
    // if need any "DirExists", use:
    //     return ( info.st_mode & S_IFDIR );
	
#endif
    
    return true;
}

// --------------------------------------------------------------------------------
// PathEndSlash
// --------------------------------------------------------------------------------
// to ensure that path ends with a slash

void PathEndSlash(std::wstring &pathWStr) {

    std::wstring slashWStr;

    if (pathWStr == L"")
        return; // what???

#ifdef __CYGWIN__
    // cygwin allows win- and linux-style, so add the right slash:
    // But if you don't use cygwin environment, you must use win-style,
    // to make chdir working - but file handling also work with linux-style ;-)
    slashWStr = L"\\";
    if (pathWStr.substr(0,1) == L"/")
        slashWStr = L"/";
#elif defined(_WIN32) || defined(_WIN64)
    slashWStr = L"\\";
#else
    slashWStr = L"/";
#endif

    if (pathWStr.substr(pathWStr.length()-1,1) != slashWStr)
        pathWStr += slashWStr; 
}

namespace {

std::wstring WorkingDirectory() {
#if defined(_WIN32) || defined(_WIN64)
    const DWORD size = GetCurrentDirectoryW(0, nullptr);
    if (!size) return L"";
    std::vector<wchar_t> path(size);
    const DWORD length = GetCurrentDirectoryW(size, path.data());
    if (!length || length >= size) return L"";
    return std::wstring(path.data(), length);
#else
    std::vector<char> path(1024);
    while (!getcwd(path.data(), path.size())) {
        if (errno != ERANGE) return L"";
        path.resize(path.size() * 2);
    }
    return CStr2WStr(path.data());
#endif
}

std::wstring AbsoluteDirectory(std::wstring path) {
    if (path.empty()) return path;
    if (!isabsolute(WStr2Str(path).c_str())) {
        std::wstring current = WorkingDirectory();
        PathEndSlash(current);
        path = current + path;
    }
    PathEndSlash(path);
    return path;
}

std::wstring ExecutableDirectory() {
    std::wstring path;
#if defined(_WIN32) || defined(_WIN64)
    std::vector<wchar_t> buffer(1024);
    for (;;) {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                               static_cast<DWORD>(buffer.size()));
        if (!length) return L"";
        if (length < buffer.size()) {
            path.assign(buffer.data(), length);
            break;
        }
        buffer.resize(buffer.size() * 2);
    }
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) return L"";
    // Resolve symlinks and any relative components before finding sibling data.
    char *resolved = realpath(buffer.data(), nullptr);
    if (!resolved) return L"";
    path = CStr2WStr(resolved);
    free(resolved);
#else
    std::vector<char> buffer(1024);
    for (;;) {
        const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if (length < 0) return L"";
        if (static_cast<size_t>(length) < buffer.size()) {
            // readlink does not append a null terminator.
            path = Str2WStr(std::string(buffer.data(), static_cast<size_t>(length)));
            break;
        }
        buffer.resize(buffer.size() * 2);
    }
#endif
    const auto slash = path.find_last_of(L"/\\");
    return slash == std::wstring::npos ? L"" : path.substr(0, slash + 1);
}

bool TryRodentHome(const std::wstring &directory) {
    if (directory.empty()) return false;
    const std::wstring path = AbsoluteDirectory(directory);
    if (!DirOrFileExists(WStr2Str(path + L"personalities/basic.ini").c_str()))
        return false;
    RodentHomeDirWStr = path;
    return true;
}

} // namespace

// --------------------------------------------------------------------------------
// SetRodentHomeDir
// --------------------------------------------------------------------------------

void SetRodentHomeDir() {
    RodentHomeDirWStr.clear();
#ifndef DEBUG
    SkipBeginningOfLog = true;
#else
    SkipBeginningOfLog = false;
#endif

    const std::string overrideHome = GetEnv("RODENT4HOME");
    const std::wstring executableDir = ExecutableDirectory();
    if (!overrideHome.empty()) {
        RodentHomeDirWStr = AbsoluteDirectory(Str2WStr(overrideHome));
    } else if (!TryRodentHome(executableDir)) {
#ifdef RODENT_INSTALL_DATA_SUBDIR
        TryRodentHome(executableDir.empty() ? L"" :
                      executableDir + CStr2WStr(RODENT_INSTALL_DATA_SUBDIR));
#endif
#ifdef RODENT_INSTALL_DATADIR
        if (RodentHomeDirWStr.empty())
            TryRodentHome(CStr2WStr(RODENT_INSTALL_DATADIR));
#endif
#ifdef ANDROID
        if (RodentHomeDirWStr.empty()) TryRodentHome(L"/sdcard/Rodent4/");
        if (RodentHomeDirWStr.empty())
            TryRodentHome(L"/sdcard/Android/data/ccc.chess.engines/files/");
#endif
        if (RodentHomeDirWStr.empty())
            RodentHomeDirWStr = AbsoluteDirectory(executableDir.empty() ? L"." : executableDir);
    }

#ifndef DEBUG
    LogFileWStr.clear();
#else
    LogFileWStr = RodentHomeDirWStr + L"rodent.log";
#endif
    printf_debug("LogFile='%s'\n", WStr2Str(LogFileWStr).c_str());

    const std::string personalityOverride = GetEnv("RODENT4PERSONALITIES");
    const std::wstring personalityDir = personalityOverride.empty()
        ? RodentHomeDirWStr + L"personalities/"
        : AbsoluteDirectory(Str2WStr(personalityOverride));
    if (!DirOrFileExists(WStr2Str(personalityDir + L"basic.ini").c_str()))
        printfUciOut("info string no 'basic.ini' - check installation, please\n");
#ifdef DEBUG
    else
        printfUciOut("info string use '%s'\n", WStr2Str(RodentHomeDirWStr).c_str());
#endif
    printf_debug("RodentHome: '%s'\n", WStr2Str(RodentHomeDirWStr).c_str());
}
