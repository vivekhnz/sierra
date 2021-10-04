using System;
using Terrain.Editor.Core;

namespace Terrain.Editor.ViewModels
{
    public class AssetViewModel : ViewModelBase
    {
        private IntPtr assetHandle;
        public IntPtr AssetHandle
        {
            get => assetHandle;
            set
            {
                assetHandle = value;
                NotifyPropertyChanged(nameof(AssetHandle));
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