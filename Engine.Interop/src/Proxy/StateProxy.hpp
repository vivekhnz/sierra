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

        property float BrushQuadX
        {
            float get()
            {
                return state.brushQuadX;
            }

            void set(float value)
            {
                state.brushQuadX = value;
            }
        }

        property float BrushQuadY
        {
            float get()
            {
                return state.brushQuadY;
            }

            void set(float value)
            {
                state.brushQuadY = value;
            }
        }
    };
}}}}