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
            if (EditorCore.BeginTransaction(out var tx))
            {
                EditorCore.AddObject(tx);
                EditorCore.CommitTransaction(tx);
            }
        });
        public readonly static ActionCommand DeleteSelectedObject = new ActionCommand(() =>
        {
            ref EditorUiState uiState = ref EditorCore.GetUiState();
            if (uiState.SelectedObjectCount > 0)
            {
                if (EditorCore.BeginTransaction(out var tx))
                {
                    for (int i = 0; i < uiState.SelectedObjectCount; i++)
                    {
                        EditorCore.DeleteObject(tx, uiState.SelectedObjectIds[i]);
                    }
                    EditorCore.CommitTransaction(tx);
                }
            }
        });

        internal static void Update(EditorDocumentViewModel doc, ref EditorUiState uiState)
        {
            AddObject.UpdateCanExecute(doc.ObjectIds.Count < maxObjectCount);
            DeleteSelectedObject.UpdateCanExecute(uiState.SelectedObjectCount > 0);
        }
    }
}