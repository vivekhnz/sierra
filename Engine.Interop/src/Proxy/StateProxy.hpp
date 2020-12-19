#pragma once

#include "../EditorState.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Proxy {
public
    ref class StateProxy
    {
    private:
        EditorState &state;

    public:
        StateProxy(EditorState &state) : state(state)
        {
        }

        property HeightmapStatus CurrentHeightmapStatus
        {
            HeightmapStatus get()
            {
                return state.heightmapStatus;
            }

            void set(HeightmapStatus value)
            {
                state.heightmapStatus = value;
            }
        }

        property float BrushRadius
        {
            float get()
            {
                return state.brushRadius;
            }

            void set(float value)
            {
                state.brushRadius = value;
            }
        }
    };
}}}}