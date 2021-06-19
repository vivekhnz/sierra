using Terrain.Editor.Core;
using Terrain.Editor.Utilities.Binding;

namespace Terrain.Editor.ViewModels
{
    public class EditorUiStateViewModel : ViewModelBase
    {
        private EditorUiState lastReadState;

        public uint SelectedObjectId
        {
            get
            {
                lastReadState.SelectedObjectId = EditorCore.GetUiState().SelectedObjectId;
                return lastReadState.SelectedObjectId;
            }
            set
            {
                EditorCore.GetUiState().SelectedObjectId = value;
                lastReadState.SelectedObjectId = value;
                NotifyPropertyChanged(nameof(SelectedObjectId));
            }
        }
        public ObjectReference SelectedObject { get; }

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

        public EditorUiStateViewModel()
        {
            SelectedObject = new ObjectReference(() => SelectedObjectId);
        }

        public void CheckForChanges()
        {
            ref EditorUiState state = ref EditorCore.GetUiState();

            if (state.SelectedObjectId != lastReadState.SelectedObjectId)
            {
                lastReadState.SelectedObjectId = state.SelectedObjectId;
                NotifyPropertyChanged(nameof(SelectedObjectId));
            }
            if (state.TerrainBrushTool != lastReadState.TerrainBrushTool)
            {
                lastReadState.TerrainBrushTool = state.TerrainBrushTool;
                NotifyPropertyChanged(nameof(TerrainBrushTool));
            }
        }
    }
}