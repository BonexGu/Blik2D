#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wmissing-declarations"
#  if defined __clang__ || defined __APPLE__
#    pragma GCC diagnostic ignored "-Wmissing-prototypes"
#    pragma GCC diagnostic ignored "-Wextra"
#  endif
#endif

#ifndef __OPENCV_TEST_PRECOMP_HPP__
#define __OPENCV_TEST_PRECOMP_HPP__

#include <iostream>
#include "opencv2/ts.hpp"
#include BLIK_OPENCV_U_opencv2__imgproc_hpp //original-code:"opencv2/imgproc.hpp"
#include BLIK_OPENCV_U_opencv2__calib3d_hpp //original-code:"opencv2/calib3d.hpp"
#include BLIK_OPENCV_U_opencv2__imgcodecs_hpp //original-code:"opencv2/imgcodecs.hpp"

namespace cvtest
{
    void Rodrigues(const Mat& src, Mat& dst, Mat* jac=0);
}

#endif
