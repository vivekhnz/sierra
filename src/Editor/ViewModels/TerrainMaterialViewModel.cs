using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor.ViewModels
{
    public class TerrainMaterialViewModel : ViewModelBase
    {
        private readonly uint materialId;

        private string name;
        public string Name { get => name; set => SetAndNotify(ref name, value); }

        private AssetViewModel albedoTexture;
        public AssetViewModel AlbedoTexture
        {
            get => albedoTexture;
            set
            {
                SetAndNotify(ref albedoTexture, value);
                EditorCore.SetMaterialTexture(materialId,
                    TerrainMaterialTextureType.Albedo, value.AssetId);
            }
        }

        private AssetViewModel normalTexture;
        public AssetViewModel NormalTexture
        {
            get => normalTexture;
            set
            {
                SetAndNotify(ref normalTexture, value);
                EditorCore.SetMaterialTexture(materialId,
                    TerrainMaterialTextureType.Normal, value.AssetId);
            }
        }

        private AssetViewModel displacementTexture;
        public AssetViewModel DisplacementTexture
        {
            get => displacementTexture;
            set
            {
                SetAndNotify(ref displacementTexture, value);
                EditorCore.SetMaterialTexture(materialId,
                    TerrainMaterialTextureType.Displacement, value.AssetId);
            }
        }

        private AssetViewModel aoTexture;
        public AssetViewModel AoTexture
        {
            get => aoTexture;
            set
            {
                SetAndNotify(ref aoTexture, value);
                EditorCore.SetMaterialTexture(materialId,
                    TerrainMaterialTextureType.AmbientOcclusion, value.AssetId);
            }
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

        private bool canSetMaterialProperties;
        public bool CanSetMaterialProperties
        {
            get => canSetMaterialProperties;
            set => SetAndNotify(ref canSetMaterialProperties, value);
        }

        public DelegateCommand MoveMaterialUpCommand { get; private set; }
        public DelegateCommand MoveMaterialDownCommand { get; private set; }
        public DelegateCommand DeleteMaterialCommand { get; private set; }

        public TerrainMaterialViewModel(
            uint materialId,
            DelegateCommandFactory<TerrainMaterialViewModel> moveMaterialUpCommandFactory,
            DelegateCommandFactory<TerrainMaterialViewModel> moveMaterialDownCommandFactory,
            DelegateCommandFactory<TerrainMaterialViewModel> deleteMaterialCommandFactory)
        {
            this.materialId = materialId;
            MoveMaterialUpCommand = moveMaterialUpCommandFactory.Create(this);
            MoveMaterialDownCommand = moveMaterialDownCommandFactory.Create(this);
            DeleteMaterialCommand = deleteMaterialCommandFactory.Create(this);
        }

        private void UpdateMaterialProperties()
        {
            EditorCore.SetMaterialProperties(materialId,
                textureSizeInWorldUnits, slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }
    }
}