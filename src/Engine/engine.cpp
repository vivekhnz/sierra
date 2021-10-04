#include "engine.h"

#include "engine_renderer.cpp"
#include "engine_render_backend_opengl.cpp"
#include "engine_assets.cpp"

#include "engine_generated.cpp"

#include "../../deps/stb/stb_image.c"
#include "../../deps/fast_obj/fast_obj.c"

global_variable EngineApi Api;
global_variable bool WasEngineReloaded = true;
global_variable EnginePlatformApi Platform;

API_EXPORT ENGINE_GET_API(engineGetApi)
{
    Platform = platformApi;
    if (WasEngineReloaded)
    {
        RenderBackendInitParams initParams;
        initParams.getGlProcAddress = getGlProcAddress;
        reloadRenderBackend(initParams);

        WasEngineReloaded = false;
    }

    bindApi(&Api);
    return &Api;
}