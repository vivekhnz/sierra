using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace Sierra.Utilities.Binding
{
    internal static class UiTransactionHelper
    {
        internal static void RegisterTransactionEventHandlers(
            DependencyObject targetObject, DependencyProperty targetProperty,
            Action beginTransaction, Action commitTransaction, Action discardTransaction)
        {
            if (targetObject is Slider slider && targetProperty == RangeBase.ValueProperty)
            {
                slider.AddHandler(Thumb.DragStartedEvent,
                    new DragStartedEventHandler((sender, e) => beginTransaction()));
                slider.AddHandler(Thumb.DragCompletedEvent,
                    new DragCompletedEventHandler((sender, e) => commitTransaction()));
                slider.KeyDown += (sender, args) =>
                {
                    if (args.Key == Key.Escape)
                    {
                        var track = slider.Template.FindName("PART_Track", slider) as Track;
                        var thumb = track?.Thumb;
                        if (thumb != null)
                        {
                            discardTransaction();
                            thumb.CancelDrag();
                        }
                    }
                };
            }
        }
    }
}