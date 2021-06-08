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
            set { SetAndNotify(ref position.X, value); UpdateTransform(); }
        }
        public float PositionY
        {
            get => position.Y;
            set { SetAndNotify(ref position.Y, value); UpdateTransform(); }
        }
        public float PositionZ
        {
            get => position.Z;
            set { SetAndNotify(ref position.Z, value); UpdateTransform(); }
        }

        public float RotationX
        {
            get => rotation.X;
            set { SetAndNotify(ref rotation.X, value); UpdateTransform(); }
        }
        public float RotationY
        {
            get => rotation.Y;
            set { SetAndNotify(ref rotation.Y, value); UpdateTransform(); }
        }
        public float RotationZ
        {
            get => rotation.Z;
            set { SetAndNotify(ref rotation.Z, value); UpdateTransform(); }
        }

        public float ScaleX
        {
            get => scale.X;
            set { SetAndNotify(ref scale.X, value); UpdateTransform(); }
        }
        public float ScaleY
        {
            get => scale.Y;
            set { SetAndNotify(ref scale.Y, value); UpdateTransform(); }
        }
        public float ScaleZ
        {
            get => scale.Z;
            set { SetAndNotify(ref scale.Z, value); UpdateTransform(); }
        }

        public EditorObjectViewModel(uint objectId)
        {
            ObjectId = objectId;

            position = new Vector3(0, 0, 0);
            rotation = new Vector3(0, 0, 0);
            scale = new Vector3(1, 1, 1);
        }

        private void UpdateTransform()
        {
            EditorCore.SetObjectTransform(ObjectId,
                position.X, position.Y, position.Z,
                rotation.X, rotation.Y, rotation.Z,
                scale.X, scale.Y, scale.Z);
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