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
using System.Windows.Interop;
using System.Windows.Threading;
using Native;


namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for DocumentView.xaml
    /// </summary>
    public partial class DocumentView : Window
    {
        DispatcherTimer m_renderTimer;

        public DocumentView()
        {
            InitializeComponent();
        }
        private void HTMLBrowser_Navigated(object sender, System.Windows.Navigation.NavigationEventArgs e)
        {
            SetSilent(HTMLBrowser, true); // make it silent
        }
		public static void SetSilent(WebBrowser browser, bool silent)
		{
			if (browser == null)
				throw new ArgumentNullException("browser");
		}
        public void SetupRender()
        {
            var wih = new WindowInteropHelper(this);
            WGLContext.GLCreate(wih.Handle);

            m_renderTimer = new DispatcherTimer(DispatcherPriority.Render);
            m_renderTimer.Interval = TimeSpan.FromMilliseconds(16);
            m_renderTimer.Tick += new EventHandler(OnTick);
            m_renderTimer.Start();
        }
        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            SetupRender();
        }

        public void OnTick(Object sender, EventArgs e)
        {
            WGLContext.Render();
        }
    }
}
