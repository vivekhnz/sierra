#pragma once

#include "../../../Engine/src/EngineViewContext.hpp"

namespace Terrain { namespace Engine { namespace Interop { namespace Worlds {
    class EditorWorld
    {
    public:
        virtual void initialize() = 0;
        virtual int addCamera(int inputControllerId) = 0;
        virtual void update(float deltaTime) = 0;
        virtual void render(EngineViewContext &vctx) = 0;
    };
}}}}