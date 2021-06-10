using System.Collections.Generic;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities.Binding
{
    internal static class EditorBindingEngine
    {
        private static readonly List<ObjectBinding> bindings = new List<ObjectBinding>();

        internal static ObjectBinding SetBinding(this DependencyObject targetObject,
            DependencyProperty targetProperty, ObjectProperty sourceProperty)
        {
            var binding = new ObjectBinding(targetObject, targetProperty, sourceProperty);
            bindings.Add(binding);
            return binding;
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