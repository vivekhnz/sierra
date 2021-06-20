using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Shapes;

namespace Terrain.Editor.Controls
{
    /// <summary>
    /// Interaction logic for ToggleButtonStrip.xaml
    /// </summary>
    [ContentProperty(nameof(Options))]
    public partial class ToggleButtonStrip : UserControl
    {
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ObservableCollection<ToggleButtonStripItem> Options { get; set; }
            = new ObservableCollection<ToggleButtonStripItem>();

        private Dictionary<ToggleButtonStripItem, RadioButton> optionToButtonMap
            = new Dictionary<ToggleButtonStripItem, RadioButton>();
        private Dictionary<RadioButton, ToggleButtonStripItem> buttonToOptionMap
            = new Dictionary<RadioButton, ToggleButtonStripItem>();
        private bool isSettingValueFromSource;
        private bool isSettingValueFromUi;

        public object SelectedValue
        {
            get { return GetValue(SelectedValueProperty); }
            set { SetValue(SelectedValueProperty, value); }
        }
        public static readonly DependencyProperty SelectedValueProperty =
            DependencyProperty.Register(nameof(SelectedValue), typeof(object),
                typeof(ToggleButtonStrip), new FrameworkPropertyMetadata
                {
                    DefaultValue = null,
                    BindsTwoWayByDefault = true,
                    PropertyChangedCallback = OnSelectedValueChanged
                });

        private static void OnSelectedValueChanged(
            DependencyObject d, DependencyPropertyChangedEventArgs e)
            => (d as ToggleButtonStrip)?.OnSelectedValueChanged(e.NewValue);

        public ToggleButtonStrip()
        {
            InitializeComponent();

            Options.CollectionChanged += OnOptionsChanged;
        }

        private void OnOptionsChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            foreach (var button in optionToButtonMap.Values)
            {
                button.Checked -= OnRadioButtonValueChanged;
                button.Unchecked -= OnRadioButtonValueChanged;
            }
            spOptions.Children.Clear();
            optionToButtonMap.Clear();
            buttonToOptionMap.Clear();

            var backgroundBrush = new SolidColorBrush(Color.FromArgb(96, 255, 255, 255));
            var foregroundBrush = new SolidColorBrush(Color.FromArgb(255, 48, 48, 48));

            for (int i = 0; i < Options.Count; i++)
            {
                var option = Options[i];
                var contentStackPanel = new StackPanel
                {
                    Orientation = Orientation.Vertical
                };
                contentStackPanel.Children.Add(new Viewbox
                {
                    Height = 20,
                    Margin = new Thickness(0, 2, 0, 2),
                    Child = new Path
                    {
                        Data = option.Icon,
                        Fill = foregroundBrush
                    }
                });
                contentStackPanel.Children.Add(new TextBlock
                {
                    Text = option.Text,
                    HorizontalAlignment = HorizontalAlignment.Center,
                    FontSize = 11
                });
                var radioButton = new RadioButton
                {
                    IsChecked = false,
                    Padding = new Thickness(4),
                    Width = 64,
                    Height = 48,
                    Background = backgroundBrush,
                    Margin = new Thickness(0, 0, i == Options.Count - 1 ? 0 : 2, 0),
                    Content = contentStackPanel,
                    Style = FindResource(typeof(ToggleButton)) as Style
                };
                radioButton.Checked += OnRadioButtonValueChanged;
                radioButton.Unchecked += OnRadioButtonValueChanged;
                spOptions.Children.Add(radioButton);

                optionToButtonMap[option] = radioButton;
                buttonToOptionMap[radioButton] = option;
            }
        }

        private void OnSelectedValueChanged(object newValue)
        {
            if (isSettingValueFromUi) return;

            isSettingValueFromSource = true;

            bool foundButton = false;
            foreach (var option in Options)
            {
                if (option.Value.Equals(newValue) && optionToButtonMap.TryGetValue(option, out var button))
                {
                    button.IsChecked = true;
                    foundButton = true;
                }
            }
            if (!foundButton)
            {
                foreach (var button in optionToButtonMap.Values)
                {
                    button.IsChecked = false;
                }
            }

            isSettingValueFromSource = false;
        }

        private void OnRadioButtonValueChanged(object sender, RoutedEventArgs e)
        {
            if (isSettingValueFromSource) return;

            if (sender is RadioButton button && button.IsChecked == true
                && buttonToOptionMap.TryGetValue(button, out var option))
            {
                isSettingValueFromUi = true;
                SelectedValue = option.Value;
                isSettingValueFromUi = false;
            }
        }
    }
}
