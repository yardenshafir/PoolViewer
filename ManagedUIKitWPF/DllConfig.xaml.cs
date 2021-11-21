using System;
using System.Windows;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.IO;
using System.Windows.Documents;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Collections.ObjectModel;
using System.Security.Policy;
using System.ComponentModel;
using System.Windows.Interop;
using System.Security.Principal;
using System.Diagnostics;
using System.Windows.Threading;

namespace PoolViewer
{
    static class NativeMethods
    {
        [DllImport("kernel32.dll")]
        public static extern IntPtr LoadLibrary(string dllToLoad);

        [DllImport("kernel32.dll")]
        public static extern IntPtr GetModuleHandle(string ModuleName);
    }

    /// <summary>
    /// Interaction logic for DllConfig.xaml
    /// </summary>
    public partial class DllConfig : Window
    {
        public DllConfig()
        {
            InitializeComponent();
        }

        private void OK_Button_Click(object sender, RoutedEventArgs e)
        {
            var path = DllSearchPath.Text;
            if (File.Exists($"{path}\\DbgEng.dll") &&
                File.Exists($"{path}\\DbgHelp.dll"))
            {
                IntPtr dbgEng = NativeMethods.GetModuleHandle("DbgEng.dll");
                IntPtr dbgHelp = NativeMethods.GetModuleHandle("DbgHelp.dll");

                if (dbgEng == IntPtr.Zero && dbgHelp == IntPtr.Zero)
                {
                    NativeMethods.LoadLibrary($"{path}\\DbgHelp.dll");
                    NativeMethods.LoadLibrary($"{path}\\DbgEng.dll");
                }

            }
            else
            {
                DllSearchPath.Text = "";
                string message = "Invalid path!";
                MessageBox.Show(message);
            }
            this.Close();
        }
        private void Cancel_Button_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
