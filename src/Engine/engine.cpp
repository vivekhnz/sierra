#include "engine.h"

#include "engine_renderer.cpp"
#include "engine_render_backend_opengl.cpp"
#include "engine_assets.cpp"

#include "../../deps/stb/stb_image.c"
#include "../../deps/fast_obj/fast_obj.c"

global_variable EnginePlatformApi Platform;

void reloadEngine(GetGLProcAddress *getGlProcAddress, EnginePlatformApi platformApi)
{
    Platform = platformApi;

    RenderBackendInitParams initParams;
    initParams.getGlProcAddress = getGlProcAddress;
    reloadRenderBackend(initParams);
}