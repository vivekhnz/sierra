#ifndef IO_PATH_HPP
#define IO_PATH_HPP

#include <string>
#include <windows.h>

namespace Path
{
std::string getAbsolutePath(std::string relativePath)
{
    // get path to current assembly
    CHAR exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    // build absolute path
    return std::string(exePath) + "/../" + relativePath;
}
} // namespace Path

#endif