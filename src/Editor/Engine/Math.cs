using System.Runtime.InteropServices;

namespace Terrain.Editor.Engine
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct Vector2
    {
        public float X;
        public float Y;

        public Vector2(float x, float y)
        {
            X = x;
            Y = y;
        }
    }
}