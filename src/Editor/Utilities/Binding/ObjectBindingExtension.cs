using System;
using System.Windows;
using System.Windows.Markup;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities.Binding
{
    [MarkupExtensionReturnType(typeof(object))]
    internal class ObjectBindingExtension : MarkupExtension
    {
        private ObjectProperty sourceProperty;

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
                var binding = EditorBindingEngine.SetBinding(
                    targetObject, targetProperty, sourceProperty);
                binding.Source = App.Current?.UiState?.SelectedObject ?? ObjectReference.None;

                if (targetProperty.PropertyType == typeof(string))
                {
                    return sourceProperty.ToString();
                }

                return targetProperty.DefaultMetadata.DefaultValue;
            }

            return null;
        }
    }
}