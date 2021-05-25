using System;
using System.Collections.Generic;
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

    public class DelegateCommandFactory<T>
    {
        Action<T> execute;
        Func<T, bool> canExecute;

        private readonly List<DelegateCommand> commands = new List<DelegateCommand>();

        public DelegateCommandFactory(Action<T> execute, Func<T, bool> canExecute = null)
        {
            this.execute = execute;
            this.canExecute = canExecute;
        }

        public DelegateCommand Create(T parameter)
        {
            var cmd = new DelegateCommand(() => execute(parameter), () => canExecute(parameter));
            commands.Add(cmd);
            return cmd;
        }

        internal void NotifyCanExecuteChanged()
            => commands.ForEach(cmd => cmd.NotifyCanExecuteChanged());
    }
}