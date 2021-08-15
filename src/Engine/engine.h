#ifndef ENGINE_H
#define ENGINE_H

#include "engine_platform.h"
#include "engine_heightfield.h"
#include "engine_renderer_common.h"
#include "engine_renderer.h"
#include "engine_assets.h"

#include "engine_generated.h"

typedef void *GetGLProcAddress(const char *procName);

#define ENGINE_GET_API(name) EngineApi *name(GetGLProcAddress *getGlProcAddress, EnginePlatformApi platformApi)
typedef ENGINE_GET_API(EngineGetApi);

#endif