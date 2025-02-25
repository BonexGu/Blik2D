/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2014, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "precomp.hpp"
#include BLIK_OPENCV_U_opencl_kernels_imgproc_hpp //original-code:"opencl_kernels_imgproc.hpp"

namespace cv
{

template <typename T, typename ST, typename QT>
struct Integral_SIMD
{
    bool operator()(const T *, size_t,
                    ST *, size_t,
                    QT *, size_t,
                    ST *, size_t,
                    Size, int) const
    {
        return false;
    }
};

#if CV_SSE2

template <>
struct Integral_SIMD<uchar, int, double>
{
    Integral_SIMD()
    {
        haveSSE2 = checkHardwareSupport(CV_CPU_SSE2);
    }

    bool operator()(const uchar * src, size_t _srcstep,
                    int * sum, size_t _sumstep,
                    double * sqsum, size_t,
                    int * tilted, size_t,
                    Size size, int cn) const
    {
        if (sqsum || tilted || cn != 1 || !haveSSE2)
            return false;

        // the first iteration
        memset(sum, 0, (size.width + 1) * sizeof(int));

        __m128i v_zero = _mm_setzero_si128(), prev = v_zero;
        int j = 0;

        // the others
        for (int i = 0; i < size.height; ++i)
        {
            const uchar * src_row = src + _srcstep * i;
            int * prev_sum_row = (int *)((uchar *)sum + _sumstep * i) + 1;
            int * sum_row = (int *)((uchar *)sum + _sumstep * (i + 1)) + 1;

            sum_row[-1] = 0;

            prev = v_zero;
            j = 0;

            for ( ; j + 7 < size.width; j += 8)
            {
                __m128i vsuml = _mm_loadu_si128((const __m128i *)(prev_sum_row + j));
                __m128i vsumh = _mm_loadu_si128((const __m128i *)(prev_sum_row + j + 4));

                __m128i el8shr0 = _mm_loadl_epi64((const __m128i *)(src_row + j));
                __m128i el8shr1 = _mm_slli_si128(el8shr0, 1);
                __m128i el8shr2 = _mm_slli_si128(el8shr0, 2);
                __m128i el8shr3 = _mm_slli_si128(el8shr0, 3);

                vsuml = _mm_add_epi32(vsuml, prev);
                vsumh = _mm_add_epi32(vsumh, prev);

                __m128i el8shr12 = _mm_add_epi16(_mm_unpacklo_epi8(el8shr1, v_zero),
                                                 _mm_unpacklo_epi8(el8shr2, v_zero));
                __m128i el8shr03 = _mm_add_epi16(_mm_unpacklo_epi8(el8shr0, v_zero),
                                                 _mm_unpacklo_epi8(el8shr3, v_zero));
                __m128i el8 = _mm_add_epi16(el8shr12, el8shr03);

                __m128i el4h = _mm_add_epi16(_mm_unpackhi_epi16(el8, v_zero),
                                             _mm_unpacklo_epi16(el8, v_zero));

                vsuml = _mm_add_epi32(vsuml, _mm_unpacklo_epi16(el8, v_zero));
                vsumh = _mm_add_epi32(vsumh, el4h);

                _mm_storeu_si128((__m128i *)(sum_row + j), vsuml);
                _mm_storeu_si128((__m128i *)(sum_row + j + 4), vsumh);

                prev = _mm_add_epi32(prev, _mm_shuffle_epi32(el4h, _MM_SHUFFLE(3, 3, 3, 3)));
            }

            for (int v = sum_row[j - 1] - prev_sum_row[j - 1]; j < size.width; ++j)
                sum_row[j] = (v += src_row[j]) + prev_sum_row[j];
        }

        return true;
    }

