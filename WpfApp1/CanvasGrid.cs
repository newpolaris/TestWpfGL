using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Interop;
using System.Windows.Media;

namespace WpfApp1
{
    class CanvasGrid : DockPanel
    {
        private DocumentView _GLWindow;

        public CanvasGrid()
            : base()
        {
            this.Initialized += OnInitializedLayout;
        }

        void OnInitializedLayout(object sender, EventArgs e)
        {
            CanvasGrid customLayout = (CanvasGrid)sender;
            Grid grid = new Grid();
            grid.Loaded += OnGridLoaded;
			grid.SizeChanged += OnSizeChanged;

            customLayout.Children.Add(grid);
        }

        void OnGridLoaded(object sender, RoutedEventArgs e)
        {
            Window parent = GetParentWindow(this);
            FrameworkElement windowContent = ((parent.Content) as FrameworkElement);
            Rect r = LayoutInformation.GetLayoutSlot(this);

            //HACK: Window border size heuristic - This assumes symmetric width of border and height of bottom border
            double windowLeftMargin = (parent.ActualWidth - windowContent.ActualWidth) / 2;
            //Determine top margin by subtracting border height of bottom border(which is same as left border)
            double windowTopMargin = (parent.ActualHeight - windowContent.ActualHeight) - windowLeftMargin;

            if (_GLWindow != null)
                _GLWindow.Close();

            _GLWindow = new DocumentView();

            _GLWindow.Left = parent.Left + windowLeftMargin;
            _GLWindow.Top = parent.Top + r.Top + windowTopMargin + 53;
            _GLWindow.Width = r.Width;
            _GLWindow.Height = r.Height; ;

            _GLWindow.Owner = parent;
            _GLWindow.Show();

            var wih = new WindowInteropHelper(_GLWindow);
            // GLCreate(wih.Handle.ToInt32());
        }

        void OnSizeChanged(object sneder, SizeChangedEventArgs e)
        {
            if (_GLWindow == null)
                return;

            Window parent = GetParentWindow(this);
            FrameworkElement windowContent = ((parent.Content) as FrameworkElement);
            Rect r = LayoutInformation.GetLayoutSlot(this);

            //HACK: Window border size heuristic - This assumes symmetric width of border and height of bottom border
            double windowLeftMargin = (parent.ActualWidth - windowContent.ActualWidth) / 2;
            //Determine top margin by subtracting border height of bottom border(which is same as left border)
            double windowTopMargin = (parent.ActualHeight - windowContent.ActualHeight) - windowLeftMargin;

            _GLWindow.Left = parent.Left + windowLeftMargin;
            _GLWindow.Top = parent.Top + r.Top + windowTopMargin + 53;
            _GLWindow.Width = r.Width;
            _GLWindow.Height = r.Height;

            _GLWindow.Owner = parent;
            _GLWindow.Show();
        }

        protected Window GetParentWindow(DependencyObject o)
        {
            DependencyObject parent = VisualTreeHelper.GetParent(o);
            if (parent == null)
            {
                FrameworkElement fe = o as FrameworkElement;
                if (fe != null)
                {
                    if (fe is Window)
                    {
                        return fe as Window;
                    }
                    else if (fe.Parent != null)
                    {
                        return GetParentWindow(fe.Parent);
                    }
                }
                throw new ApplicationException("A window parent could not be found for " + o.ToString());
            }
            else
            {
                return GetParentWindow(parent);
            }
        }
    }
}
