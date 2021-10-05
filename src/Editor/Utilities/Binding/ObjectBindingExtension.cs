using System;
using System.Windows;
using System.Windows.Markup;
using Sierra.Core;

namespace Sierra.Utilities.Binding
{
    [MarkupExtensionReturnType(typeof(object))]
    internal class ObjectBindingExtension : MarkupExtension
    {
        private ObjectProperty sourceProperty;

        private static ObjectReference selectedObjectRef = new ObjectReference(
            () =>
            {
                ref EditorUiState state = ref EditorCore.GetUiState();
                return state.SelectedObjectIds
                    .Slice(0, (int)state.SelectedObjectCount)
                    .ToArray();
            });

        public ObjectBindingExtension(ObjectProperty sourceProperty)
        {
            this.sourceProperty = sourceProperty;
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            var provideValueTarget = serviceProvider.GetService(
                typeof(IProvideValueTarget)) as IProvideValueTarget;
            if (provideValueTarget == null) return null;

            if (provideValueTarget.TargetObject is DependencyObject targetObject &&
                provideValueTarget.TargetProperty is DependencyProperty targetProperty)
            {
                var binding = EditorBindingEngine.SetObjectBinding(
                    targetObject, targetProperty, sourceProperty);
                binding.Source = selectedObjectRef;
                return targetProperty.DefaultMetadata.DefaultValue;
            }

            return null;
        }
    }
}