#pragma once

using namespace System;

namespace WGLGraphics {
	public ref class GLControl : public System::Windows::Controls::UserControl
	{
	public:

		void OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info) override;
	};
}
