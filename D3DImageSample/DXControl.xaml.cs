using SlimDX;
using SlimDX.Direct3D9;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Permissions;
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

namespace D3DImageSample
{
    /// <summary>
    /// Interaction logic for DXControl.xaml
    /// </summary>
    public partial class DXControl : UserControl
    {
        public DXControl()
        {
            InitializeComponent();

            d3dimg.IsFrontBufferAvailableChanged += IsFrontBufferAvailableChanged;

            Loaded += new RoutedEventHandler((object sender, RoutedEventArgs args) => { StartRendering(); });
            SizeChanged += new SizeChangedEventHandler(FastGLControl_SizeChanged);
            Dispatcher.ShutdownStarted += Dispatcher_ShutdownStarted;
        }

        void Dispatcher_ShutdownStarted(object sender, EventArgs e)
        {
            StopRendering();
            _d3dex.Dispose();
        }

        // Gets called when the availability of the Frontbuffer has changed.
        void IsFrontBufferAvailableChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if (d3dimg.IsFrontBufferAvailable)
            {
                StartRendering();
            }
            else
            {
                StopRendering();
            }
        }

        private void StartRendering()
        {
            _hwnd = new HwndSource(0, 0, 0, 0, 0, "test", IntPtr.Zero).Handle;
            ResizeRendering();

            CompositionTarget.Rendering += OnRenderOpenGL;
        }

        private void StopRendering()
        {
            _renderBuffer.Dispose();
            _texture.Dispose();
            _backBuffer.Dispose();
            _device.Dispose();

            // This method is called when WPF loses its D3D device.
            // In such a circumstance, it is very likely that we have lost 
            // our custom D3D device also, so we should just release the scene.
            // We will create a new scene when a D3D device becomes 
            // available again.
            CompositionTarget.Rendering -= OnRenderOpenGL;
        }

        void FastGLControl_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (_hwnd != null)
            {
                ResizeRendering();
            }
        }

        private void ResizeRendering()
        {
            if (_texture != null)
                _texture.Dispose();

            if (_renderBuffer != null)
                _renderBuffer.Dispose();

            if (_backBuffer != null)
                _backBuffer.Dispose();

            if (_device != null)
                _device.Dispose();

            double NewWidth = ActualWidth;
            double NewHeight = ActualHeight;

            PresentParameters pp = new PresentParameters();
            pp.DeviceWindowHandle = _hwnd;
            pp.Windowed = true;
            pp.SwapEffect = SwapEffect.Flip;
            pp.PresentationInterval = PresentInterval.Default;
            pp.BackBufferWidth = (int)NewWidth;
            pp.BackBufferHeight = (int)NewHeight;
            pp.BackBufferFormat = Format.X8R8G8B8;
            pp.BackBufferCount = 1;

            _device = new DeviceEx(_d3dex, 0, DeviceType.Hardware, _hwnd, CreateFlags.HardwareVertexProcessing | CreateFlags.PureDevice | CreateFlags.FpuPreserve | CreateFlags.Multithreaded, pp);
            _texture = new Texture(_device, (int)ActualWidth, (int)ActualHeight, 1, Usage.None, Format.X8R8G8B8, Pool.Default);
            _renderBuffer = _texture.GetSurfaceLevel(0);

            d3dimg.Lock();
            _backBuffer = _device.GetBackBuffer(0, 0);
            d3dimg.SetBackBuffer(D3DResourceType.IDirect3DSurface9, _backBuffer.ComPointer);
            d3dimg.Unlock();
        }

        void OnRenderOpenGL(object sender, EventArgs e)
        {
            RenderingEventArgs args = (RenderingEventArgs)e;

            // It's possible for Rendering to call back twice in the same frame
            // so only render when we haven't already rendered in this frame.
            if (d3dimg.IsFrontBufferAvailable && _lastRender != args.RenderingTime)
            {
                d3dimg.Lock();
                // Repeatedly calling SetBackBuffer with the same IntPtr is
                // a no-op. There is no performance penalty.
                // d3dimg.SetBackBuffer(D3DResourceType.IDirect3DSurface9, pSurface);

                _device.Clear(ClearFlags.Target | ClearFlags.ZBuffer, new Color4(System.Drawing.Color.Red), 1.0f, 0);
                _device.BeginScene();
                _device.EndScene();

                d3dimg.AddDirtyRect(new Int32Rect(0, 0, d3dimg.PixelWidth, d3dimg.PixelHeight));
                d3dimg.Unlock();

                _lastRender = args.RenderingTime;
            }

        }

        TimeSpan _lastRender;

        /// Our instance of Direct3DEx to create devices.
        Direct3DEx _d3dex = new Direct3DEx();

        /// The current device.
        DeviceEx _device;

        /// <summary>
        /// The DirectX-surface to render on (a surface of _texture).
        /// </summary>
        Surface _renderBuffer;

        /// <summary>
        /// BackBuffer of our device
        /// </summary>
        Surface _backBuffer;

        /// <summary>
        /// The DirectX-texture to render on.
        /// </summary>
        Texture _texture;

        /// <summary>
        /// The handle of our dummy-window.
        /// </summary>
        IntPtr _hwnd;
    }

    public static class HRESULT
    { 
        [SecurityPermissionAttribute(SecurityAction.Demand, Flags = SecurityPermissionFlag.UnmanagedCode)]
        public static void Check(int hr)
        {
            Marshal.ThrowExceptionForHR(hr);
        }
    }
}
