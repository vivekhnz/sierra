using Terrain.Editor.Core;
using Terrain.Editor.Utilities;
using Terrain.Editor.ViewModels;

namespace Terrain.Editor
{
    public static class EditorCommands
    {
        private const int maxObjectCount = 32;

        public readonly static ActionCommand AddObject = new ActionCommand(() =>
        {
            EditorCore.AddObject();
        });
        public readonly static ActionCommand DeleteSelectedObject = new ActionCommand(() =>
        {
            ref EditorUiState uiState = ref EditorCore.GetUiState();
            EditorCore.DeleteObject(uiState.SelectedObjectId);
        });

        internal static void Update(EditorDocumentViewModel doc, ref EditorUiState uiState)
        {
            AddObject.UpdateCanExecute(doc.ObjectIds.Count < maxObjectCount);
            DeleteSelectedObject.UpdateCanExecute(uiState.SelectedObjectId != 0U);
        }
    }
}