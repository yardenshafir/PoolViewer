using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using Microsoft.Win32;

namespace PoolViewer
{
    /// <summary>
    /// Interaction logic for ChildWindow.xaml
    /// </summary>
    public partial class ChildWindow : Window
    {
        public event Action<string> Notify1;
        public event Action<string> Notify2;
        public event EventHandler ButtonClicked;
        public ChildWindow()
        {
            InitializeComponent();
        }
        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (livekd.Text == "")
            {
                MessageBox.Show("No path for livekd.exe was supplied!");
                return;
            }
            Notify1(livekd.Text);
            Notify2(dmp.Text);
            var handler = ButtonClicked;
            if (handler != null)
                handler(this, e);
        }

        private void Button_Click1(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                livekd.Text = openFileDialog.FileName.ToString();
            }
        }
    }
}
