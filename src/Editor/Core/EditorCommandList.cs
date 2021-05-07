using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Terrain.Editor.Core
{
    internal ref struct EditorCommandList
    {
        internal ref struct Span
        {
            public bool IsValid;
            public EditorCommandType Type;
            public int Offset;
            public int Size;
        }

        Span<byte> commandBuffer;
        int offset;

        public EditorCommandList(Span<byte> commandBuffer)
        {
            this.commandBuffer = commandBuffer;
            offset = 0;
        }

        public Span Pop()
        {
            if (offset >= commandBuffer.Length) return new Span { IsValid = false };

            EditorCommandType type = PopInternal<EditorCommandType>();
            Span result = new Span
            {
                IsValid = true,
                Type = type,
                Offset = offset,
                Size = type switch
                {
                    EditorCommandType.AddMaterial => Unsafe.SizeOf<AddMaterialCommand>(),
                    EditorCommandType.DeleteMaterial => Unsafe.SizeOf<DeleteMaterialCommand>(),
                    EditorCommandType.SwapMaterial => Unsafe.SizeOf<SwapMaterialCommand>(),
                    EditorCommandType.SetMaterialTexture => Unsafe.SizeOf<SetMaterialTextureCommand>(),
                    EditorCommandType.SetMaterialProperties => Unsafe.SizeOf<SetMaterialPropertiesCommand>(),
                    _ => 0
                }
            };
            offset += result.Size;

            return result;
        }

        private ref T PopInternal<T>()
            where T : struct
        {
            int size = Unsafe.SizeOf<T>();
            Span<byte> sliceSpan = commandBuffer.Slice(offset, size);
            offset += size;
            return ref MemoryMarshal.AsRef<T>(sliceSpan);
        }

        public ref T Get<T>(Span span)
            where T : struct
        {
            Span<byte> sliceSpan = commandBuffer.Slice(span.Offset, span.Size);
            return ref MemoryMarshal.AsRef<T>(sliceSpan);
        }
    }
}