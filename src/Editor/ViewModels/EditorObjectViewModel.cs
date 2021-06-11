using System.Runtime.CompilerServices;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;

namespace Terrain.Editor.ViewModels
{
    public class EditorObjectViewModel : ViewModelBase
    {
        public uint ObjectId { get; private set; }

        private string name;
        public string Name { get => name; set => SetAndNotify(ref name, value); }

        private Vector3 position;
        private Vector3 rotation;
        private Vector3 scale;

        public float PositionX
        {
            get => position.X;
            set => SetObjectPropertyInternal(ref position.X, ObjectProperty.ObjectPositionX, value);
        }
        public float PositionY
        {
            get => position.Y;
            set => SetObjectPropertyInternal(ref position.Y, ObjectProperty.ObjectPositionY, value);
        }
        public float PositionZ
        {
            get => position.Z;
            set => SetObjectPropertyInternal(ref position.Z, ObjectProperty.ObjectPositionZ, value);
        }

        public float RotationX
        {
            get => rotation.X;
            set => SetObjectPropertyInternal(ref rotation.X, ObjectProperty.ObjectRotationX, value);
        }
        public float RotationY
        {
            get => rotation.Y;
            set => SetObjectPropertyInternal(ref rotation.Y, ObjectProperty.ObjectRotationY, value);
        }
        public float RotationZ
        {
            get => rotation.Z;
            set => SetObjectPropertyInternal(ref rotation.Z, ObjectProperty.ObjectRotationZ, value);
        }

        public float ScaleX
        {
            get => scale.X;
            set => SetObjectPropertyInternal(ref scale.X, ObjectProperty.ObjectScaleX, value);
        }
        public float ScaleY
        {
            get => scale.Y;
            set => SetObjectPropertyInternal(ref scale.Y, ObjectProperty.ObjectScaleY, value);
        }
        public float ScaleZ
        {
            get => scale.Z;
            set => SetObjectPropertyInternal(ref scale.Z, ObjectProperty.ObjectScaleZ, value);
        }

        public EditorObjectViewModel(uint objectId)
        {
            ObjectId = objectId;

            position = new Vector3(0, 0, 0);
            rotation = new Vector3(0, 0, 0);
            scale = new Vector3(1, 1, 1);
        }

        private void SetObjectPropertyInternal(
            ref float field, ObjectProperty property,
            float value, [CallerMemberName] string propertyName = null)
        {
            SetAndNotify(ref field, value, propertyName);
            EditorCore.SetObjectProperty(ObjectId, property, value);
        }

        internal void SetProperty(ObjectProperty property, float value)
        {
            switch (property)
            {
                case ObjectProperty.ObjectPositionX:
                    SetAndNotify(ref position.X, value, nameof(PositionX));
                    break;
                case ObjectProperty.ObjectPositionY:
                    SetAndNotify(ref position.Y, value, nameof(PositionY));
                    break;
                case ObjectProperty.ObjectPositionZ:
                    SetAndNotify(ref position.Z, value, nameof(PositionZ));
                    break;
                case ObjectProperty.ObjectRotationX:
                    SetAndNotify(ref rotation.X, value, nameof(RotationX));
                    break;
                case ObjectProperty.ObjectRotationY:
                    SetAndNotify(ref rotation.Y, value, nameof(RotationY));
                    break;
                case ObjectProperty.ObjectRotationZ:
                    SetAndNotify(ref rotation.Z, value, nameof(RotationZ));
                    break;
                case ObjectProperty.ObjectScaleX:
                    SetAndNotify(ref scale.X, value, nameof(ScaleX));
                    break;
                case ObjectProperty.ObjectScaleY:
                    SetAndNotify(ref scale.Y, value, nameof(ScaleY));
                    break;
                case ObjectProperty.ObjectScaleZ:
                    SetAndNotify(ref scale.Z, value, nameof(ScaleZ));
                    break;
            }
        }
    }
}