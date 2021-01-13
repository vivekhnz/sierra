#pragma once

#include "../EditorState.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class RampParamsProxy
    {
    private:
        RampParams &ramp;

    public:
        RampParamsProxy(RampParams &ramp) : ramp(ramp)
        {
        }

        property float SlopeStart
        {
            float get()
            {
                return ramp.slopeStart;
            }

            void set(float value)
            {
                ramp.slopeStart = value;
            }
        }

        property float SlopeEnd
        {
            float get()
            {
                return ramp.slopeEnd;
            }

            void set(float value)
            {
                ramp.slopeEnd = value;
            }
        }

        property float AltitudeStart
        {
            float get()
            {
                return ramp.altitudeStart;
            }

            void set(float value)
            {
                ramp.altitudeStart = value;
            }
        }

        property float AltitudeEnd
        {
            float get()
            {
                return ramp.altitudeEnd;
            }

            void set(float value)
            {
                ramp.altitudeEnd = value;
            }
        }
    };
}}}}