/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
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
//   * The name of Intel Corporation may not be used to endorse or promote products
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

/*
 * Visualization functions for Shape Contexts
 */
namespace cv
{
    void drawSCD( const Mat& descriptors, int _angularBins, int _radialBins, Mat& outImage, int index, float pixelsPerBin, int flags)
    {
        outImage = Mat::zeros(_radialBins*pixelsPerBin,_angularBins*pixelsPerBin,CV_32F);
        
        for (int i=0; i<_radialBins; i++)
        {
            for (int j=0; j<_angularBins; j++)
            {
                Mat sub = outImage( Rect(j*pixelsPerBin,i*pixelsPerBin,pixelsPerBin,pixelsPerBin) );
                sub = Scalar(descriptors.at<float>(index, j+i*_angularBins));
            }        
        }
        
        if( flags & DrawSCDFlags::DRAW_NORM )
        {
            normalize(outImage, outImage,0,255, NORM_MINMAX);
            outImage.convertTo(outImage,CV_8U);
        }
        if( flags & DrawSCDFlags::DRAW_NORM_NEG )
        {
            normalize(outImage, outImage,0,255, NORM_MINMAX);
            outImage.convertTo(outImage,CV_8U);
            bitwise_not(outImage,outImage);
        }
    }
}




