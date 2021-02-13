#include "Renderer.hpp"

#include <glad/glad.h>
#include "../terrain_renderer.h"

namespace Terrain { namespace Engine { namespace Graphics {
    Renderer::Renderer(EngineMemory *memory) : memory(memory)
    {
    }

    void Renderer::initialize()
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glPatchParameteri(GL_PATCH_VERTICES, 4);

        rendererCreateUniformBuffers(memory);
    }

    Renderer::~Renderer()
    {
        rendererDestroyResources(memory);
    }
}}}