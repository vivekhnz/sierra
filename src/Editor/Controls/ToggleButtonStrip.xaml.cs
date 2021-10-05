using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;

namespace Sierra.Controls
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

        private Style firstToggleButtonStyle;
        private Style middleToggleButtonStyle;
        private Style lastToggleButtonStyle;

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

            firstToggleButtonStyle = FindResource("FirstToggleButtonStyle") as Style;
            middleToggleButtonStyle = FindResource("ToggleButtonStyle") as Style;
            lastToggleButtonStyle = FindResource("LastToggleButtonStyle") as Style;
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

            for (int i = 0; i < Options.Count; i++)
            {
                var option = Options[i];
                var radioButton = new RadioButton
                {
                    IsChecked = false,
                    DataContext = option,
                    Style = i == 0
                        ? firstToggleButtonStyle
                        : (i == Options.Count - 1 ? lastToggleButtonStyle : middleToggleButtonStyle)
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
