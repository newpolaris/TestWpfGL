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

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for DocumentView.xaml
    /// </summary>
    public partial class DocumentView : Window
    {
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

	}
}
