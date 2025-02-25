﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include BLIK_OPENCV_V_opencv2__core_hpp //original-code:<opencv2/core.hpp>
#include BLIK_OPENCV_V_opencv2__imgproc_hpp //original-code:<opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/videoio/cap_winrt.hpp>

using namespace video_capture_xaml;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

using namespace ::Windows::Foundation;
using namespace Windows::UI::Xaml::Media::Imaging;

namespace video_capture_xaml
{
    // nb. implemented in main.cpp
    void cvMain();

    MainPage::MainPage()
    {
        InitializeComponent();

        Window::Current->VisibilityChanged += ref new Windows::UI::Xaml::WindowVisibilityChangedEventHandler(this, &video_capture_xaml::MainPage::OnVisibilityChanged);

        // attach XAML elements
        cv::winrt_setFrameContainer(cvImage);

        // start (1) frame-grabbing loop and (2) message loop
        //
        // 1. Function passed as an argument must implement common OCV reading frames
        //    pattern (see cv::VideoCapture documentation) AND call cv::winrt_imgshow().
        // 2. Message processing loop required to overcome WinRT container and type
        //    conversion restrictions. OCV provides default implementation
        cv::winrt_startMessageLoop(cvMain);
    }
}

void video_capture_xaml::MainPage::OnVisibilityChanged(Platform::Object ^sender,
    Windows::UI::Core::VisibilityChangedEventArgs ^e)
{
    cv::winrt_onVisibilityChanged(e->Visible);
}