    bool haveSSE2;
};

#endif

template<typename T, typename ST, typename QT>
void integral_( const T* src, size_t _srcstep, ST* sum, size_t _sumstep,
                QT* sqsum, size_t _sqsumstep, ST* tilted, size_t _tiltedstep,
                Size size, int cn )
{
    int x, y, k;

    if (Integral_SIMD<T, ST, QT>()(src, _srcstep,
                                   sum, _sumstep,
                                   sqsum, _sqsumstep,
                                   tilted, _tiltedstep,
                                   size, cn))
        return;

    int srcstep = (int)(_srcstep/sizeof(T));
    int sumstep = (int)(_sumstep/sizeof(ST));
    int tiltedstep = (int)(_tiltedstep/sizeof(ST));
    int sqsumstep = (int)(_sqsumstep/sizeof(QT));

    size.width *= cn;

    memset( sum, 0, (size.width+cn)*sizeof(sum[0]));
    sum += sumstep + cn;

    if( sqsum )
    {
        memset( sqsum, 0, (size.width+cn)*sizeof(sqsum[0]));
        sqsum += sqsumstep + cn;
    }

    if( tilted )
    {
        memset( tilted, 0, (size.width+cn)*sizeof(tilted[0]));
        tilted += tiltedstep + cn;
    }

    if( sqsum == 0 && tilted == 0 )
    {
        for( y = 0; y < size.height; y++, src += srcstep - cn, sum += sumstep - cn )
        {
            for( k = 0; k < cn; k++, src++, sum++ )
            {
                ST s = sum[-cn] = 0;
                for( x = 0; x < size.width; x += cn )
                {
                    s += src[x];
                    sum[x] = sum[x - sumstep] + s;
                }
            }
        }
    }
    else if( tilted == 0 )
    {
        for( y = 0; y < size.height; y++, src += srcstep - cn,
                        sum += sumstep - cn, sqsum += sqsumstep - cn )
        {
            for( k = 0; k < cn; k++, src++, sum++, sqsum++ )
            {
                ST s = sum[-cn] = 0;
                QT sq = sqsum[-cn] = 0;
                for( x = 0; x < size.width; x += cn )
                {
                    T it = src[x];
                    s += it;
                    sq += (QT)it*it;
                    ST t = sum[x - sumstep] + s;
                    QT tq = sqsum[x - sqsumstep] + sq;
                    sum[x] = t;
                    sqsum[x] = tq;
                }
            }
        }
    }
    else
    {
        AutoBuffer<ST> _buf(size.width+cn);
        ST* buf = _buf;
        ST s;
        QT sq;
        for( k = 0; k < cn; k++, src++, sum++, tilted++, buf++ )
        {
            sum[-cn] = tilted[-cn] = 0;

            for( x = 0, s = 0, sq = 0; x < size.width; x += cn )
            {
                T it = src[x];
                buf[x] = tilted[x] = it;
                s += it;
                sq += (QT)it*it;
                sum[x] = s;
                if( sqsum )
                    sqsum[x] = sq;
            }

            if( size.width == cn )
                buf[cn] = 0;

            if( sqsum )
            {
                sqsum[-cn] = 0;
                sqsum++;
            }
        }

        for( y = 1; y < size.height; y++ )
        {
            src += srcstep - cn;
            sum += sumstep - cn;
            tilted += tiltedstep - cn;
            buf += -cn;

            if( sqsum )
                sqsum += sqsumstep - cn;

            for( k = 0; k < cn; k++, src++, sum++, tilted++, buf++ )
            {
                T it = src[0];
                ST t0 = s = it;
                QT tq0 = sq = (QT)it*it;

                sum[-cn] = 0;
                if( sqsum )
                    sqsum[-cn] = 0;
                tilted[-cn] = tilted[-tiltedstep];

                sum[0] = sum[-sumstep] + t0;
                if( sqsum )
                    sqsum[0] = sqsum[-sqsumstep] + tq0;
                tilted[0] = tilted[-tiltedstep] + t0 + buf[cn];

                for( x = cn; x < size.width - cn; x += cn )
                {
                    ST t1 = buf[x];
                    buf[x - cn] = t1 + t0;
                    t0 = it = src[x];
                    tq0 = (QT)it*it;
                    s += t0;
                    sq += tq0;
                    sum[x] = sum[x - sumstep] + s;
                    if( sqsum )
                        sqsum[x] = sqsum[x - sqsumstep] + sq;
                    t1 += buf[x + cn] + t0 + tilted[x - tiltedstep - cn];
                    tilted[x] = t1;
                }

                if( size.width > cn )
                {
                    ST t1 = buf[x];
                    buf[x - cn] = t1 + t0;
                    t0 = it = src[x];
                    tq0 = (QT)it*it;
                    s += t0;
                    sq += tq0;
                    sum[x] = sum[x - sumstep] + s;
                    if( sqsum )
                        sqsum[x] = sqsum[x - sqsumstep] + sq;
                    tilted[x] = t0 + t1 + tilted[x - tiltedstep - cn];
                    buf[x] = t0;
                }

                if( sqsum )
                    sqsum++;
            }
        }
    }
}


#define DEF_INTEGRAL_FUNC(suffix, T, ST, QT) \
static void integral_##suffix( T* src, size_t srcstep, ST* sum, size_t sumstep, QT* sqsum, size_t sqsumstep, \
                              ST* tilted, size_t tiltedstep, Size size, int cn ) \
{ integral_(src, srcstep, sum, sumstep, sqsum, sqsumstep, tilted, tiltedstep, size, cn); }

