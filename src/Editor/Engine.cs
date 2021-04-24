using Terrain.Engine.Interop;

namespace Terrain.Editor
{
    internal static class Engine
    {
        internal static TextureAssetRegistrationProxy[] GetRegisteredTextureAssets()
        {
            return EngineInterop.GetRegisteredTextureAssets();
        }
    }
}
