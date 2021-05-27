using System;
using System.Windows;
using System.Windows.Threading;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;
using Terrain.Editor.ViewModels;

namespace Terrain.Editor
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private DispatcherTimer updateUiTimer;

        public EditorUiStateViewModel UiState { get; private set; }
        public EditorAssetsViewModel Assets { get; private set; }
        public EditorDocumentViewModel Document { get; private set; }

        public new static App Current => Application.Current as App;

        protected override void OnStartup(StartupEventArgs e)
        {
            UiState = (EditorUiStateViewModel)FindResource("EditorUiState");
            Assets = (EditorAssetsViewModel)FindResource("EditorAssets");
            Document = (EditorDocumentViewModel)FindResource("EditorDocument");

            TerrainEngine.AssetRegistered += Assets.OnAssetRegistered;
            EditorCore.TransactionPublished += Document.OnTransactionPublished;

            updateUiTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(50)
            };
            updateUiTimer.Tick += updateUiTimer_Tick;
            updateUiTimer.Start();

            base.OnStartup(e);
        }

        private void updateUiTimer_Tick(object sender, EventArgs e)
        {
            UiState.CheckForChanges();
        }
    }
}
