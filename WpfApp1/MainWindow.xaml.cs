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
        public MainWindow()
        {
            InitializeComponent();

            Loaded += new RoutedEventHandler(OnLoadedMainWindow);
        }

        public void SetupRender()
        {
            var wih = new WindowInteropHelper(this);
            WGLContext.GLCreate(wih.Handle);

            m_renderTimer = new DispatcherTimer(DispatcherPriority.Render);
            m_renderTimer.Interval = TimeSpan.FromMilliseconds(1);
            m_renderTimer.Tick += new EventHandler(OnTick);
            m_renderTimer.Start();
        }

        public void OnTick(Object sender, EventArgs e)
        {
            WGLContext.Render();
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
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
        }
    }
}
