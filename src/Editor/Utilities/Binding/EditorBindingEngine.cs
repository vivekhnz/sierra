using System.Collections.Generic;
using System.Windows;
using Sierra.Core;

namespace Sierra.Utilities.Binding
{
    internal static class EditorBindingEngine
    {
        private static readonly List<ObjectBinding> objectBindings = new List<ObjectBinding>();
        private static readonly List<UiBinding> uiBindings = new List<UiBinding>();

        internal static ObjectBinding SetObjectBinding(this DependencyObject targetObject,
            DependencyProperty targetProperty, ObjectProperty sourceProperty)
        {
            var binding = new ObjectBinding(targetObject, targetProperty, sourceProperty);
            objectBindings.Add(binding);
            return binding;
        }

        internal static UiBinding SetUiBinding(this DependencyObject targetObject,
            DependencyProperty targetProperty, UiProperty sourceProperty)
        {
            var binding = new UiBinding(targetObject, targetProperty, sourceProperty);
            uiBindings.Add(binding);
            return binding;
        }

        internal static void UpdateBindings(ref EditorUiState uiState)
        {
            foreach (var binding in objectBindings)
            {
                binding.UpdateFromSource();
            }
            foreach (var binding in uiBindings)
            {
                binding.Update(ref uiState);
            }
        }
    }
}