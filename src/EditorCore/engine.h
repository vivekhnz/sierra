#ifndef ENGINE_H
#define ENGINE_H

#include "engine_platform.h"
#include "engine_math.h"
#include "engine_renderer_common.h"
#include "engine_renderer.h"
#include "engine_assets.h"

typedef void *GetGLProcAddress(const char *procName);
void reloadEngine(GetGLProcAddress *getGlProcAddress, EnginePlatformApi platformApi);

#endif