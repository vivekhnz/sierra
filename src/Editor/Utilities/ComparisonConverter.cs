using System;
using System.Globalization;
using System.Windows.Data;

namespace Terrain.Editor.Utilities
{
    // based on: https://stackoverflow.com/questions/397556/how-to-bind-radiobuttons-to-an-enum/2908885#2908885

    public class ComparisonConverter : IValueConverter
    {
        public bool Invert { get; set; }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
            => value?.Equals(parameter) != Invert;

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
            => value?.Equals(true) != Invert ? parameter : Binding.DoNothing;
    }
}