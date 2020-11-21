#pragma once

using namespace System;
using namespace System::Windows;
using namespace System::Diagnostics;

class GraphicsEngine;

namespace DXControl {
	public ref class ImageControl : public System::Windows::Controls::UserControl
	{
	private:

		System::Windows::Interop::D3DImage^ d3dimg;

		void executeStartRendering(Object^ sender, RoutedEventArgs^ args);
		void FastGLControl_SizeChanged(Object^ sender, SizeChangedEventArgs^ args);
		void OnShutdownStarted(Object^ sender, EventArgs^ args);
		void OnRenderOpenGL(Object^ sender, EventArgs^ e);

		void ResizeRendering();
		void StartRendering();
		void StopRendering();

	public:

		ImageControl();

		void Destroy(void);

		void OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info) override;

	};
}
