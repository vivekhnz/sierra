using System;
using System.Windows;
using System.Windows.Markup;

namespace Sierra.Utilities.Binding
{
    [MarkupExtensionReturnType(typeof(object))]
    internal class UiBindingExtension : MarkupExtension
    {
        private UiProperty sourceProperty;

        public UiBindingExtension(UiProperty sourceProperty)
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
                EditorBindingEngine.SetUiBinding(targetObject, targetProperty, sourceProperty);
                return targetProperty.DefaultMetadata.DefaultValue;
            }

            return null;
        }
    }
}