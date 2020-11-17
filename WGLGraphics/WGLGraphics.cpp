#include "pch.h"

#include "WGLGraphics.h"
#include <msclr/lock.h>

#pragma managed(push, off)
#include "../WGLContext/GraphicsEngine.h"
#pragma managed(pop)

using namespace System::Threading::Tasks;
using namespace System::Windows;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;
using namespace System::Windows::Controls;
using namespace System::Windows::Threading;

namespace WGLGraphics
{
    int _w = 0, _h = 0, fpsCounter = 0;
	bool isInitialized = false;

    void WGLGraphics::GLControl::OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info)
    {
        _w = (int)info->NewSize.Width;
        _h = (int)info->NewSize.Height;

        if (!isInitialized)
        {
            Grid^ mainGrid = gcnew Grid();
            m_fpsCounter = gcnew TextBlock();
            m_fpsCounter->Margin = Thickness(3);
            m_fpsCounter->VerticalAlignment = System::Windows::VerticalAlignment::Bottom;

            m_lastUpdate = System::DateTime::Now;

            m_renderTimer = gcnew DispatcherTimer(DispatcherPriority::Normal);
            // DispatcherTimer 는 UI Thread 기반, 더 작게 써도 16 ms 정도로 동작함
            m_renderTimer->Interval = System::TimeSpan::FromMilliseconds(10);
            m_renderTimer->Tick += gcnew System::EventHandler(this, &WGLGraphics::GLControl::OnTick);
            m_renderTimer->Start();

            m_ImageControl = gcnew Image();
            m_ImageControl->RenderTransformOrigin = Point(0.5, 0.5);
            m_ImageControl->RenderTransform = gcnew ScaleTransform(1.0, -1.0);

            mainGrid->Children->Add(m_ImageControl);
            mainGrid->Children->Add(m_fpsCounter);

			AddChild(mainGrid);

            System::Windows::Controls::Panel::SetZIndex(m_ImageControl, -1);

            m_graphicsEngine = new GraphicsEngine();
			m_graphicsEngine->create(_w, _h);

            isInitialized = true;
        }
        
        m_graphicsEngine->resize(_w, _h);

        m_writeableImg = gcnew WriteableBitmap(_w, _h, 96, 96, PixelFormats::Pbgra32, nullptr);
        m_WriteableBuffer = (char*)m_writeableImg->BackBuffer.ToPointer();

        // 여기서 Source를 update 하면, OnTick 을 타서 화면이 칠하기 전인 백색 화면이 표시된다 (왜 흰색인거지?)
        // m_ImageControl->Source = m_writeableImg;
    }

    void GLControl::Destroy(void)
	{
		delete m_graphicsEngine;
	}

    void GLControl::UpdateImageData()
    {
        m_writeableImg->Lock();
        m_writeableImg->AddDirtyRect(Int32Rect(0, 0, _w, _h));
        m_writeableImg->Unlock();

        // texture에 데이터 update 후, 기존 source를 제거
        // OnTick 에서는 m_ImageControl의 Drawy 호출이 상관 없으므로 순서 관계 없을 듯
        m_ImageControl->Source = m_writeableImg;
    }

    void GLControl::OnTick(System::Object^ sender, System::EventArgs^ e)
    {
        System::TimeSpan elaped = (System::DateTime::Now - m_lastUpdate);
        if (elaped.TotalMilliseconds >= 1000)
        {
            m_fpsCounter->Text = "FPS= " + fpsCounter.ToString() + " / frametime: " + frameTime.ToString() + " ms";
            fpsCounter = 0;
            m_lastUpdate = System::DateTime::Now;
        }

        stopwatch->Restart();
        // 3 ms ~ 10 ms (fullscreen) due to glReadpixel
        m_graphicsEngine->renderToBuffer(m_WriteableBuffer);
        stopwatch->Stop();

        // ~ 0.1 ms
        // 3개 다 그럭저럭 동작함, 감빡이는 정도는 다시 테스트 해봐야하겠지만, 느껴지지 않음
        UpdateImageData();
        // m_ImageControl->Dispatcher->Invoke(gcnew System::Action(this, &GLControl::UpdateImageData), DispatcherPriority::Normal);
        // m_ImageControl->Dispatcher->Invoke(gcnew System::Action(this, &GLControl::UpdateImageData), DispatcherPriority::Render); 

        fpsCounter++;

        frameTime = stopwatch->Elapsed.TotalMilliseconds;
    }

}
