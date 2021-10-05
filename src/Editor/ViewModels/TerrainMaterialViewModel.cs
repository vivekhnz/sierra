using System.Collections.ObjectModel;
using Sierra.Core;
using Sierra.Utilities;

namespace Sierra.ViewModels
{
    public class TerrainMaterialViewModel : ViewModelBase
    {
        public uint MaterialId { get; private set; }

        private string name;
        public string Name { get => name; set => SetAndNotify(ref name, value); }

        private AssetViewModel albedoTexture;
        public AssetViewModel AlbedoTexture
        {
            get => albedoTexture;
            set
            {
                SetAndNotify(ref albedoTexture, value);
                EditorCore.SetMaterialTexture(MaterialId,
                    TerrainMaterialTextureType.Albedo, value.AssetHandle);
            }
        }

        private AssetViewModel normalTexture;
        public AssetViewModel NormalTexture
        {
            get => normalTexture;
            set
            {
                SetAndNotify(ref normalTexture, value);
                EditorCore.SetMaterialTexture(MaterialId,
                    TerrainMaterialTextureType.Normal, value.AssetHandle);
            }
        }

        private AssetViewModel displacementTexture;
        public AssetViewModel DisplacementTexture
        {
            get => displacementTexture;
            set
            {
                SetAndNotify(ref displacementTexture, value);
                EditorCore.SetMaterialTexture(MaterialId,
                    TerrainMaterialTextureType.Displacement, value.AssetHandle);
            }
        }

        private AssetViewModel aoTexture;
        public AssetViewModel AoTexture
        {
            get => aoTexture;
            set
            {
                SetAndNotify(ref aoTexture, value);
                EditorCore.SetMaterialTexture(MaterialId,
                    TerrainMaterialTextureType.AmbientOcclusion, value.AssetHandle);
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

        public TerrainMaterialViewModel(uint materialId,
            ObservableCollection<TerrainMaterialViewModel> materials)
        {
            MaterialId = materialId;

            MoveMaterialUpCommand = new DelegateCommand(
                () =>
                {
                    int index = materials.IndexOf(this);
                    if (index == -1) return;

                    EditorCore.SwapMaterial(index, index - 1);
                },
                () => materials.IndexOf(this) > 0);
            MoveMaterialDownCommand = new DelegateCommand(
                () =>
                {
                    int index = materials.IndexOf(this);
                    if (index == -1) return;

                    EditorCore.SwapMaterial(index, index + 1);
                },
                () => materials.IndexOf(this) < materials.Count - 1);
            DeleteMaterialCommand = new DelegateCommand(
                () =>
                {
                    int index = materials.IndexOf(this);
                    if (index == -1) return;

                    EditorCore.DeleteMaterial(index);
                },
                () => materials.IndexOf(this) != -1);
        }

        private void UpdateMaterialProperties()
        {
            EditorCore.SetMaterialProperties(MaterialId,
                textureSizeInWorldUnits, slopeStart, slopeEnd, altitudeStart, altitudeEnd);
        }
    }
}