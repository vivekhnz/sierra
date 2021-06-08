using System;
using System.Windows;
using System.Windows.Threading;
using Terrain.Editor.Core;
using Terrain.Editor.Engine;
using Terrain.Editor.Platform;
using Terrain.Editor.ViewModels;

namespace Terrain.Editor
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private DispatcherTimer renderTimer;

        public EditorUiStateViewModel UiState { get; private set; }
        public EditorAssetsViewModel Assets { get; private set; }
        public EditorDocumentViewModel Document { get; private set; }

        public new static App Current => Application.Current as App;

        internal delegate void TickEventHandler();
        internal static event TickEventHandler CoreTick;

        protected override void OnStartup(StartupEventArgs e)
        {
            UiState = (EditorUiStateViewModel)FindResource("EditorUiState");
            Assets = (EditorAssetsViewModel)FindResource("EditorAssets");
            Document = (EditorDocumentViewModel)FindResource("EditorDocument");

            TerrainEngine.AssetRegistered += Assets.OnAssetRegistered;
            EditorCore.TransactionPublished += Document.OnTransactionPublished;

            EditorPlatform.Initialize();

            renderTimer = new DispatcherTimer(DispatcherPriority.Send)
            {
                Interval = TimeSpan.FromMilliseconds(1)
            };
            renderTimer.Tick += OnTick;
            renderTimer.Start();

            base.OnStartup(e);
        }

        protected override void OnExit(ExitEventArgs e)
        {
            renderTimer.Stop();
            EditorPlatform.Shutdown();

            base.OnExit(e);
        }

        private void OnTick(object sender, EventArgs e)
        {
            UiState.CheckForChanges();
            EditorPlatform.Tick();
            CoreTick?.Invoke();
        }
    }
}
