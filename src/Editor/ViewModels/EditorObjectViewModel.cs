using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor.ViewModels
{
    public class EditorObjectViewModel : ViewModelBase
    {
        public uint ObjectId { get; private set; }
        public string Name { get; private set; }

        public DelegateCommand DeleteObjectCommand { get; private set; }

        public EditorObjectViewModel(uint objectId)
        {
            ObjectId = objectId;
            Name = $"Object {objectId}";

            DeleteObjectCommand = new DelegateCommand(
                () => EditorCore.DeleteObject(ObjectId),
                () => true);
        }
    }
}