#ifndef IO_MOUSECAPTUREMODE_HPP
#define IO_MOUSECAPTUREMODE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace IO {
    enum class MouseCaptureMode
    {
        DoNotCapture = 0,
        Capture = 1,
        CaptureRetainPosition = 2
    };
}}}

#endif