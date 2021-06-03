using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Terrain.Editor.Core
{
    internal ref struct EditorCommandEntry
    {
        public EditorCommandType Type;
        public ReadOnlySpan<byte> Data;

        public ref readonly T As<T>() where T : struct
            => ref MemoryMarshal.AsRef<T>(Data);
    }

    internal ref struct EditorCommandList
    {
        private readonly ReadOnlySpan<byte> commandBuffer;

        public Enumerator GetEnumerator() => new Enumerator(commandBuffer);

        public EditorCommandList(ReadOnlySpan<byte> commandBuffer)
        {
            this.commandBuffer = commandBuffer;
        }

        public ref struct Enumerator
        {
            private ReadOnlySpan<byte> commandBuffer;
            private int offset;

            private EditorCommandEntry currentEntry;

            public Enumerator(ReadOnlySpan<byte> commandBuffer)
            {
                this.commandBuffer = commandBuffer;
                this.offset = 0;
                currentEntry = new EditorCommandEntry();
            }

            public EditorCommandEntry Current => currentEntry;

            public bool MoveNext()
            {
                if (offset >= commandBuffer.Length) return false;

                ReadOnlySpan<byte> cmdTypeSpan = commandBuffer.Slice(offset, sizeof(uint));
                ref readonly EditorCommandType cmdType =
                    ref MemoryMarshal.AsRef<EditorCommandType>(cmdTypeSpan);
                Debug.Assert(Enum.IsDefined(typeof(EditorCommandType), cmdType));
                offset += cmdTypeSpan.Length;

                ReadOnlySpan<byte> cmdSizeSpan = commandBuffer.Slice(offset, sizeof(ulong));
                ref readonly ulong cmdSize = ref MemoryMarshal.AsRef<ulong>(cmdSizeSpan);
                offset += cmdSizeSpan.Length;

                currentEntry.Type = cmdType;
                currentEntry.Data = commandBuffer.Slice(offset, (int)cmdSize);
                offset += (int)cmdSize;

                return true;
            }
        }
    }
}