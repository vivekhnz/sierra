using Terrain.Editor.Engine;

namespace Terrain.Editor.ViewModels
{
    public class AssetViewModel : ViewModelBase
    {
        private uint assetId;
        public uint AssetId
        {
            get => assetId;
            set
            {
                assetId = value;
                NotifyPropertyChanged(nameof(AssetId));
            }
        }

        private AssetRegistrationType registrationType;
        public AssetRegistrationType RegistrationType
        {
            get => registrationType;
            set
            {
                registrationType = value;
                NotifyPropertyChanged(nameof(RegistrationType));
            }
        }

        private AssetType assetType;
        public AssetType AssetType
        {
            get => assetType;
            set
            {
                assetType = value;
                NotifyPropertyChanged(nameof(AssetType));
            }
        }

        private string fileRelativePath;
        public string FileRelativePath
        {
            get => fileRelativePath;
            set
            {
                fileRelativePath = value;
                NotifyPropertyChanged(nameof(FileRelativePath));
            }
        }
    }
}