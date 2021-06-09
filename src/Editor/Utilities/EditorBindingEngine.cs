using System.Collections.Generic;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities
{
    internal static class EditorBindingEngine
    {
        private static readonly List<EditorBinding> bindings = new List<EditorBinding>();

        internal static void SetBinding(this DependencyObject targetObject,
            DependencyProperty targetProperty, ObjectProperty sourceProperty)
        {
            bindings.Add(new EditorBinding(targetObject, targetProperty, sourceProperty));
        }

        internal static void UpdateBindings()
        {
            foreach (var binding in bindings)
            {
                binding.UpdateFromSource();
            }
        }
    }
}