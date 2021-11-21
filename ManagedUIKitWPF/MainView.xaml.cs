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
    public partial class MainView : Window
    {
        public List<Heap> heaps = new List<Heap> { };
        public List<PoolBlock> poolBlocks = new List<PoolBlock> { };
        public Dictionary<string, int> tags = new Dictionary<string, int>();
        public Dictionary<string, int> subsegs = new Dictionary<string, int>();

        //// API Delegates
        delegate long GetPoolInformation_Ptr(string FilePath, bool CreateLiveDump, out int res);
        delegate bool GetNextHeapInformation_Ptr(out Int64 Address, out int NodeNumber, out long NumberOfAllocations, out int PoolType, out bool Special);
        delegate string GetNextAllocation_Ptr(out Int64 Address, out int Size, out bool Allocated, out int Type);


        //// Ported Functions
        GetPoolInformation_Ptr GetPoolInformation;
        GetNextHeapInformation_Ptr GetNextHeapInformation;
        GetNextAllocation_Ptr GetNextAllocation;

        public class Heap
        {
            public Int64 Address { get; set; }
            public int NodeNumber { get; set; }
            public string Special { get; set; }
            public string PoolType { get; set; }
            public Int64 NumberOfAllocs { get; set; }
            public override string ToString()
            {
                return this.Address.ToString("X") + "," +
                    this.NodeNumber.ToString() + "," +
                    this.Special + "," +
                    this.PoolType + "," +
                    this.NumberOfAllocs.ToString();
            }
        }
        public class PoolBlock
        {
            public Int64 Address { get; set; }
            public Int64 Heap { get; set; }
            public string PoolType { get; set; }
            public long Size { get; set; }
            public string Tag { get; set; }
            public string Allocated { get; set; }
            public string SubsegType { get; set; }
            public string Special { get; set; }
            public override string ToString()
            {
                return this.Address.ToString("X") + "," +
                    this.Heap.ToString("X") + "," +
                    this.Size.ToString() + "," +
                    this.Tag + "," +
                    this.Allocated + "," +
                    this.SubsegType + "," +
                    this.PoolType + "," +
                    this.Special;
            }
        }

        void PrintLog(string log_input)
        {
            LogView.Text += $"[{DateTime.Now.ToShortTimeString()}] :: {log_input}\r\n";
            Dispatcher.Invoke(() => { LogView.ScrollToEnd(); }, DispatcherPriority.ContextIdle);
        }

        public MainView(IntPtr api_1_ptr, IntPtr api_2_ptr, IntPtr api_3_ptr)
        {
            InitializeComponent();
            PrintLog("Initializing ...");

            //// Recovering Native Functions
            if (api_1_ptr != IntPtr.Zero && api_2_ptr != IntPtr.Zero && api_3_ptr != IntPtr.Zero)
            {
                GetPoolInformation = (GetPoolInformation_Ptr)Marshal.GetDelegateForFunctionPointer(api_1_ptr, typeof(GetPoolInformation_Ptr));
                GetNextHeapInformation = (GetNextHeapInformation_Ptr)Marshal.GetDelegateForFunctionPointer(api_2_ptr, typeof(GetNextHeapInformation_Ptr));
                GetNextAllocation = (GetNextAllocation_Ptr)Marshal.GetDelegateForFunctionPointer(api_3_ptr, typeof(GetNextAllocation_Ptr));
            }
            PrintLog("Successfully initialized");
        }

        private void FilterPoolBlocks()
        {
            var _itemSourceList = new CollectionViewSource() { Source = poolBlocks };
            _itemSourceList.Filter += new FilterEventHandler(DataFilter);
            ICollectionView Itemlist = _itemSourceList.View;
            Blocks.ItemsSource = Itemlist;
        }
        private void Search_Button_Click(object sender, RoutedEventArgs e)
        {
            FilterPoolBlocks();
        }
        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {
            // Get the clicked MenuItem
            var menuItem = (MenuItem)sender;

            // Get the ContextMenu to which the menuItem belongs
            var contextMenu = (ContextMenu)menuItem.Parent;

            // Find the placementTarget
            var item = (DataGrid)contextMenu.PlacementTarget;

            // Get the underlying item, that you cast to your object that is bound
            // to the DataGrid (and has subject and state as property)
            var heap = (Heap)item.SelectedCells[0].Item;

            HeapAddress.Text = heap.Address.ToString("X");
            PoolType.SelectedIndex = -1;
            MinSize.Clear();
            MaxSize.Clear();
            Allocated.SelectedIndex = -1;
            Special.SelectedIndex = -1;
            Subseg.SelectedIndex = -1;
            Tag.Clear();
            FilterPoolBlocks();
            if (Pools.SelectedIndex == 0)
            {
                Pools.SelectedIndex++;
            }
        }

        private void MenuItem_Click_1(object sender, RoutedEventArgs e)
        {
            System.Text.StringBuilder sb = new System.Text.StringBuilder();
            sb.AppendLine("Address,Heap,Size,Tag,Allocated,Subsegment,Pool Type,Special Pool");
            foreach (var item in poolBlocks)
            {
                sb.AppendLine(item.ToString());
            }
            System.IO.File.WriteAllText(
                System.IO.Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory, "export.csv"),
                sb.ToString());
            PrintLog($"Exported pool blocks to {System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "export.csv")}.");
        }

        private void DisplayInfoFromDmp(int NumberOfHeaps)
        {
            long address;
            int nodeNum;
            long allocNum;
            int poolType;
            bool special;
            string tag;
            bool allocated;
            int allocSize;
            long blockAddress;
            int allocType;
            string poolTypeStr = "Unknown";
            string specialStr = "No";

            heaps.Clear();
            poolBlocks.Clear();
            subsegs.Clear();
            tags.Clear();

            subsegs.Add("Lfh", 0);
            subsegs.Add("Vs", 0);
            subsegs.Add("Large", 0);
            subsegs.Add("Big", 0);
            subsegs.Add("Unknown", 0);

            for (int i = 0; i < NumberOfHeaps; i++)
            {
                GetNextHeapInformation(out address, out nodeNum, out allocNum, out poolType, out special);
                if (special)
                {
                    specialStr = "Yes";
                }
                if (poolType == 0)
                {
                    poolTypeStr = "NonPagedPool";
                }
                else if (poolType == 1)
                {
                    poolTypeStr = "PagedPool";
                }
                else if (poolType == 512)
                {
                    poolTypeStr = "NonPagedPoolNx";
                }
                Heap h = new Heap
                {
                    Address = address,
                    PoolType = poolTypeStr,
                    NodeNumber = nodeNum,
                    Special = specialStr,
                    NumberOfAllocs = allocNum
                };
                heaps.Add(h);

                //
                // Get all pool blocks for heap
                //
                for (int j = 0; j < allocNum; j++)
                {
                    tag = GetNextAllocation(out blockAddress, out allocSize, out allocated, out allocType);
                    if (allocSize == 0)
                    {
                        continue;
                    }
                    string subsegType = "Unknown";
                    string isAllocated = "No";
                    if (allocType == 0)
                    {
                        subsegType = "Lfh";
                    }
                    else if (allocType == 1)
                    {
                        subsegType = "Vs";
                    }
                    else if (allocType == 2)
                    {
                        subsegType = "Large";
                    }
                    else if (allocType == 3)
                    {
                        subsegType = "Big";
                    }
                    if (allocated == true)
                    {
                        isAllocated = "Yes";
                    }
                    PoolBlock p = new PoolBlock
                    {
                        Address = blockAddress,
                        Size = allocSize,
                        SubsegType = subsegType,
                        Tag = tag,
                        Heap = h.Address,
                        PoolType = h.PoolType,
                        Special = h.Special,
                        Allocated = isAllocated
                    };
                    poolBlocks.Add(p);
                    if (!tags.ContainsKey(tag))
                    {
                        tags.Add(tag, 1);
                    }
                    else
                    {
                        tags[tag] += 1;
                    }
                    subsegs[subsegType] += 1;
                }
            }
            Heaps.ItemsSource = heaps;
            Heaps.Visibility = Visibility.Visible;
            heapsText.Visibility = Visibility.Visible;
            Blocks.ItemsSource = poolBlocks;
            Blocks.Visibility = Visibility.Visible;

            Dictionary<string, int> tags2 = new Dictionary<string, int>();
            var maxTags = tags.OrderByDescending(x => x.Value);

            int n = 0;
            foreach (KeyValuePair<string, int> entry in maxTags)
            {
                tags2.Add(entry.Key, entry.Value);
                n++;
                if (n == 10)
                {
                    break;
                }
            }
            Tags.ItemsSource = tags2;
            Tags.Visibility = Visibility.Visible;
            tagsText.Visibility = Visibility.Visible;
            Subsegments.ItemsSource = subsegs;
            Subsegments.Visibility = Visibility.Visible;
            SubsegText.Visibility = Visibility.Visible;
        }

        private bool LoadDbgDlls()
        {
            IntPtr dbgEng = NativeMethods.GetModuleHandle("DbgEng.dll");
            if (dbgEng != IntPtr.Zero)
            {
                return true;
            }

            string currentDir = Directory.GetCurrentDirectory();
            if (File.Exists($"{currentDir}\\DbgEng.dll") &&
                File.Exists($"{currentDir}\\DbgHelp.dll"))
            {
                //
                // Load DLLs from current directory if they exist.
                // Loading DbgEng.dll will load DbgHelp.dll as a dependency
                //
                NativeMethods.LoadLibrary($"{currentDir}\\DbgEng.dll");
            }

            dbgEng = NativeMethods.GetModuleHandle("DbgEng.dll");
            if (dbgEng == IntPtr.Zero)
            {
                string message = "No DbgEng.dll! Configure load path";
                MessageBox.Show(message);
                return false;
            }
            return true;
        }

        private void MenuItem_Click_2(object sender, RoutedEventArgs e)
        {
            int heapNum;
            long result;

            if (!LoadDbgDlls())
            {
                return;
            }

            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == true)
            {
                PrintLog($"Opening file '{Path.GetFileName(openFileDialog.FileName)}'.");
                result = GetPoolInformation(openFileDialog.FileName.ToString(), false, out heapNum);
                if (result != 0)
                {
                    PrintLog($"Opening dmp file failed with error {result}");
                    return;
                }
                PrintLog($"File '{Path.GetFileName(openFileDialog.FileName)}' has been opened.");
                DisplayInfoFromDmp(heapNum);
            }
        }

        private void DataFilter(object sender, FilterEventArgs e)
        {
            Int64 heap = 0;
            string poolType = "";
            long minSize = 0;
            long maxSize = 0xffffffff;
            var tag = Tag.Text;
            bool filterByAllocated = false;
            bool allocated = false;
            bool special = false;
            bool filterBySpecial = false;
            string subseg = "";
            ComboBoxItem typeItem = (ComboBoxItem)PoolType.SelectedItem;
            if (typeItem != null)
            {
                poolType = typeItem.Content.ToString();
            }
            if (HeapAddress.Text != "")
            {
                heap = Convert.ToInt64(HeapAddress.Text, 16);
            }
            if (MinSize.Text != "")
            {
                if (MinSize.Text.StartsWith("0x") || MinSize.Text.StartsWith("0X"))
                {
                    minSize = Convert.ToInt64(MinSize.Text, 16);
                }
                else
                {
                    minSize = Convert.ToInt64(MinSize.Text, 10);
                }
            }
            if (MaxSize.Text != "")
            {
                if (MaxSize.Text.StartsWith("0x") || MaxSize.Text.StartsWith("0X"))
                {
                    maxSize = Convert.ToInt64(MaxSize.Text, 16);
                }
                else
                {
                    maxSize = Convert.ToInt64(MaxSize.Text, 10);
                }
            }
            if (Allocated.SelectedItem != null)
            {
                if (((ComboBoxItem)(Allocated.SelectedItem)).Tag.ToString() != "Both")
                {
                    allocated = ((ComboBoxItem)(Allocated.SelectedItem)).Tag.ToString() == "True";
                    filterByAllocated = true;
                }
            }

            if (Special.SelectedItem != null)
            {
                if (((ComboBoxItem)(Special.SelectedItem)).Tag.ToString() != "Both")
                {
                    special = ((ComboBoxItem)(Special.SelectedItem)).Tag.ToString() == "True";
                    filterBySpecial = true;
                }
            }

            if (Subseg.SelectedItem != null)
            {
                if (((ComboBoxItem)(Subseg.SelectedItem)).Tag.ToString() != "All")
                {
                    subseg = ((ComboBoxItem)(Subseg.SelectedItem)).Tag.ToString();
                }
            }

            var obj = e.Item as PoolBlock;
            if (obj != null)
            {
                if (((heap == 0) || (obj.Heap == heap)) &&
                    ((obj.Size >= minSize) && (obj.Size <= maxSize)) &&
                    ((poolType == "") || (poolType == "All") || (obj.PoolType == poolType)) &&
                    ((tag == "" || obj.Tag == tag)) &&
                    ((filterByAllocated == false) || ((obj.Allocated == "Yes") == allocated)) &&
                    ((filterBySpecial == false) || ((obj.Special == "Yes") == special)) &&
                    ((subseg == "") || (obj.SubsegType == subseg)))
                {
                    e.Accepted = true;
                }
                else
                    e.Accepted = false;
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            PrintLog("WPF UI has been initialized successfully.");
        }

        private void MenuItem_Click_3(object sender, RoutedEventArgs e)
        {
            // Get the clicked MenuItem
            var menuItem = (MenuItem)sender;

            // Get the ContextMenu to which the menuItem belongs
            var contextMenu = (ContextMenu)menuItem.Parent;

            // Find the placementTarget
            var item = (DataGrid)contextMenu.PlacementTarget;

            // Get the underlying item, that you cast to your object that is bound
            // to the DataGrid (and has subject and state as property)
            KeyValuePair<string, int> tag = (KeyValuePair<string, int>)item.SelectedCells[0].Item;

            HeapAddress.Clear();
            PoolType.SelectedIndex = -1;
            MinSize.Clear();
            MaxSize.Clear();
            Allocated.SelectedIndex = -1;
            Special.SelectedIndex = -1;
            Subseg.SelectedIndex = -1;
            Tag.Text = tag.Key;
            FilterPoolBlocks();
            if (Pools.SelectedIndex == 0)
            {
                Pools.SelectedIndex++;
            }
        }

        private void MenuItem_Click_4(object sender, RoutedEventArgs e)
        {
            // Get the clicked MenuItem
            var menuItem = (MenuItem)sender;

            // Get the ContextMenu to which the menuItem belongs
            var contextMenu = (ContextMenu)menuItem.Parent;

            // Find the placementTarget
            var item = (DataGrid)contextMenu.PlacementTarget;

            // Get the underlying item, that you cast to your object that is bound
            // to the DataGrid (and has subject and state as property)
            KeyValuePair<string, int> subseg = (KeyValuePair<string, int>)item.SelectedCells[0].Item;

            HeapAddress.Clear();
            PoolType.SelectedIndex = -1;
            MinSize.Clear();
            MaxSize.Clear();
            Allocated.SelectedIndex = -1;
            Special.SelectedIndex = -1;
            Tag.Clear();

            if (subseg.Key == "Lfh")
            {
                Subseg.SelectedIndex = 0;
            }
            else if (subseg.Key == "Vs")
            {
                Subseg.SelectedIndex = 1;
            }
            else if (subseg.Key == "Large")
            {
                Subseg.SelectedIndex = 2;
            }
            else
            {
                PrintLog("Unknown subsegment");
                return;
            }

            FilterPoolBlocks();
            if (Pools.SelectedIndex == 0)
            {
                Pools.SelectedIndex++;
            }
        }
        private void MenuItem_Click_5(object sender, RoutedEventArgs e)
        {
            Environment.Exit(0);
        }

        private void DllPath_Click(object sender, RoutedEventArgs e)
        {
            DllConfig dllConfig = new DllConfig();
            dllConfig.Show();
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            Environment.Exit(0);
        }

        [Serializable]
        public struct ShellExecuteInfo
        {
            public int Size;
            public uint Mask;
            public IntPtr hwnd;
            public string Verb;
            public string File;
            public string Parameters;
            public string Directory;
            public uint Show;
            public IntPtr InstApp;
            public IntPtr IDList;
            public string Class;
            public IntPtr hkeyClass;
            public uint HotKey;
            public IntPtr Icon;
            public IntPtr Monitor;
        }

        [DllImport("shell32.dll", SetLastError = true)]
        extern public static bool ShellExecuteEx(ref ShellExecuteInfo lpExecInfo);
        public void RunAsAdmin()
        {
            const uint SW_NORMAL = 1;
            ShellExecuteInfo sei = new ShellExecuteInfo();
            sei.Size = Marshal.SizeOf(sei);
            sei.Verb = "runas";
            sei.File = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, System.Diagnostics.Process.GetCurrentProcess().MainModule.ModuleName);
            sei.Show = SW_NORMAL;
            if (!ShellExecuteEx(ref sei))
            {
                return;
            }
            Environment.Exit(0);
        }
        public bool IsUserAdministrator()
        {
            bool isAdmin;
            try
            {
                WindowsIdentity user = WindowsIdentity.GetCurrent();
                WindowsPrincipal principal = new WindowsPrincipal(user);
                isAdmin = principal.IsInRole(WindowsBuiltInRole.Administrator);
            }
            catch (Exception ex)
            {
                isAdmin = false;
            }
            return isAdmin;
        }
        private void MenuItem_Click_6(object sender, RoutedEventArgs e)
        {
            int heapNum;
            long res;
            var isAdmin = IsUserAdministrator();
            if (!isAdmin)
            {
                RunAsAdmin();
                PrintLog("Failed to open PoolViewer as admin. Can't analyze machine.");
                return;
            }

            if (!LoadDbgDlls())
            {
                return;
            }

            var filePath = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "live.dmp");
            PrintLog("Creating dump file of the current machine. This can take a few minutes.");
            res = GetPoolInformation(filePath, true, out heapNum);
            if (res != 0)
            {
                PrintLog($"Failed opening dmp file with error {res}");
                return;
            }
            PrintLog($"File '{filePath}' has been opened.");
            try
            {
                DisplayInfoFromDmp(heapNum);
            }
            catch (Exception ex)
            {
                PrintLog($"Exception {ex.Message}");
            }
        }

        private void dg_Sorting(object sender, DataGridSortingEventArgs e)
        {
            FilterPoolBlocks();
        }
    }
}