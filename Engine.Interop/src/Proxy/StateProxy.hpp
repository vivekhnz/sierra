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

        property bool DoesHeightmapRequireRedraw
        {
            bool get()
            {
                return state.doesHeightmapRequireRedraw;
            }

            void set(bool value)
            {
                state.doesHeightmapRequireRedraw = value;
            }
        }

        property bool WasHeightmapUpdated
        {
            bool get()
            {
                return state.wasHeightmapUpdated;
            }

            void set(bool value)
            {
                state.wasHeightmapUpdated = value;
            }
        }
    };
}}}}