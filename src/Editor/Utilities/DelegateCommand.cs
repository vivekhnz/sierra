using System;
using System.Windows.Input;

namespace Terrain.Editor.Utilities
{
    public class DelegateCommand : ICommand
    {
        Action execute;
        Func<bool> canExecute;

        public event EventHandler CanExecuteChanged;

        public DelegateCommand(Action execute, Func<bool> canExecute = null)
        {
            this.execute = execute;
            this.canExecute = canExecute;
        }

        public bool CanExecute(object parameter) => canExecute?.Invoke() ?? true;
        public void Execute(object parameter) => execute();

        internal void NotifyCanExecuteChanged() => CanExecuteChanged?.Invoke(null, null);
    }
}