DEF_INTEGRAL_FUNC(8u32s, uchar, int, double)
DEF_INTEGRAL_FUNC(8u32s32s, uchar, int, int)
DEF_INTEGRAL_FUNC(8u32f64f, uchar, float, double)
DEF_INTEGRAL_FUNC(8u64f64f, uchar, double, double)
DEF_INTEGRAL_FUNC(16u64f64f, ushort, double, double)
DEF_INTEGRAL_FUNC(16s64f64f, short, double, double)
DEF_INTEGRAL_FUNC(32f32f64f, float, float, double)
DEF_INTEGRAL_FUNC(32f64f64f, float, double, double)
DEF_INTEGRAL_FUNC(64f64f64f, double, double, double)

DEF_INTEGRAL_FUNC(8u32s32f, uchar, int, float)
DEF_INTEGRAL_FUNC(8u32f32f, uchar, float, float)
DEF_INTEGRAL_FUNC(32f32f32f, float, float, float)

typedef void (*IntegralFunc)(const uchar* src, size_t srcstep, uchar* sum, size_t sumstep,
                             uchar* sqsum, size_t sqsumstep, uchar* tilted, size_t tstep,
                             Size size, int cn );

#ifdef HAVE_OPENCL

static bool ocl_integral( InputArray _src, OutputArray _sum, int sdepth )
{
    bool doubleSupport = ocl::Device::getDefault().doubleFPConfig() > 0;

    if ( (_src.type() != CV_8UC1) ||
        !(sdepth == CV_32S || sdepth == CV_32F || (doubleSupport && sdepth == CV_64F)))
        return false;

    static const int tileSize = 16;

    String build_opt = format("-D sumT=%s -D LOCAL_SUM_SIZE=%d%s",
                                ocl::typeToStr(sdepth), tileSize,
                                doubleSupport ? " -D DOUBLE_SUPPORT" : "");

    ocl::Kernel kcols("integral_sum_cols", ocl::imgproc::integral_sum_oclsrc, build_opt);
    if (kcols.empty())
        return false;

    UMat src = _src.getUMat();
    Size src_size = src.size();
    Size bufsize(((src_size.height + tileSize - 1) / tileSize) * tileSize, ((src_size.width + tileSize - 1) / tileSize) * tileSize);
    UMat buf(bufsize, sdepth);
    kcols.args(ocl::KernelArg::ReadOnly(src), ocl::KernelArg::WriteOnlyNoSize(buf));
    size_t gt = src.cols, lt = tileSize;
    if (!kcols.run(1, &gt, &lt, false))
        return false;

    ocl::Kernel krows("integral_sum_rows", ocl::imgproc::integral_sum_oclsrc, build_opt);
    if (krows.empty())
        return false;

    Size sumsize(src_size.width + 1, src_size.height + 1);
    _sum.create(sumsize, sdepth);
    UMat sum = _sum.getUMat();

    krows.args(ocl::KernelArg::ReadOnlyNoSize(buf), ocl::KernelArg::WriteOnly(sum));
    gt = src.rows;
    return krows.run(1, &gt, &lt, false);
}

static bool ocl_integral( InputArray _src, OutputArray _sum, OutputArray _sqsum, int sdepth, int sqdepth )
{
    bool doubleSupport = ocl::Device::getDefault().doubleFPConfig() > 0;

    if ( _src.type() != CV_8UC1 || (!doubleSupport && (sdepth == CV_64F || sqdepth == CV_64F)) )
        return false;

    static const int tileSize = 16;

    String build_opt = format("-D SUM_SQUARE -D sumT=%s -D sumSQT=%s -D LOCAL_SUM_SIZE=%d%s",
                                ocl::typeToStr(sdepth), ocl::typeToStr(sqdepth),
                                tileSize,
                                doubleSupport ? " -D DOUBLE_SUPPORT" : "");

    ocl::Kernel kcols("integral_sum_cols", ocl::imgproc::integral_sum_oclsrc, build_opt);
    if (kcols.empty())
        return false;

    UMat src = _src.getUMat();
    Size src_size = src.size();
    Size bufsize(((src_size.height + tileSize - 1) / tileSize) * tileSize, ((src_size.width + tileSize - 1) / tileSize) * tileSize);
    UMat buf(bufsize, sdepth);
    UMat buf_sq(bufsize, sqdepth);
    kcols.args(ocl::KernelArg::ReadOnly(src), ocl::KernelArg::WriteOnlyNoSize(buf), ocl::KernelArg::WriteOnlyNoSize(buf_sq));
    size_t gt = src.cols, lt = tileSize;
    if (!kcols.run(1, &gt, &lt, false))
        return false;

    ocl::Kernel krows("integral_sum_rows", ocl::imgproc::integral_sum_oclsrc, build_opt);
    if (krows.empty())
        return false;

    Size sumsize(src_size.width + 1, src_size.height + 1);
    _sum.create(sumsize, sdepth);
    UMat sum = _sum.getUMat();
    _sqsum.create(sumsize, sqdepth);
    UMat sum_sq = _sqsum.getUMat();

    krows.args(ocl::KernelArg::ReadOnlyNoSize(buf), ocl::KernelArg::ReadOnlyNoSize(buf_sq), ocl::KernelArg::WriteOnly(sum), ocl::KernelArg::WriteOnlyNoSize(sum_sq));
    gt = src.rows;
    return krows.run(1, &gt, &lt, false);
}

