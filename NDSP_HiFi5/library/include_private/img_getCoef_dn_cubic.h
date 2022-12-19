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
#ifndef IMG_COEF_DN_CUBIC_H__
#define IMG_COEF_DN_CUBIC_H__
#include "NatureDSP_types.h"
#include "NatureDSP_Signal_img.h"
#include "common.h"
/* coefficients for bilinear downsampler with ratio 1..2 
   M - input length, N - output length, 1<M/N<2
*/

typedef struct
{
    int16_t   *coef;    // [8][N] coefficients, aligned
    int16_t   *left;    // [N]    indices, aligned
}
img_coefdn_cubic_t;

size_t img_getCoef_dn_cubic_alloc(int M,int N);

/* function returns w(N,8) coefficients and left(N) indices
  for downsampler
  M - input length, N - output length, 1<M/N<2
*/
img_coefdn_cubic_t* img_getCoef_dn_cubic_init(void* objmem,int M,int N);

#endif
