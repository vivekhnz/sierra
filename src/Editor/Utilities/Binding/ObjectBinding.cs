using System;
using System.ComponentModel;
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

        bool isSettingTargetProperty;

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

            var targetPropMetadata = targetProperty.GetMetadata(targetProperty.OwnerType);
            if (targetPropMetadata is FrameworkPropertyMetadata frameworkPropertyMetadata &&
                frameworkPropertyMetadata.BindsTwoWayByDefault)
            {
                var targetPropDescriptor = DependencyPropertyDescriptor.FromProperty(
                    targetProperty, targetProperty.OwnerType);
                targetPropDescriptor.AddValueChanged(targetObject, OnTargetPropertyValueChanged);
            }
        }

        private void OnTargetPropertyValueChanged(object sender, EventArgs e)
        {
            if (isSettingTargetProperty) return;

            uint objectId = Source.GetObjectId();
            if (objectId == 0U) return;

            object value = targetObject.GetValue(targetProperty);
            float convertedValue = (float)Convert.ChangeType(value, typeof(float));

            if (EditorCore.BeginTransaction(out var tx))
            {
                EditorCore.SetObjectProperty(tx, objectId, sourceProperty, convertedValue);
                EditorCore.CommitTransaction(tx);
            }
        }

        public void UpdateFromSource()
        {
            uint objectId = Source.GetObjectId();
            if (objectId == 0U)
            {
                SetTargetPropertyValue(targetProperty.DefaultMetadata.DefaultValue);
                return;
            }

            float value = EditorCore.GetObjectProperty(objectId, sourceProperty);
            object convertedValue = Convert.ChangeType(value, targetProperty.PropertyType);
            SetTargetPropertyValue(convertedValue);
        }

        private void SetTargetPropertyValue(object value)
        {
            isSettingTargetProperty = true;
            targetObject.SetValue(targetProperty, value);
            isSettingTargetProperty = false;
        }
    }
}