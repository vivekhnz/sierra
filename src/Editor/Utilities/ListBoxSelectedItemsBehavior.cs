using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using Microsoft.Xaml.Behaviors;

namespace Sierra.Utilities
{
    public class ListBoxSelectedItemsBehavior : Behavior<ListBox>
    {
        private bool isUpdatingItemsFromSource;

        public ObservableCollection<object> SelectedItems
        {
            get { return (ObservableCollection<object>)GetValue(SelectedItemsProperty); }
            set { SetValue(SelectedItemsProperty, value); }
        }
        public static readonly DependencyProperty SelectedItemsProperty =
            DependencyProperty.Register(nameof(SelectedItems),
                typeof(ObservableCollection<object>),
                typeof(ListBoxSelectedItemsBehavior),
                new FrameworkPropertyMetadata(new ObservableCollection<object>())
                {
                    BindsTwoWayByDefault = true,
                    PropertyChangedCallback = OnSelectedItemsChanged
                });

        private static void OnSelectedItemsChanged(
            DependencyObject d, DependencyPropertyChangedEventArgs e)
            => (d as ListBoxSelectedItemsBehavior)?.OnSelectedItemsChanged(e);

        private void OnSelectedItemsChanged(DependencyPropertyChangedEventArgs e)
        {
            if (e.OldValue is ObservableCollection<object> oldCollection)
            {
                oldCollection.CollectionChanged -= OnSelectedItemsCollectionChanged;
            }
            if (e.NewValue is ObservableCollection<object> newCollection)
            {
                newCollection.CollectionChanged += OnSelectedItemsCollectionChanged;
            }
        }

        public ListBoxSelectedItemsBehavior()
        {
            SelectedItems.CollectionChanged += OnSelectedItemsCollectionChanged;
        }

        protected override void OnAttached()
        {
            base.OnAttached();

            if (this.AssociatedObject != null)
            {
                this.AssociatedObject.SelectionChanged += OnSelectionChanged;
            }
        }
        protected override void OnDetaching()
        {
            base.OnDetaching();
            if (this.AssociatedObject != null)
            {
                this.AssociatedObject.SelectionChanged -= OnSelectionChanged;
            }
        }

        private void OnSelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (isUpdatingItemsFromSource) return;

            if (e.AddedItems != null)
            {
                foreach (var item in e.AddedItems)
                {
                    this.SelectedItems.Add(item);
                }
            }
            if (e.RemovedItems != null)
            {
                foreach (var item in e.RemovedItems)
                {
                    this.SelectedItems.Remove(item);
                }
            }
        }

        private void OnSelectedItemsCollectionChanged(object sender,
            NotifyCollectionChangedEventArgs e)
        {
            isUpdatingItemsFromSource = true;
            if (this.AssociatedObject != null)
            {
                switch (e.Action)
                {
                    case NotifyCollectionChangedAction.Add:
                        if (e.NewItems != null)
                        {
                            foreach (var item in e.NewItems)
                            {
                                AssociatedObject.SelectedItems.Add(item);
                            }
                        }
                        break;
                    case NotifyCollectionChangedAction.Remove:
                        if (e.OldItems != null)
                        {
                            foreach (var item in e.OldItems)
                            {
                                AssociatedObject.SelectedItems.Remove(item);
                            }
                        }
                        break;
                    case NotifyCollectionChangedAction.Reset:
                        AssociatedObject.SelectedItems.Clear();
                        break;
                }
            }
            isUpdatingItemsFromSource = false;
        }
    }
}