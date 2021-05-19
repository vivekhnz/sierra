using Terrain.Editor.Core;

namespace Terrain.Editor.ViewModels
{
    public class TerrainMaterialViewModel : ViewModelBase
    {
        private int index;
        public int Index
        {
            get => index;
            set
            {
                index = value;
                NotifyPropertyChanged(nameof(Index));
            }
        }

        private string name;
        public string Name
        {
            get => name;
            set
            {
                name = value;
                NotifyPropertyChanged(nameof(Name));
            }
        }

        private uint albedoTextureAssetId;
        public uint AlbedoTextureAssetId
        {
            get => albedoTextureAssetId;
            set
            {
                albedoTextureAssetId = value;
                NotifyPropertyChanged(nameof(AlbedoTextureAssetId));
            }
        }

        private uint normalTextureAssetId;
        public uint NormalTextureAssetId
        {
            get => normalTextureAssetId;
            set
            {
                normalTextureAssetId = value;
                NotifyPropertyChanged(nameof(NormalTextureAssetId));
            }
        }

        private uint displacementTextureAssetId;
        public uint DisplacementTextureAssetId
        {
            get => displacementTextureAssetId;
            set
            {
                displacementTextureAssetId = value;
                NotifyPropertyChanged(nameof(DisplacementTextureAssetId));
            }
        }

        private uint aoTextureAssetId;
        public uint AoTextureAssetId
        {
            get => aoTextureAssetId;
            set
            {
                aoTextureAssetId = value;
                NotifyPropertyChanged(nameof(AoTextureAssetId));
            }
        }

        private float textureSizeInWorldUnits;
        public float TextureSizeInWorldUnits
        {
            get => textureSizeInWorldUnits;
            set
            {
                textureSizeInWorldUnits = value;
                NotifyPropertyChanged(nameof(TextureSizeInWorldUnits));
                UpdateMaterialProperties();
            }
        }

        private float slopeStart;
        public float SlopeStart
        {
            get => slopeStart;
            set
            {
                slopeStart = value;
                NotifyPropertyChanged(nameof(SlopeStart));
                UpdateMaterialProperties();
            }
        }

        private float slopeEnd;
        public float SlopeEnd
        {
            get => slopeEnd;
            set
            {
                slopeEnd = value;
                NotifyPropertyChanged(nameof(SlopeEnd));
                UpdateMaterialProperties();
            }
        }

        private float altitudeStart;
        public float AltitudeStart
        {
            get => altitudeStart;
            set
            {
                altitudeStart = value;
                NotifyPropertyChanged(nameof(AltitudeStart));
                UpdateMaterialProperties();
            }
        }

        private float altitudeEnd;
        public float AltitudeEnd
        {
            get => altitudeEnd;
            set
            {
                altitudeEnd = value;
                NotifyPropertyChanged(nameof(AltitudeEnd));
                UpdateMaterialProperties();
            }
        }

        private void UpdateMaterialProperties()
        {
            if (Index == -1) return;

            EditorCore.SetMaterialProperties(Index,
                textureSizeInWorldUnits, slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }
    }
}