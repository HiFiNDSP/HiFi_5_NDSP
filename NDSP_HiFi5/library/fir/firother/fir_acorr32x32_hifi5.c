/* ------------------------------------------------------------------------ */
/* Copyright (c) 2021 by Cadence Design Systems, Inc. ALL RIGHTS RESERVED.  */
/* These coded instructions, statements, and computer programs ('Cadence    */
/* Libraries') are the copyrighted works of Cadence Design Systems Inc.     */
/* Cadence IP is licensed for use with Cadence processor cores only and     */
/* must not be used for any other processors and platforms. Your use of the */
/* Cadence Libraries is subject to the terms of the license agreement you   */
/* have entered into with Cadence Design Systems, or a sublicense granted   */
/* to you by a direct Cadence license.                                     */
/* ------------------------------------------------------------------------ */
/*  IntegrIT, Ltd.   www.integrIT.com, info@integrIT.com                    */
/*                                                                          */
/* NatureDSP_Baseband Library                                               */
/*                                                                          */
/* This library contains copyrighted materials, trade secrets and other     */
/* proprietary information of IntegrIT, Ltd. This software is licensed for  */
/* use with Cadence processor cores only and must not be used for any other */
/* processors and platforms. The license to use these sources was given to  */
/* Cadence, Inc. under Terms and Condition of a Software License Agreement  */
/* between Cadence, Inc. and IntegrIT, Ltd.                                 */
/* ------------------------------------------------------------------------ */
/*          Copyright (C) 2009-2021 IntegrIT, Limited.                      */
/*                      All Rights Reserved.                                */
/* ------------------------------------------------------------------------ */

/*
    NatureDSP Signal Processing Library. FIR part
    Real data circular auto-correlation, 32x32-bit
    C code optimized for HiFi5
    IntegrIT, 2006-2019
*/

/* Portable data types. */
#include "NatureDSP_types.h"
/* Signal Processing Library API. */
#include "NatureDSP_Signal_fir.h"
/* Common utility and macros declarations. */
#include "common.h"


/*-------------------------------------------------------------------------
  Circular Autocorrelation 
  Estimates the auto-correlation of vector x. Returns autocorrelation of 
  length N.

  Precision: 
  16x16     16-bit data, 16-bit outputs
  32x32     32-bit data, 32-bit outputs
  32x32ep   the same as above but using 72-bit accumulator for intermediate 
            computations
  f         floating point

  Input:
  x[N]      input data, Q15, Q31 or floating point
  N         length of x
  Output:
  r[N]      output data, Q15, Q31 or floating point

  Restrictions:
  x,r       should not overlap
  x,r       aligned on an 16-bytes boundary
  N         multiple of 4 and >0
-------------------------------------------------------------------------*/
void fir_acorr32x32( int32_t * restrict r,
               const int32_t * restrict x,
               int N )
{
    fir_xcorr32x32( r,x,x,N,N );
}
