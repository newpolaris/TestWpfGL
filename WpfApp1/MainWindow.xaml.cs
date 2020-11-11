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
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using Native;

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        DispatcherTimer m_renderTimer;
        CanvasGrid _canvasGrid;

        public MainWindow()
        {
            InitializeComponent();

            Loaded += new RoutedEventHandler(OnLoadedMainWindow);
        }

        public void SetupRender()
        {
        }

        public void OnTick(Object sender, EventArgs e)
        {
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            SetupRender();
        }

        protected override void OnRenderSizeChanged(SizeChangedInfo sizeInfo)
        {
            base.OnRenderSizeChanged(sizeInfo);
        }

        protected override void OnRender(DrawingContext drawingContext)
        {
            base.OnRender(drawingContext);
        }

        void OnLoadedMainWindow(object sender, RoutedEventArgs e)
        {
            _canvasGrid = new CanvasGrid();
            View.Children.Add(_canvasGrid);
        }
    }
}
