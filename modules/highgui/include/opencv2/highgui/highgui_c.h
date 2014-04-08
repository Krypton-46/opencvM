/*
///////////////////////////////////////////////////////////////////////////////////////
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

#ifndef __OPENCV_HIGHGUI_H__
#define __OPENCV_HIGHGUI_H__

#include "opencv2/core/core_c.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************************\
*                                  Basic GUI functions                                   *
\****************************************************************************************/
//YV
//-----------New for Qt
/* For font */
enum {  CV_FONT_LIGHT           = 25,//QFont::Light,
        CV_FONT_NORMAL          = 50,//QFont::Normal,
        CV_FONT_DEMIBOLD        = 63,//QFont::DemiBold,
        CV_FONT_BOLD            = 75,//QFont::Bold,
        CV_FONT_BLACK           = 87 //QFont::Black
};

enum {  CV_STYLE_NORMAL         = 0,//QFont::StyleNormal,
        CV_STYLE_ITALIC         = 1,//QFont::StyleItalic,
        CV_STYLE_OBLIQUE        = 2 //QFont::StyleOblique
};
/* ---------*/


enum
{
/* 8bit, color or not */
    CV_LOAD_IMAGE_UNCHANGED  =-1,
/* 8bit, gray */
    CV_LOAD_IMAGE_GRAYSCALE  =0,
/* ?, color */
    CV_LOAD_IMAGE_COLOR      =1,
/* any depth, ? */
    CV_LOAD_IMAGE_ANYDEPTH   =2,
/* ?, any color */
    CV_LOAD_IMAGE_ANYCOLOR   =4
};

/* load image from file
  iscolor can be a combination of above flags where CV_LOAD_IMAGE_UNCHANGED
  overrides the other flags
  using CV_LOAD_IMAGE_ANYCOLOR alone is equivalent to CV_LOAD_IMAGE_UNCHANGED
  unless CV_LOAD_IMAGE_ANYDEPTH is specified images are converted to 8bit
*/
CVAPI(IplImage*) cvLoadImage( const char* filename, int iscolor CV_DEFAULT(CV_LOAD_IMAGE_COLOR));
CVAPI(CvMat*) cvLoadImageM( const char* filename, int iscolor CV_DEFAULT(CV_LOAD_IMAGE_COLOR));

enum
{
    CV_IMWRITE_JPEG_QUALITY =1,
    CV_IMWRITE_JPEG_PROGRESSIVE =2,
    CV_IMWRITE_JPEG_OPTIMIZE =3,
    CV_IMWRITE_PNG_COMPRESSION =16,
    CV_IMWRITE_PNG_STRATEGY =17,
    CV_IMWRITE_PNG_BILEVEL =18,
    CV_IMWRITE_PNG_STRATEGY_DEFAULT =0,
    CV_IMWRITE_PNG_STRATEGY_FILTERED =1,
    CV_IMWRITE_PNG_STRATEGY_HUFFMAN_ONLY =2,
    CV_IMWRITE_PNG_STRATEGY_RLE =3,
    CV_IMWRITE_PNG_STRATEGY_FIXED =4,
    CV_IMWRITE_PXM_BINARY =32,
    CV_IMWRITE_WEBP_QUALITY =64
};

/* save image to file */
CVAPI(int) cvSaveImage( const char* filename, const CvArr* image,
                        const int* params CV_DEFAULT(0) );

/* decode image stored in the buffer */
CVAPI(IplImage*) cvDecodeImage( const CvMat* buf, int iscolor CV_DEFAULT(CV_LOAD_IMAGE_COLOR));
CVAPI(CvMat*) cvDecodeImageM( const CvMat* buf, int iscolor CV_DEFAULT(CV_LOAD_IMAGE_COLOR));

/* encode image and store the result as a byte vector (single-row 8uC1 matrix) */
CVAPI(CvMat*) cvEncodeImage( const char* ext, const CvArr* image,
                             const int* params CV_DEFAULT(0) );

enum
{
    CV_CVTIMG_FLIP      =1,
    CV_CVTIMG_SWAP_RB   =2
};

/* utility function: convert one image to another with optional vertical flip */
CVAPI(void) cvConvertImage( const CvArr* src, CvArr* dst, int flags CV_DEFAULT(0));


  int cvHaveImageReader( const char* filename );
  int cvHaveImageWriter(const char*);

#ifdef __cplusplus
}
#endif

#endif
