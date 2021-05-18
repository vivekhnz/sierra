using System;
using System.Runtime.CompilerServices;
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
                offset += cmdTypeSpan.Length;

                int dataSize = cmdType switch
                {
                    EditorCommandType.AddMaterial => Unsafe.SizeOf<AddMaterialCommand>(),
                    EditorCommandType.DeleteMaterial => Unsafe.SizeOf<DeleteMaterialCommand>(),
                    EditorCommandType.SwapMaterial => Unsafe.SizeOf<SwapMaterialCommand>(),
                    EditorCommandType.SetMaterialTexture => Unsafe.SizeOf<SetMaterialTextureCommand>(),
                    EditorCommandType.SetMaterialProperties => Unsafe.SizeOf<SetMaterialPropertiesCommand>(),
                    _ => 0
                };

                currentEntry.Type = cmdType;
                currentEntry.Data = commandBuffer.Slice(offset, dataSize);
                offset += dataSize;

                return true;
            }
        }
    }
}