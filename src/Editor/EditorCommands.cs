using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor
{
    public static class EditorCommands
    {
        private const int maxObjectCount = 32;

        public static DelegateCommand AddObject { get; private set; } = new DelegateCommand(
            () => EditorCore.AddObject(),
            () => (App.Current?.Document?.ObjectIds?.Count ?? 0) < maxObjectCount);

        public static DelegateCommand DeletedSelectedObject { get; private set; } = new DelegateCommand(
            () =>
            {
                ref EditorUiState state = ref EditorCore.GetUiState();
                EditorCore.DeleteObject(state.SelectedObjectId);
            },
            () =>
            {
                ref EditorUiState state = ref EditorCore.GetUiState();
                return state.SelectedObjectId != 0U;
            });

        public static void Update()
        {
            AddObject.NotifyCanExecuteChanged();
            DeletedSelectedObject.NotifyCanExecuteChanged();
        }
    }
}