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

        property EditorTool CurrentTool
        {
            EditorTool get()
            {
                return state.tool;
            }

            void set(EditorTool value)
            {
                state.tool = value;
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

        property float BrushFalloff
        {
            float get()
            {
                return state.brushFalloff;
            }

            void set(float value)
            {
                state.brushFalloff = value;
            }
        }

        property float LightDirection
        {
            float get()
            {
                return state.lightDirection;
            }

            void set(float value)
            {
                state.lightDirection = value;
            }
        }

        property float Material1TextureSize
        {
            float get()
            {
                return state.mat1_textureSize;
            }

            void set(float value)
            {
                state.mat1_textureSize = value;
            }
        }

        property float Material2TextureSize
        {
            float get()
            {
                return state.mat2_textureSize;
            }

            void set(float value)
            {
                state.mat2_textureSize = value;
            }
        }
    };
}}}}