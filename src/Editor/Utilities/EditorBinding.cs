using System;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities
{
    internal class EditorBinding : DependencyObject
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

        public uint SourceObjectId
        {
            get { return (uint)GetValue(SourceObjectIdProperty); }
            set { SetValue(SourceObjectIdProperty, value); }
        }
        public static readonly DependencyProperty SourceObjectIdProperty =
            DependencyProperty.Register(
                nameof(SourceObjectId),
                typeof(uint),
                typeof(EditorBinding),
                new PropertyMetadata(0U));

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
            float value = SourceObjectId == 0
                ? 0
                : EditorCore.GetObjectProperty(SourceObjectId, sourceProperty);

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