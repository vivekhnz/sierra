using System;
using System.ComponentModel;
using System.Windows;
using Terrain.Editor.Core;

namespace Terrain.Editor.Utilities.Binding
{
    internal class UiBinding : DependencyObject
    {
        DependencyObject targetObject;
        DependencyProperty targetProperty;
        UiProperty sourceProperty;

        bool isTargetUpdatingFromBinding;
        bool isTargetUpdatingFromUi;
        object newValueFromUi;

        public UiBinding(
            DependencyObject targetObject, DependencyProperty targetProperty,
            UiProperty sourceProperty)
        {
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
            if (isTargetUpdatingFromBinding) return;

            isTargetUpdatingFromUi = true;
            newValueFromUi = targetObject.GetValue(targetProperty);
        }

        public void Update(ref EditorUiState state)
        {
            if (isTargetUpdatingFromUi)
            {
                // update source
                void SetSourceProperty<T>(ref T prop)
                {
                    prop = newValueFromUi == null
                        ? default(T)
                        : (T)Convert.ChangeType(newValueFromUi, typeof(T));
                }
                switch (sourceProperty)
                {
                    case UiProperty.TerrainBrushTool:
                        SetSourceProperty(ref state.TerrainBrushTool);
                        break;
                    case UiProperty.TerrainBrushRadius:
                        SetSourceProperty(ref state.TerrainBrushRadius);
                        break;
                    case UiProperty.TerrainBrushFalloff:
                        SetSourceProperty(ref state.TerrainBrushFalloff);
                        break;
                    case UiProperty.TerrainBrushStrength:
                        SetSourceProperty(ref state.TerrainBrushStrength);
                        break;
                    case UiProperty.SelectedObjectId:
                        SetSourceProperty(ref state.SelectedObjectId);
                        break;
                    case UiProperty.SceneLightDirection:
                        SetSourceProperty(ref state.SceneLightDirection);
                        break;
                }

                newValueFromUi = null;
                isTargetUpdatingFromUi = false;
            }
            else
            {
                // update target
                object value = sourceProperty switch
                {
                    UiProperty.TerrainBrushTool => state.TerrainBrushTool,
                    UiProperty.TerrainBrushRadius => state.TerrainBrushRadius,
                    UiProperty.TerrainBrushFalloff => state.TerrainBrushFalloff,
                    UiProperty.TerrainBrushStrength => state.TerrainBrushStrength,
                    UiProperty.SelectedObjectId => state.SelectedObjectId,
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
        }

        private void SetTargetPropertyValue(object value)
        {
            isTargetUpdatingFromBinding = true;
            targetObject.SetValue(targetProperty, value);
            isTargetUpdatingFromBinding = false;
        }
    }

    internal enum UiProperty
    {
        TerrainBrushTool,
        TerrainBrushRadius,
        TerrainBrushFalloff,
        TerrainBrushStrength,
        SelectedObjectId,
        SceneLightDirection,
    }
}