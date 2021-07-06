using System;
using System.Collections.Generic;

namespace Terrain.Editor.Utilities.Binding
{
    public class ObjectReference
    {
        public static ObjectReference None { get; } = new ObjectReference(() => new uint[0]);
        public static ObjectReference ById(uint id) => new ObjectReference(() => new uint[] { id });

        private readonly Func<IEnumerable<uint>> getObjectIds;

        public ObjectReference(Func<IEnumerable<uint>> getObjectIds)
        {
            this.getObjectIds = getObjectIds;
        }

        public IEnumerable<uint> GetObjectIds() => getObjectIds();
    }
}