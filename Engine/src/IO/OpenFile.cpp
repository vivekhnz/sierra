#include "OpenFile.hpp"

#include <sstream>
#include "Path.hpp"

OpenFile::OpenFile(std::string path)
{
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fileStream.open(Path::getAbsolutePath(path));
}

std::string OpenFile::readAllText()
{
    std::stringstream stringStream;
    stringStream << fileStream.rdbuf();
    return stringStream.str();
}

OpenFile::~OpenFile()
{
    fileStream.close();
}