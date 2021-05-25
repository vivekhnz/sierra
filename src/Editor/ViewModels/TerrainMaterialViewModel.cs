using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor.ViewModels
{
    public class TerrainMaterialViewModel : ViewModelBase
    {
        private int index;
        public int Index { get => index; set => SetAndNotify(ref index, value); }

        private string name;
        public string Name { get => name; set => SetAndNotify(ref name, value); }

        private uint albedoTextureAssetId;
        public uint AlbedoTextureAssetId
        {
            get => albedoTextureAssetId;
            set => SetAndNotify(ref albedoTextureAssetId, value);
        }

        private uint normalTextureAssetId;
        public uint NormalTextureAssetId
        {
            get => normalTextureAssetId;
            set => SetAndNotify(ref normalTextureAssetId, value);
        }

        private uint displacementTextureAssetId;
        public uint DisplacementTextureAssetId
        {
            get => displacementTextureAssetId;
            set => SetAndNotify(ref displacementTextureAssetId, value);
        }

        private uint aoTextureAssetId;
        public uint AoTextureAssetId
        {
            get => aoTextureAssetId;
            set => SetAndNotify(ref aoTextureAssetId, value);
        }

        private float textureSizeInWorldUnits;
        public float TextureSizeInWorldUnits
        {
            get => textureSizeInWorldUnits;
            set
            {
                SetAndNotify(ref textureSizeInWorldUnits, value);
                UpdateMaterialProperties();
            }
        }

        private float slopeStart;
        public float SlopeStart
        {
            get => slopeStart;
            set
            {
                SetAndNotify(ref slopeStart, value);
                UpdateMaterialProperties();
            }
        }

        private float slopeEnd;
        public float SlopeEnd
        {
            get => slopeEnd;
            set
            {
                SetAndNotify(ref slopeEnd, value);
                UpdateMaterialProperties();
            }
        }

        private float altitudeStart;
        public float AltitudeStart
        {
            get => altitudeStart;
            set
            {
                SetAndNotify(ref altitudeStart, value);
                UpdateMaterialProperties();
            }
        }

        private float altitudeEnd;
        public float AltitudeEnd
        {
            get => altitudeEnd;
            set
            {
                SetAndNotify(ref altitudeEnd, value);
                UpdateMaterialProperties();
            }
        }

        public DelegateCommand MoveMaterialUpCommand { get; private set; }
        public DelegateCommand MoveMaterialDownCommand { get; private set; }
        public DelegateCommand DeleteMaterialCommand { get; private set; }

        public TerrainMaterialViewModel(
            DelegateCommandFactory<TerrainMaterialViewModel> moveMaterialUpCommandFactory,
            DelegateCommandFactory<TerrainMaterialViewModel> moveMaterialDownCommandFactory,
            DelegateCommandFactory<TerrainMaterialViewModel> deleteMaterialCommandFactory)
        {
            MoveMaterialUpCommand = moveMaterialUpCommandFactory.Create(this);
            MoveMaterialDownCommand = moveMaterialDownCommandFactory.Create(this);
            DeleteMaterialCommand = deleteMaterialCommandFactory.Create(this);
        }

        private void UpdateMaterialProperties()
        {
            if (Index == -1) return;

            EditorCore.SetMaterialProperties(Index,
                textureSizeInWorldUnits, slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }
    }
}