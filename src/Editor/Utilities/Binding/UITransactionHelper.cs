using System;
using System.Windows;
using System.Windows.Controls.Primitives;

namespace Terrain.Editor.Utilities.Binding
{
    internal static class UITransactionHelper
    {
        internal static void RegisterTransactionEventHandlers(
            DependencyObject targetObject, DependencyProperty targetProperty,
            Action beginTransaction, Action commitTransaction)
        {
            if (targetObject is RangeBase rangeBase
                && targetProperty == RangeBase.ValueProperty)
            {
                rangeBase.AddHandler(Thumb.DragStartedEvent,
                    new DragStartedEventHandler((sender, e) => beginTransaction()));
                rangeBase.AddHandler(Thumb.DragCompletedEvent,
                    new DragCompletedEventHandler((sender, e) => commitTransaction()));
            }
        }
    }
}