using System;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities.Binding
{
    internal class ObjectBinding : DependencyObject
    {
        static readonly Type[] AllowedTargetPropertyTypes = new[]
        {
            typeof(float),
            typeof(double),
            typeof(string)
        };

        DependencyObject targetObject;
        DependencyProperty targetProperty;
        ObjectProperty sourceProperty;

        public ObjectReference Source
        {
            get { return (ObjectReference)GetValue(SourceProperty); }
            set { SetValue(SourceProperty, value); }
        }
        public static readonly DependencyProperty SourceProperty =
            DependencyProperty.Register(
                nameof(Source),
                typeof(ObjectReference),
                typeof(ObjectBinding),
                new PropertyMetadata(ObjectReference.None));

        public ObjectBinding(
            DependencyObject targetObject, DependencyProperty targetProperty,
            ObjectProperty sourceProperty)
        {
            if (!AllowedTargetPropertyTypes.Contains(targetProperty.PropertyType))
            {
                throw new NotSupportedException(
                    $"Unable to bind object property to target property of type '{targetProperty.PropertyType}'.");
            }

            this.targetObject = targetObject;
            this.targetProperty = targetProperty;
            this.sourceProperty = sourceProperty;
        }

        public void UpdateFromSource()
        {
            uint objectId = Source.GetObjectId();
            if (objectId == 0U)
            {
                targetObject.SetValue(targetProperty,
                    targetProperty.DefaultMetadata.DefaultValue);
                return;
            }

            float value = EditorCore.GetObjectProperty(objectId, sourceProperty);
            object convertedValue = Convert.ChangeType(value, targetProperty.PropertyType);
            targetObject.SetValue(targetProperty, convertedValue);
        }
    }
}