#endif

}

#if defined(HAVE_IPP)
namespace cv
{
static bool ipp_integral(InputArray _src, OutputArray _sum, OutputArray _sqsum, OutputArray _tilted, int sdepth, int sqdepth)
{
#if !defined(HAVE_IPP_ICV_ONLY) && (IPP_VERSION_X100 != 900) // Disabled on ICV due invalid results
    int type = _src.type(), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);
    if( sdepth <= 0 )
        sdepth = depth == CV_8U ? CV_32S : CV_64F;
    if ( sqdepth <= 0 )
         sqdepth = CV_64F;
    sdepth = CV_MAT_DEPTH(sdepth), sqdepth = CV_MAT_DEPTH(sqdepth);


    Size ssize = _src.size(), isize(ssize.width + 1, ssize.height + 1);
    _sum.create( isize, CV_MAKETYPE(sdepth, cn) );
    Mat src = _src.getMat(), sum =_sum.getMat(), sqsum, tilted;

    if( _sqsum.needed() )
    {
        _sqsum.create( isize, CV_MAKETYPE(sqdepth, cn) );
        sqsum = _sqsum.getMat();
    };

    if( ( depth == CV_8U ) && ( sdepth == CV_32F || sdepth == CV_32S ) && ( !_tilted.needed() ) && ( !_sqsum.needed() || sqdepth == CV_64F ) && ( cn == 1 ) )
    {
        IppStatus status = ippStsErr;
        IppiSize srcRoiSize = ippiSize( src.cols, src.rows );
        if( sdepth == CV_32F )
        {
            if( _sqsum.needed() )
            {
                status = ippiSqrIntegral_8u32f64f_C1R( (const Ipp8u*)src.data, (int)src.step, (Ipp32f*)sum.data, (int)sum.step, (Ipp64f*)sqsum.data, (int)sqsum.step, srcRoiSize, 0, 0 );
            }
            else
            {
                status = ippiIntegral_8u32f_C1R( (const Ipp8u*)src.data, (int)src.step, (Ipp32f*)sum.data, (int)sum.step, srcRoiSize, 0 );
            }
        }
        else if( sdepth == CV_32S )
        {
            if( _sqsum.needed() )
            {
                status = ippiSqrIntegral_8u32s64f_C1R( (const Ipp8u*)src.data, (int)src.step, (Ipp32s*)sum.data, (int)sum.step, (Ipp64f*)sqsum.data, (int)sqsum.step, srcRoiSize, 0, 0 );
            }
            else
            {
                status = ippiIntegral_8u32s_C1R( (const Ipp8u*)src.data, (int)src.step, (Ipp32s*)sum.data, (int)sum.step, srcRoiSize, 0 );
            }
        }
        if (0 <= status)
        {
            CV_IMPL_ADD(CV_IMPL_IPP);
            return true;
        }
    }
#else
    CV_UNUSED(_src); CV_UNUSED(_sum); CV_UNUSED(_sqsum); CV_UNUSED(_tilted); CV_UNUSED(sdepth); CV_UNUSED(sqdepth);
#endif
    return false;
}
}
#endif

