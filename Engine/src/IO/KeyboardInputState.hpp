#ifndef IO_KEYBOARDINPUTSTATE_HPP
#define IO_KEYBOARDINPUTSTATE_HPP

#include "../Common.hpp"

namespace Terrain { namespace Engine { namespace IO {
    struct EXPORT KeyboardInputState
    {
        bool space : 1;
        bool d0 : 1;
        bool d1 : 1;
        bool d2 : 1;
        bool d3 : 1;
        bool d4 : 1;
        bool d5 : 1;
        bool d6 : 1;
        bool d7 : 1;
        bool d8 : 1;
        bool d9 : 1;
        bool a : 1;
        bool b : 1;
        bool c : 1;
        bool d : 1;
        bool e : 1;
        bool f : 1;
        bool g : 1;
        bool h : 1;
        bool i : 1;
        bool j : 1;
        bool k : 1;
        bool l : 1;
        bool m : 1;
        bool n : 1;
        bool o : 1;
        bool p : 1;
        bool q : 1;
        bool r : 1;
        bool s : 1;
        bool t : 1;
        bool u : 1;
        bool v : 1;
        bool w : 1;
        bool x : 1;
        bool y : 1;
        bool z : 1;
        bool escape : 1;
        bool enter : 1;
        bool right : 1;
        bool left : 1;
        bool down : 1;
        bool up : 1;
        bool f1 : 1;
        bool f2 : 1;
        bool f3 : 1;
        bool f4 : 1;
        bool f5 : 1;
        bool f6 : 1;
        bool f7 : 1;
        bool f8 : 1;
        bool f9 : 1;
        bool f10 : 1;
        bool f11 : 1;
        bool f12 : 1;
        bool leftShift : 1;
        bool leftControl : 1;
        bool leftAlt : 1;
        bool rightShift : 1;
        bool rightControl : 1;
        bool rightAlt : 1;
    };
}}}

#endif