using System;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities
{
    internal class EditorBinding
    {
        static readonly Type[] AllowedTargetPropertyTypes = new[]
        {
            typeof(float),
            typeof(string)
        };

        DependencyObject targetObject;
        DependencyProperty targetProperty;
        ObjectProperty sourceProperty;

        bool isStringProperty;

        public EditorBinding(
            DependencyObject targetObject, DependencyProperty targetProperty,
            ObjectProperty sourceProperty)
        {
            Debug.Assert(AllowedTargetPropertyTypes.Contains(targetProperty.PropertyType));

            this.targetObject = targetObject;
            this.targetProperty = targetProperty;
            this.sourceProperty = sourceProperty;

            this.isStringProperty = targetProperty.PropertyType == typeof(string);
        }

        public void UpdateFromSource()
        {
            uint objectId = App.Current.UiState.SelectedObjectId;
            float value = objectId == 0
                ? 0
                : EditorCore.GetObjectProperty(objectId, sourceProperty);

            if (isStringProperty)
            {
                targetObject.SetValue(targetProperty, value.ToString());
            }
            else
            {
                targetObject.SetValue(targetProperty, value);
            }
        }
    }
}