void cv::integral( InputArray _src, OutputArray _sum, OutputArray _sqsum, OutputArray _tilted, int sdepth, int sqdepth )
{
    int type = _src.type(), depth = CV_MAT_DEPTH(type), cn = CV_MAT_CN(type);
    if( sdepth <= 0 )
        sdepth = depth == CV_8U ? CV_32S : CV_64F;
    if ( sqdepth <= 0 )
         sqdepth = CV_64F;
    sdepth = CV_MAT_DEPTH(sdepth), sqdepth = CV_MAT_DEPTH(sqdepth);

#ifdef HAVE_OPENCL
    if (ocl::useOpenCL() && _sum.isUMat() && !_tilted.needed())
    {
        if (!_sqsum.needed())
        {
            CV_OCL_RUN(ocl::useOpenCL(), ocl_integral(_src, _sum, sdepth))
        }
        else if (_sqsum.isUMat())
            CV_OCL_RUN(ocl::useOpenCL(), ocl_integral(_src, _sum, _sqsum, sdepth, sqdepth))
    }
#endif

    Size ssize = _src.size(), isize(ssize.width + 1, ssize.height + 1);
    _sum.create( isize, CV_MAKETYPE(sdepth, cn) );
    Mat src = _src.getMat(), sum =_sum.getMat(), sqsum, tilted;

    if( _sqsum.needed() )
    {
        _sqsum.create( isize, CV_MAKETYPE(sqdepth, cn) );
        sqsum = _sqsum.getMat();
    };

    CV_IPP_RUN(( depth == CV_8U ) && ( sdepth == CV_32F || sdepth == CV_32S ) &&
        ( !_tilted.needed() ) && ( !_sqsum.needed() || sqdepth == CV_64F ) && ( cn == 1 ),
        ipp_integral(_src, _sum, _sqsum, _tilted, sdepth, sqdepth));

    if( _tilted.needed() )
    {
        _tilted.create( isize, CV_MAKETYPE(sdepth, cn) );
        tilted = _tilted.getMat();
    }

    IntegralFunc func = 0;
    if( depth == CV_8U && sdepth == CV_32S && sqdepth == CV_64F )
        func = (IntegralFunc)GET_OPTIMIZED(integral_8u32s);
    else if( depth == CV_8U && sdepth == CV_32S && sqdepth == CV_32F )
        func = (IntegralFunc)integral_8u32s32f;
    else if( depth == CV_8U && sdepth == CV_32S && sqdepth == CV_32S )
        func = (IntegralFunc)integral_8u32s32s;
    else if( depth == CV_8U && sdepth == CV_32F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_8u32f64f;
    else if( depth == CV_8U && sdepth == CV_32F && sqdepth == CV_32F )
        func = (IntegralFunc)integral_8u32f32f;
    else if( depth == CV_8U && sdepth == CV_64F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_8u64f64f;
    else if( depth == CV_16U && sdepth == CV_64F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_16u64f64f;
    else if( depth == CV_16S && sdepth == CV_64F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_16s64f64f;
    else if( depth == CV_32F && sdepth == CV_32F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_32f32f64f;
    else if( depth == CV_32F && sdepth == CV_32F && sqdepth == CV_32F )
        func = (IntegralFunc)integral_32f32f32f;
    else if( depth == CV_32F && sdepth == CV_64F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_32f64f64f;
    else if( depth == CV_64F && sdepth == CV_64F && sqdepth == CV_64F )
        func = (IntegralFunc)integral_64f64f64f;
    else
        CV_Error( CV_StsUnsupportedFormat, "" );

    func( src.ptr(), src.step, sum.ptr(), sum.step, sqsum.ptr(), sqsum.step,
          tilted.ptr(), tilted.step, src.size(), cn );
}

void cv::integral( InputArray src, OutputArray sum, int sdepth )
{
    integral( src, sum, noArray(), noArray(), sdepth );
}

void cv::integral( InputArray src, OutputArray sum, OutputArray sqsum, int sdepth, int sqdepth )
{
    integral( src, sum, sqsum, noArray(), sdepth, sqdepth );
}


CV_IMPL void
cvIntegral( const CvArr* image, CvArr* sumImage,
            CvArr* sumSqImage, CvArr* tiltedSumImage )
{
    cv::Mat src = cv::cvarrToMat(image), sum = cv::cvarrToMat(sumImage), sum0 = sum;
    cv::Mat sqsum0, sqsum, tilted0, tilted;
    cv::Mat *psqsum = 0, *ptilted = 0;

    if( sumSqImage )
    {
        sqsum0 = sqsum = cv::cvarrToMat(sumSqImage);
        psqsum = &sqsum;
    }

    if( tiltedSumImage )
    {
        tilted0 = tilted = cv::cvarrToMat(tiltedSumImage);
        ptilted = &tilted;
    }
    cv::integral( src, sum, psqsum ? cv::_OutputArray(*psqsum) : cv::_OutputArray(),
                  ptilted ? cv::_OutputArray(*ptilted) : cv::_OutputArray(), sum.depth() );

    CV_Assert( sum.data == sum0.data && sqsum.data == sqsum0.data && tilted.data == tilted0.data );
}

/* End of file. */
