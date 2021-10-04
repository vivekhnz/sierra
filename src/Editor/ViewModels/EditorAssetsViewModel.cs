using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Data;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;

namespace Terrain.Editor.ViewModels
{
    public class EditorAssetsViewModel : ViewModelBase
    {
        public ObservableCollection<AssetViewModel> RegisteredAssets { get; private set; }
            = new ObservableCollection<AssetViewModel>();

        internal void OnAssetRegistered(in AssetRegistration assetReg)
        {
            var assetVm = new AssetViewModel
            {
                AssetHandle = assetReg.Handle,
                RegistrationType = assetReg.RegistrationType,
                AssetType = assetReg.AssetType
            };
            if (assetVm.RegistrationType == AssetRegistrationType.File)
            {
                var fileState = assetReg.GetFileState();
                assetVm.FileRelativePath = fileState.RelativePath;
            }
            RegisteredAssets.Add(assetVm);
        }

        internal static FilterEventHandler BuildAssetFilter(
            IEnumerable<AssetRegistrationType> registrationTypes,
            IEnumerable<AssetType> assetTypes)
        {
            return (sender, args) =>
            {
                args.Accepted = args.Item is AssetViewModel assetVm
                    && registrationTypes.Contains(assetVm.RegistrationType)
                    && assetTypes.Contains(assetVm.AssetType);
            };
        }
    }
}