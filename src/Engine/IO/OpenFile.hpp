#ifndef IO_OPENFILE_HPP
#define IO_OPENFILE_HPP

#include "../Common.hpp"
#include <fstream>

namespace Terrain { namespace Engine { namespace IO {
    class EXPORT OpenFile
    {
        std::ifstream fileStream;

    public:
        OpenFile(std::string path);
        OpenFile(const OpenFile &that) = delete;
        OpenFile &operator=(const OpenFile &that) = delete;
        OpenFile(OpenFile &&) = delete;
        OpenFile &operator=(OpenFile &&) = delete;

        std::string readAllText();

        ~OpenFile();
    };
}}}

#endif