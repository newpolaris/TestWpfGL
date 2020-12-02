#pragma once

using namespace System;
using namespace System::Windows;
using namespace System::Diagnostics;

class DxGLRender;

namespace DXControl {
	public ref class ImageControl : public System::Windows::Controls::UserControl
	{
	private:

		System::Windows::Interop::D3DImage^ d3dimg;
		DxGLRender* m_dxglRender;

		void executeStartRendering(Object^ sender, RoutedEventArgs^ args);
		void OnSizeChanged(Object^ sender, SizeChangedEventArgs^ args);
		void OnShutdownStarted(Object^ sender, EventArgs^ args);
		void OnRenderOpenGL(Object^ sender, EventArgs^ e);
		void IsFrontBufferAvailableChanged(Object^ sender, DependencyPropertyChangedEventArgs e);

		void ResizeRendering();
		void StartRendering();
		void StopRendering();

	public:

		ImageControl();

		void Destroy(void);
	};
}
