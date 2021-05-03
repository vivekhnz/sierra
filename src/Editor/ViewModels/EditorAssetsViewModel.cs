using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Data;
using Terrain.Editor.Engine;

namespace Terrain.Editor.ViewModels
{
    public class EditorAssetsViewModel : ViewModelBase
    {
        private int prevAssetCount;

        public ObservableCollection<AssetViewModel> RegisteredAssets { get; private set; }
            = new ObservableCollection<AssetViewModel>();

        public void CheckForChanges()
        {
            int assetCount = TerrainEngine.GetRegisteredAssetCount();
            if (assetCount != prevAssetCount)
            {
                var registeredAssets = TerrainEngine.GetRegisteredAssets();
                var newAssets = registeredAssets.Slice(prevAssetCount);

                foreach (var asset in newAssets)
                {
                    var assetVm = new AssetViewModel
                    {
                        AssetId = asset.Id,
                        RegistrationType = asset.RegistrationType,
                        AssetType = asset.GetAssetType()
                    };
                    if (assetVm.RegistrationType == AssetRegistrationType.File)
                    {
                        var fileState = asset.GetFileState();
                        assetVm.FileRelativePath = fileState.RelativePath;
                    }
                    RegisteredAssets.Add(assetVm);
                }

                prevAssetCount = assetCount;
            }
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