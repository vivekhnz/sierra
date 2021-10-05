using System;
using System.Windows.Input;

namespace Sierra.Utilities
{
    public class ActionCommand : ICommand
    {
        Action execute;
        bool canExecuteValue;

        public event EventHandler CanExecuteChanged;

        public ActionCommand(Action execute)
        {
            this.execute = execute;
            this.canExecuteValue = false;
        }

        public bool CanExecute(object parameter) => canExecuteValue;
        public void Execute(object parameter) => execute();

        internal void UpdateCanExecute(bool canExecute)
        {
            this.canExecuteValue = canExecute;
            CanExecuteChanged?.Invoke(null, null);
        }
    }
}