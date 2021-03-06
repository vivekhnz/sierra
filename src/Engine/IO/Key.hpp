#ifndef IO_KEY_HPP
#define IO_KEY_HPP

#include "../Common.hpp"
#include "../terrain_platform.h"

namespace Terrain { namespace Engine { namespace IO {
    enum class Key : uint64
    {
        Space = 1ULL << 0,
        D0 = 1ULL << 1,
        D1 = 1ULL << 2,
        D2 = 1ULL << 3,
        D3 = 1ULL << 4,
        D4 = 1ULL << 5,
        D5 = 1ULL << 6,
        D6 = 1ULL << 7,
        D7 = 1ULL << 8,
        D8 = 1ULL << 9,
        D9 = 1ULL << 10,
        A = 1ULL << 11,
        B = 1ULL << 12,
        C = 1ULL << 13,
        D = 1ULL << 14,
        E = 1ULL << 15,
        F = 1ULL << 16,
        G = 1ULL << 17,
        H = 1ULL << 18,
        I = 1ULL << 19,
        J = 1ULL << 20,
        K = 1ULL << 21,
        L = 1ULL << 22,
        M = 1ULL << 23,
        N = 1ULL << 24,
        O = 1ULL << 25,
        P = 1ULL << 26,
        Q = 1ULL << 27,
        R = 1ULL << 28,
        S = 1ULL << 29,
        T = 1ULL << 30,
        U = 1ULL << 31,
        V = 1ULL << 32,
        W = 1ULL << 33,
        X = 1ULL << 34,
        Y = 1ULL << 35,
        Z = 1ULL << 36,
        Escape = 1ULL << 37,
        Enter = 1ULL << 38,
        Right = 1ULL << 39,
        Left = 1ULL << 40,
        Down = 1ULL << 41,
        Up = 1ULL << 42,
        F1 = 1ULL << 43,
        F2 = 1ULL << 44,
        F3 = 1ULL << 45,
        F4 = 1ULL << 46,
        F5 = 1ULL << 47,
        F6 = 1ULL << 48,
        F7 = 1ULL << 49,
        F8 = 1ULL << 50,
        F9 = 1ULL << 51,
        F10 = 1ULL << 52,
        F11 = 1ULL << 53,
        F12 = 1ULL << 54,
        LeftShift = 1ULL << 55,
        LeftCtrl = 1ULL << 56,
        LeftAlt = 1ULL << 57,
        RightShift = 1ULL << 58,
        RightCtrl = 1ULL << 59,
        RightAlt = 1ULL << 60
    };
}}}

#endif