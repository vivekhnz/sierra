using System;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities.Binding
{
    internal class UiBinding : DependencyObject
    {
        static readonly Type[] AllowedTargetPropertyTypes = new[]
        {
            typeof(float),
            typeof(double),
            typeof(string)
        };

        DependencyObject targetObject;
        DependencyProperty targetProperty;
        UiProperty sourceProperty;

        bool isSettingTargetProperty;
        object newValueFromTarget;

        public UiBinding(
            DependencyObject targetObject, DependencyProperty targetProperty,
            UiProperty sourceProperty)
        {
            if (!AllowedTargetPropertyTypes.Contains(targetProperty.PropertyType))
            {
                throw new NotSupportedException(
                    $"Unable to bind UI property to target property of type '{targetProperty.PropertyType}'.");
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

            newValueFromTarget = targetObject.GetValue(targetProperty);
        }

        public void Update(ref EditorUiState state)
        {
            if (newValueFromTarget == null)
            {
                // update target
                object value = sourceProperty switch
                {
                    UiProperty.TerrainBrushRadius => state.TerrainBrushRadius,
                    UiProperty.TerrainBrushFalloff => state.TerrainBrushFalloff,
                    UiProperty.TerrainBrushStrength => state.TerrainBrushStrength,
                    UiProperty.SceneLightDirection => state.SceneLightDirection,
                    _ => null
                };
                if (value == null)
                {
                    SetTargetPropertyValue(targetProperty.DefaultMetadata.DefaultValue);
                    return;
                }

                object convertedValue = Convert.ChangeType(value, targetProperty.PropertyType);
                SetTargetPropertyValue(convertedValue);
            }
            else
            {
                // update source
                void SetSourceProperty<T>(ref T prop)
                {
                    prop = (T)Convert.ChangeType(newValueFromTarget, typeof(T));
                }
                switch (sourceProperty)
                {
                    case UiProperty.TerrainBrushRadius:
                        SetSourceProperty(ref state.TerrainBrushRadius);
                        break;
                    case UiProperty.TerrainBrushFalloff:
                        SetSourceProperty(ref state.TerrainBrushFalloff);
                        break;
                    case UiProperty.TerrainBrushStrength:
                        SetSourceProperty(ref state.TerrainBrushStrength);
                        break;
                    case UiProperty.SceneLightDirection:
                        SetSourceProperty(ref state.SceneLightDirection);
                        break;
                }
                newValueFromTarget = null;
            }
        }

        private void SetTargetPropertyValue(object value)
        {
            isSettingTargetProperty = true;
            targetObject.SetValue(targetProperty, value);
            isSettingTargetProperty = false;
        }
    }

    internal enum UiProperty
    {
        TerrainBrushRadius,
        TerrainBrushFalloff,
        TerrainBrushStrength,
        SceneLightDirection,
    }
}