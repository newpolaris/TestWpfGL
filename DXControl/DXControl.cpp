#include "pch.h"

#include "DXControl.h"

#pragma managed(push, off)
#include <assert.h>
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

	RotateTransform^ s = gcnew RotateTransform();
	s->CenterX = 0.5;
	s->CenterY = 0.5;
	s->Angle = 180;
	
	TransformGroup^ tg = gcnew TransformGroup();
	tg->Children->Add(s);
	
	ImageBrush^ brush = gcnew ImageBrush(d3dimg);
	brush->RelativeTransform = tg;
	Background = brush;

	Loaded += gcnew RoutedEventHandler(this, &ImageControl::executeStartRendering);
	SizeChanged += gcnew SizeChangedEventHandler(this, &ImageControl::OnSizeChanged);
	Dispatcher->ShutdownStarted += gcnew System::EventHandler(this, &ImageControl::OnShutdownStarted);
}

void ImageControl::IsFrontBufferAvailableChanged(Object^ sender, DependencyPropertyChangedEventArgs e)
{
	if (d3dimg->IsFrontBufferAvailable)
	{
		StartRendering();
	}
	else
	{
		StopRendering();
	}
}

void ImageControl::executeStartRendering(Object^ sender, RoutedEventArgs^ args)
{
	StartRendering();
}

void ImageControl::OnShutdownStarted(Object^ sender, EventArgs^ args)
{
	StopRendering();
}

void ImageControl::OnRenderOpenGL(Object^ sender, EventArgs^ e)
{
	if (m_dxglRender) {
		m_dxglRender->render();

		d3dimg->Lock();
		d3dimg->AddDirtyRect(Int32Rect(0, 0, d3dimg->PixelWidth, d3dimg->PixelHeight));
		d3dimg->Unlock();
	}
}

void ImageControl::OnSizeChanged(Object^ sender, SizeChangedEventArgs^ args)
{
	ResizeRendering();
}

void ImageControl::Destroy(void)
{
	if (m_dxglRender) {
		delete m_dxglRender;
		m_dxglRender = nullptr;
	}
}

void ImageControl::ResizeRendering()
{
	if (m_dxglRender) {
		m_dxglRender->resize(ActualWidth, ActualHeight);

		void* pSurface = m_dxglRender->getBackBuffer();
		if (pSurface == nullptr)
			throw gcnew Exception("Create failure");

		d3dimg->Lock();
		d3dimg->SetBackBuffer(D3DResourceType::IDirect3DSurface9, IntPtr(pSurface));
		d3dimg->Unlock();
	}
}

void ImageControl::StartRendering()
{
	assert(m_dxglRender == nullptr);

	m_dxglRender = new DxGLRender();
	if (!m_dxglRender->create())
		throw gcnew Exception("Create failure");

	// SetBackBuffer is required
	ResizeRendering();

	CompositionTarget::Rendering += gcnew EventHandler(this, &ImageControl::OnRenderOpenGL);
}

void ImageControl::StopRendering()
{
	assert(m_dxglRender != nullptr);

	CompositionTarget::Rendering -= gcnew EventHandler(this, &ImageControl::OnRenderOpenGL);

	m_dxglRender->destroy();

	delete m_dxglRender;
	m_dxglRender = nullptr;
}
