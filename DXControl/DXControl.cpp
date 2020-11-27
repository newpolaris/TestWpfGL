#include "pch.h"

#include "DXControl.h"

#pragma managed(push, off)
#include "../DxGLLib/DxLib.h"
#pragma managed(pop)

using namespace DXControl;
using namespace System::Threading::Tasks;
using namespace System::Windows::Threading;
using namespace System::Windows::Interop;
using namespace System::Windows::Media;

ImageControl::ImageControl()
{
	d3dimg = gcnew D3DImage();
	d3dimg->IsFrontBufferAvailableChanged += gcnew DependencyPropertyChangedEventHandler(this, &ImageControl::IsFrontBufferAvailableChanged);

	Background = gcnew ImageBrush(d3dimg);

	Loaded += gcnew RoutedEventHandler(this, &ImageControl::executeStartRendering);
	SizeChanged += gcnew SizeChangedEventHandler(this, &ImageControl::FastGLControl_SizeChanged);
	Dispatcher->ShutdownStarted += gcnew System::EventHandler(this, &ImageControl::OnShutdownStarted);
}

void ImageControl::IsFrontBufferAvailableChanged(Object^ sender, DependencyPropertyChangedEventArgs e)
{
	if (d3dimg->IsFrontBufferAvailable)
	{
	}
	else
	{
	}
}

void ImageControl::executeStartRendering(Object^ sender, RoutedEventArgs^ args)
{
}

void ImageControl::OnShutdownStarted(Object^ sender, EventArgs^ args)
{
	StopRendering();
}

void ImageControl::OnRenderOpenGL(Object^ sender, EventArgs^ e)
{
}

void ImageControl::FastGLControl_SizeChanged(Object^ sender, SizeChangedEventArgs^ args)
{
}

void ImageControl::Destroy(void)
{
}

void ImageControl::OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info)
{
}

void ImageControl::ResizeRendering()
{
	double NewWidth = ActualWidth;
	double NewHeight = ActualHeight;

	d3dimg->Lock();
	// d3dimg->SetBackBuffer(D3DResourceType::IDirect3DSurface9,  )
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
