/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Copyright (C) 2015, Itseez Inc., all rights reserved.
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

/*============================================================================

This C header file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3c, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#pragma once
#ifndef softfloat_h
#define softfloat_h 1

#include "cvdef.h"

namespace cv
{

/*----------------------------------------------------------------------------
| Software floating-point rounding mode.
*----------------------------------------------------------------------------*/
enum {
    round_near_even   = 0,
    round_minMag      = 1,
    round_min         = 2,
    round_max         = 3,
    round_near_maxMag = 4,
    round_odd         = 5
};

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/

struct softfloat;
struct softdouble;

struct CV_EXPORTS softfloat
{
public:
    softfloat() { v = 0; }
    softfloat( const softfloat& c) { v = c.v; }
    softfloat& operator=( const softfloat& c )
    {
        if(&c != this) v = c.v;
        return *this;
    }
    static const softfloat fromRaw( const uint32_t a ) { softfloat x; x.v = a; return x; }

    explicit softfloat( const uint32_t );
    explicit softfloat( const uint64_t );
    explicit softfloat( const int32_t );
    explicit softfloat( const int64_t );
    explicit softfloat( const float a ) { Cv32suf s; s.f = a; v = s.u; }

    uint_fast32_t toUI32( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;
    uint_fast64_t toUI64( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;
    int_fast32_t   toI32( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;
    int_fast64_t   toI64( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;

    softfloat  round( uint_fast8_t roundingMode = round_near_even, bool exact = false) const;
    operator softdouble() const;
    float toFloat() const { Cv32suf s; s.u = v; return s.f; }

    softfloat operator + (const softfloat&) const;
    softfloat operator - (const softfloat&) const;
    softfloat operator * (const softfloat&) const;
    softfloat operator / (const softfloat&) const;
    softfloat operator % (const softfloat&) const;
    softfloat operator - () const { softfloat x; x.v = v ^ (1U << 31); return x; }

    softfloat& operator += (const softfloat& a) { *this = *this + a; return *this; }
    softfloat& operator -= (const softfloat& a) { *this = *this - a; return *this; }
    softfloat& operator *= (const softfloat& a) { *this = *this * a; return *this; }
    softfloat& operator /= (const softfloat& a) { *this = *this / a; return *this; }
    softfloat& operator %= (const softfloat& a) { *this = *this % a; return *this; }

    bool operator == ( const softfloat& ) const;
    bool operator != ( const softfloat& ) const;
    bool operator >  ( const softfloat& ) const;
    bool operator >= ( const softfloat& ) const;
    bool operator <  ( const softfloat& ) const;
    bool operator <= ( const softfloat& ) const;

    bool isNaN() const { return (v & 0x7fffffff)  > 0x7f800000; }
    bool isInf() const { return (v & 0x7fffffff) == 0x7f800000; }

    static softfloat zero() { return softfloat::fromRaw( 0 ); }
    static softfloat  inf() { return softfloat::fromRaw( 0xFF << 23 ); }
    static softfloat  nan() { return softfloat::fromRaw( 0x7fffffff ); }
    static softfloat  one() { return softfloat::fromRaw(  127 << 23 ); }

    uint32_t v;
};

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/

struct CV_EXPORTS softdouble
{
public:
    softdouble() { }
    softdouble( const softdouble& c) { v = c.v; }
    softdouble& operator=( const softdouble& c )
    {
        if(&c != this) v = c.v;
        return *this;
    }
    static softdouble fromRaw( const uint64_t a ) { softdouble x; x.v = a; return x; }

    explicit softdouble( const uint32_t );
    explicit softdouble( const uint64_t );
    explicit softdouble( const  int32_t );
    explicit softdouble( const  int64_t );
    explicit softdouble( const double a ) { Cv64suf s; s.f = a; v = s.u; }

    uint_fast32_t toUI32( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;
    uint_fast64_t toUI64( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;
    int_fast32_t   toI32( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;
    int_fast64_t   toI64( uint_fast8_t roundingMode = round_near_even, bool exact = false ) const;

    softdouble  round( uint_fast8_t roundingMode = round_near_even, bool exact = false) const;
    operator softfloat() const;
    double toDouble() const { Cv64suf s; s.u = v; return s.f; }

    softdouble operator + (const softdouble&) const;
    softdouble operator - (const softdouble&) const;
    softdouble operator * (const softdouble&) const;
    softdouble operator / (const softdouble&) const;
    softdouble operator % (const softdouble&) const;
    softdouble operator - () const { softdouble x; x.v = v ^ (1ULL << 63); return x; }

    softdouble& operator += (const softdouble& a) { *this = *this + a; return *this; }
    softdouble& operator -= (const softdouble& a) { *this = *this - a; return *this; }
    softdouble& operator *= (const softdouble& a) { *this = *this * a; return *this; }
    softdouble& operator /= (const softdouble& a) { *this = *this / a; return *this; }
    softdouble& operator %= (const softdouble& a) { *this = *this % a; return *this; }

    bool operator == ( const softdouble& ) const;
    bool operator != ( const softdouble& ) const;
    bool operator >  ( const softdouble& ) const;
    bool operator >= ( const softdouble& ) const;
    bool operator <  ( const softdouble& ) const;
    bool operator <= ( const softdouble& ) const;

    bool isNaN() const { return (v & 0x7fffffffffffffff)  > 0x7ff0000000000000; }
    bool isInf() const { return (v & 0x7fffffffffffffff) == 0x7ff0000000000000; }

    static softdouble zero() { return softdouble::fromRaw( 0 ); }
    static softdouble  inf() { return softdouble::fromRaw( (uint_fast64_t)(0x7FF) << 52 ); }
    static softdouble  nan() { return softdouble::fromRaw( CV_BIG_INT(0x7FFFFFFFFFFFFFFF) ); }
    static softdouble  one() { return softdouble::fromRaw( (uint_fast64_t)( 1023) << 52 ); }

    uint64_t v;
};

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/

CV_EXPORTS softfloat f32_mulAdd( softfloat, softfloat, softfloat );
CV_EXPORTS softfloat f32_sqrt( softfloat );
CV_EXPORTS softdouble f64_mulAdd( softdouble, softdouble, softdouble );
CV_EXPORTS softdouble f64_sqrt( softdouble );

/*----------------------------------------------------------------------------
| Ported from OpenCV and added for usability
*----------------------------------------------------------------------------*/

inline softfloat min(const softfloat a, const softfloat b);
inline softdouble min(const softdouble a, const softdouble b);

inline softfloat max(const softfloat a, const softfloat b);
inline softdouble max(const softdouble a, const softdouble b);

inline softfloat min(const softfloat a, const softfloat b) { return (a > b) ? b : a; }
inline softdouble min(const softdouble a, const softdouble b) { return (a > b) ? b : a; }

inline softfloat max(const softfloat a, const softfloat b) { return (a > b) ? a : b; }
inline softdouble max(const softdouble a, const softdouble b) { return (a > b) ? a : b; }

inline softfloat f32_abs( softfloat a) { softfloat x; x.v = a.v & ((1U   << 31) - 1); return x; }
inline softdouble f64_abs( softdouble a) { softdouble x; x.v = a.v & ((1ULL << 63) - 1); return x; }

CV_EXPORTS softfloat f32_exp( softfloat );
CV_EXPORTS softfloat f32_log( softfloat );
CV_EXPORTS softfloat f32_pow( softfloat, softfloat );

CV_EXPORTS softdouble f64_exp( softdouble );
CV_EXPORTS softdouble f64_log( softdouble );
CV_EXPORTS softdouble f64_pow( softdouble, softdouble );

CV_EXPORTS softfloat f32_cbrt( softfloat );

}

#endif
