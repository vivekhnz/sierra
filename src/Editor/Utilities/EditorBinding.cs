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

        public ObjectReference Source
        {
            get { return (ObjectReference)GetValue(SourceProperty); }
            set { SetValue(SourceProperty, value); }
        }
        public static readonly DependencyProperty SourceProperty =
            DependencyProperty.Register(
                nameof(Source),
                typeof(ObjectReference),
                typeof(EditorBinding),
                new PropertyMetadata(ObjectReference.None));

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
            uint objectId = Source.GetObjectId();
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

    internal class ObjectReference
    {
        internal static ObjectReference None { get; } = new ObjectReference(() => 0U);
        internal static ObjectReference ById(uint id) => new ObjectReference(() => id);

        private readonly Func<uint> getObjectId;

        public ObjectReference(Func<uint> getObjectId)
        {
            this.getObjectId = getObjectId;
        }

        internal uint GetObjectId() => getObjectId();
    }
}