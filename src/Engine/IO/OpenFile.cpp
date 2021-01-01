#include "OpenFile.hpp"

#include <sstream>
#include "Path.hpp"

namespace Terrain { namespace Engine { namespace IO {
    OpenFile::OpenFile(std::string path)
    {
        fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fileStream.open(path);
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
}}}