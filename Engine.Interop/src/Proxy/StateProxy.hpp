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

        property EditStatus CurrentEditStatus
        {
            EditStatus get()
            {
                return state.editStatus;
            }

            void set(EditStatus value)
            {
                state.editStatus = value;
            }
        }
    };
}}}}