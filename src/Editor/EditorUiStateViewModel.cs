using System.ComponentModel;
using Terrain.Editor.Core;

namespace Terrain.Editor
{
    public class EditorUiStateViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private EditorUiState lastReadState;

        public TerrainBrushTool TerrainBrushTool
        {
            get
            {
                lastReadState.TerrainBrushTool = EditorCore.GetUiState().TerrainBrushTool;
                return lastReadState.TerrainBrushTool;
            }
            set
            {
                EditorCore.GetUiState().TerrainBrushTool = value;
                lastReadState.TerrainBrushTool = value;
                NotifyPropertyChanged(nameof(TerrainBrushTool));
            }
        }
        public float TerrainBrushRadius
        {
            get
            {
                lastReadState.TerrainBrushRadius = EditorCore.GetUiState().TerrainBrushRadius;
                return lastReadState.TerrainBrushRadius;
            }
            set
            {
                EditorCore.GetUiState().TerrainBrushRadius = value;
                lastReadState.TerrainBrushRadius = value;
                NotifyPropertyChanged(nameof(TerrainBrushRadius));
            }
        }
        public float TerrainBrushFalloff
        {
            get
            {
                lastReadState.TerrainBrushFalloff = EditorCore.GetUiState().TerrainBrushFalloff;
                return lastReadState.TerrainBrushFalloff;
            }
            set
            {
                EditorCore.GetUiState().TerrainBrushFalloff = value;
                lastReadState.TerrainBrushFalloff = value;
                NotifyPropertyChanged(nameof(TerrainBrushFalloff));
            }
        }
        public float TerrainBrushStrength
        {
            get
            {
                lastReadState.TerrainBrushStrength = EditorCore.GetUiState().TerrainBrushStrength;
                return lastReadState.TerrainBrushStrength;
            }
            set
            {
                EditorCore.GetUiState().TerrainBrushStrength = value;
                lastReadState.TerrainBrushStrength = value;
                NotifyPropertyChanged(nameof(TerrainBrushStrength));
            }
        }
        public float SceneLightDirection
        {
            get
            {
                lastReadState.SceneLightDirection = EditorCore.GetUiState().SceneLightDirection;
                return lastReadState.SceneLightDirection;
            }
            set
            {
                EditorCore.GetUiState().SceneLightDirection = value;
                lastReadState.SceneLightDirection = value;
                NotifyPropertyChanged(nameof(SceneLightDirection));
            }
        }

        public void CheckForChanges()
        {
            ref EditorUiState state = ref EditorCore.GetUiState();

            if (state.TerrainBrushTool != lastReadState.TerrainBrushTool)
            {
                lastReadState.TerrainBrushTool = state.TerrainBrushTool;
                NotifyPropertyChanged(nameof(TerrainBrushTool));
            }
            if (state.TerrainBrushRadius != lastReadState.TerrainBrushRadius)
            {
                lastReadState.TerrainBrushRadius = state.TerrainBrushRadius;
                NotifyPropertyChanged(nameof(TerrainBrushRadius));
            }
            if (state.TerrainBrushFalloff != lastReadState.TerrainBrushFalloff)
            {
                lastReadState.TerrainBrushFalloff = state.TerrainBrushFalloff;
                NotifyPropertyChanged(nameof(TerrainBrushFalloff));
            }
            if (state.TerrainBrushStrength != lastReadState.TerrainBrushStrength)
            {
                lastReadState.TerrainBrushStrength = state.TerrainBrushStrength;
                NotifyPropertyChanged(nameof(TerrainBrushStrength));
            }
            if (state.SceneLightDirection != lastReadState.SceneLightDirection)
            {
                lastReadState.SceneLightDirection = state.SceneLightDirection;
                NotifyPropertyChanged(nameof(SceneLightDirection));
            }
        }
    }
}