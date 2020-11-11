#pragma once

using namespace System;

namespace WGLGraphics {
	public ref class GLControl : public System::Windows::Controls::UserControl
	{
	private:

		System::Windows::Threading::DispatcherTimer^ m_renderTimer;
        System::Windows::Controls::TextBlock^ m_fpsCounter;
		System::DateTime m_lastUpdate;
;

		char* m_WriteableBuffer;
		bool m_isRunning;

		System::Windows::Controls::Image^ m_ImageControl;
		System::Windows::Media::Imaging::WriteableBitmap^ m_writeableImg;

        void OnTick(System::Object^ sender, System::EventArgs^ e);
        void UpdateImageData();

	protected:

	public:
		bool GetIsRunning(void);
		void Destroy(void);

		void OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info) override;
	};
}
