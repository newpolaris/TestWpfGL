#include "pch.h"

#include "WGLGraphics.h"

using namespace System::Threading::Tasks;
using namespace System::Windows;
using namespace System::Windows::Media;
using namespace System::Windows::Media::Imaging;
using namespace System::Windows::Controls;

namespace WGLGraphics
{
    int _w = 0, _h = 0;
	bool isInitialized = false;

    void WGLGraphics::GLControl::OnRenderSizeChanged(System::Windows::SizeChangedInfo^ info)
    {
        _w = (int)info->NewSize.Width;
        _h = (int)info->NewSize.Height;

        if (!isInitialized)
        {
            Grid^ mainGrid = gcnew Grid();
			AddChild(mainGrid);

            isInitialized = true;
        }
    }
}
