using System;

namespace Terrain.Editor.Utilities.Binding
{
    public class ObjectReference
    {
        public static ObjectReference None { get; } = new ObjectReference(() => 0U);
        public static ObjectReference ById(uint id) => new ObjectReference(() => id);

        private readonly Func<uint> getObjectId;

        public ObjectReference(Func<uint> getObjectId)
        {
            this.getObjectId = getObjectId;
        }

        public uint GetObjectId() => getObjectId();
    }
}