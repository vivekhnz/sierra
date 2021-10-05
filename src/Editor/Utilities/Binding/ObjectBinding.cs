using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using Sierra.Core;

namespace Sierra.Utilities.Binding
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
        Transaction activeTx;

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
                UiTransactionHelper.RegisterTransactionEventHandlers(
                    targetObject, targetProperty,
                    BeginTransaction, CommitTransaction, DiscardTransaction);
            }
        }

        private void BeginTransaction()
        {
            Debug.Assert(!activeTx.IsValid);
            if (EditorCore.BeginTransaction(out var tx))
            {
                activeTx = tx;
            }
        }

        private void CommitTransaction()
        {
            if (activeTx.IsValid)
            {
                EditorCore.CommitTransaction(activeTx);
                activeTx = Transaction.Invalid;
            }
        }

        private void DiscardTransaction()
        {
            if (activeTx.IsValid)
            {
                EditorCore.DiscardTransaction(activeTx);
                activeTx = Transaction.Invalid;
            }
        }

        private void OnTargetPropertyValueChanged(object sender, EventArgs e)
        {
            if (isSettingTargetProperty) return;

            var objectIds = Source.GetObjectIds();
            if (objectIds.Count() == 0) return;

            object value = targetObject.GetValue(targetProperty);
            float convertedValue = (float)Convert.ChangeType(value, typeof(float));

            if (activeTx.IsValid)
            {
                EditorCore.ClearTransaction(activeTx);
                foreach (uint objectId in objectIds)
                {
                    EditorCore.SetObjectProperty(
                        activeTx, objectId, sourceProperty, convertedValue);
                }
            }
            else
            {
                if (EditorCore.BeginTransaction(out var tx))
                {
                    foreach (uint objectId in objectIds)
                    {
                        EditorCore.SetObjectProperty(
                            tx, objectId, sourceProperty, convertedValue);
                    }
                    EditorCore.CommitTransaction(tx);
                }
            }
        }

        public void UpdateFromSource()
        {
            var objectIds = Source.GetObjectIds();
            int objectCount = objectIds.Count();
            if (objectCount == 0)
            {
                SetTargetPropertyValue(targetProperty.DefaultMetadata.DefaultValue);
                return;
            }

            uint firstObjectId = objectIds.First();
            float firstObjectValue = EditorCore.GetObjectProperty(firstObjectId, sourceProperty);

            foreach (uint objectId in objectIds.Skip(1))
            {
                float value = EditorCore.GetObjectProperty(objectId, sourceProperty);
                if (value != firstObjectValue)
                {
                    SetTargetPropertyValue(targetProperty.DefaultMetadata.DefaultValue);
                    return;
                }
            }

            object convertedValue = Convert.ChangeType(
                firstObjectValue, targetProperty.PropertyType);
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