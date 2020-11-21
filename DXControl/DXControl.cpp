#include "pch.h"

#include "DXControl.h"

using namespace DXControl;
using namespace System::Threading::Tasks;
using namespace System::Windows::Threading;
using namespace System::Windows::Interop;
using namespace System::Windows::Media;

ImageControl::ImageControl()
{
	Loaded += gcnew RoutedEventHandler(this, &ImageControl::executeStartRendering);
	SizeChanged += gcnew SizeChangedEventHandler(this, &ImageControl::FastGLControl_SizeChanged);
	Dispatcher->ShutdownStarted += gcnew System::EventHandler(this, &ImageControl::OnShutdownStarted);
}

void ImageControl::executeStartRendering(Object^ sender, RoutedEventArgs^ args)
{
}

void ImageControl::OnShutdownStarted(Object^ sender, EventArgs^ args)
{
}

void ImageControl::OnRenderOpenGL(Object^ sender, EventArgs^ e)
{
	throw gcnew System::NotImplementedException();
}

void ImageControl::FastGLControl_SizeChanged(Object^ sender, SizeChangedEventArgs^ args)
{
	throw gcnew System::NotImplementedException();
}

void ImageControl::Destroy(void)
{
	throw gcnew System::NotImplementedException();
}

void ImageControl::OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info)
{
	throw gcnew System::NotImplementedException();
}

void ImageControl::ResizeRendering()
{
	double NewWidth = ActualWidth;
	double NewHeight = ActualHeight;

	d3dimg->Lock();
	// d3dimg->SetBackBuffer(System::Windows::Interop::D3DResourceType::IDirect3DSurface9, )
	d3dimg->Unlock();
}

void ImageControl::StartRendering()
{
	CompositionTarget::Rendering += gcnew EventHandler(this, &ImageControl::OnRenderOpenGL);
}

void ImageControl::StopRendering()
{
	CompositionTarget::Rendering -= gcnew EventHandler(this, &ImageControl::OnRenderOpenGL);
}
