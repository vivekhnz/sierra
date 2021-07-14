using System;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
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

        ObservableCollection<object> backingObservableCollection;

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

                if (typeof(INotifyCollectionChanged).IsAssignableFrom(targetProperty.PropertyType))
                {
                    backingObservableCollection = (ObservableCollection<object>)
                        targetObject.GetValue(targetProperty);
                    if (backingObservableCollection == null)
                    {
                        backingObservableCollection = new ObservableCollection<object>();
                        SetTargetPropertyValue(backingObservableCollection);
                    }
                    backingObservableCollection.CollectionChanged += OnTargetPropertyCollectionChanged;
                }
            }
        }

        private void OnTargetPropertyValueChanged(object sender, EventArgs e)
        {
            if (isTargetUpdatingFromBinding) return;

            isTargetUpdatingFromUi = true;
            newValueFromUi = targetObject.GetValue(targetProperty);
        }

        private void OnTargetPropertyCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (isTargetUpdatingFromBinding) return;

            isTargetUpdatingFromUi = true;
        }

        public void Update(ref EditorUiState state)
        {
            if (isTargetUpdatingFromUi)
            {
                // update source
                void SetSourceProperty<T>(ref T prop)
                {
                    if (newValueFromUi == null)
                    {
                        prop = default(T);
                    }
                    else if (typeof(T).IsEnum)
                    {
                        prop = (T)newValueFromUi;
                    }
                    else
                    {
                        prop = (T)Convert.ChangeType(newValueFromUi, typeof(T));
                    }
                }
                switch (sourceProperty)
                {
                    case UiProperty.CurrentContext:
                        SetSourceProperty(ref state.CurrentContext);
                        break;
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
                    case UiProperty.SelectedObjectIds:
                        state.SelectedObjectCount = 0;
                        foreach (uint objectId in backingObservableCollection.OfType<uint>())
                        {
                            state.SelectedObjectIds[(int)state.SelectedObjectCount] = objectId;
                            state.SelectedObjectCount++;
                        }
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
                if (sourceProperty == UiProperty.SelectedObjectIds)
                {
                    isTargetUpdatingFromBinding = true;
                    backingObservableCollection.Clear();
                    for (int i = 0; i < state.SelectedObjectCount; i++)
                    {
                        backingObservableCollection.Add(state.SelectedObjectIds[i]);
                    }
                    isTargetUpdatingFromBinding = false;
                }

                object value = sourceProperty switch
                {
                    UiProperty.CurrentContext => state.CurrentContext,
                    UiProperty.TerrainBrushTool => state.TerrainBrushTool,
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
        CurrentContext,
        TerrainBrushTool,
        TerrainBrushRadius,
        TerrainBrushFalloff,
        TerrainBrushStrength,
        SelectedObjectIds,
        SceneLightDirection,
    }
}