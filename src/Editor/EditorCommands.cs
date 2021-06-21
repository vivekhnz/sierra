using Terrain.Editor.Core;
using Terrain.Editor.Utilities;

namespace Terrain.Editor
{
    public static class EditorCommands
    {
        private const int maxObjectCount = 32;

        public static DelegateCommand AddObject { get; private set; } = new DelegateCommand(
            () => EditorCore.AddObject(),
            () => (App.Current?.Document?.Objects?.Count ?? 0) < maxObjectCount);

        public static DelegateCommand DeletedSelectedObject { get; private set; } = new DelegateCommand(
            () => EditorCore.DeleteObject(App.Current.UiState.SelectedObjectId),
            () => (App.Current?.UiState?.SelectedObjectId ?? 0U) != 0U);

        public static void Update()
        {
            AddObject.NotifyCanExecuteChanged();
            DeletedSelectedObject.NotifyCanExecuteChanged();
        }
    }
}