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
  NatureDSP Signal Processing Library. FFT part
    Complex-valued FFT stages with butterflies radix-2, radix-3, radix-5
    with dynamic data scaling: 32-bit data, 16-bit twiddle factors
    C code optimized for HiFi5
  IntegrIT, 2006-2019
*/
#include "NatureDSP_types.h"
#include "common.h"
#include "fft_32x16_stages.h"

/*
DFT3 algorithm:
x - input complex vector
y - output complex vector
y = fft(x)
y = [ x(1) + x(2)  + x(3);
x(1) + (x(2) + x(3))*cos(2*pi/3) - 1j*(x(2) - x(3))*sin(2*pi/3);
x(1) + (x(2) + x(3))*cos(2*pi/3) + 1j*(x(2) - x(3))*sin(2*pi/3) ]

*/

/* radix-3 butterfly with data scaling using AE.SAR register & 'shift' value */
#define DFT3X1_RNG_shift(x0, x1, x2, shift)\
{\
    ae_int32x2 s0, s1, d0, c32; \
    ae_int16x4 c;               \
    c32 = AE_MOVDA32X2(0x40004000,0x00006EDA); \
    c = AE_MOVINT16X4_FROMINT32X2(c32);        \
    x0 = AE_SRAA32(x0, shift);                 \
    AE_ADDANDSUBRNG32(s0, d0, x1, x2);         \
    s1 = AE_ADD32S(x0, s0);                    \
    AE_MULSFP32X16X2RAS_H(x0, s0, c);          \
    d0 = AE_MULFC32X16RAS_L(d0, c);            \
    s0 = x0;                                   \
    x0 = s1;                                   \
    AE_ADDANDSUB32S(x2, x1, s0, d0);           \
}

/*
 *  32x16 FFT/IFFT intermediate stage Radix 3, scalingOption=2
 *  Works only for v=4 !
 */
int fft_32x16_stage_inner_scl2_DFT3(const int16_t *tw, const int32_t *x, int32_t *y, int N, int *v, int tw_step, int *bexp)
{
  const int R = 3; // stage radix
  const int stride = N / R;
  int min_shift = 3;
  int i, shift;
  ae_int32x2 * restrict px0;
  ae_int32x2 * restrict px1;
  ae_int32x2 * restrict px2;        
  ae_int32x2 * restrict py0;
  const ae_int16x4 * restrict ptwd = (const ae_int16x4 *)tw;

  shift = XT_MAX(0, min_shift - *bexp);
  px0 = (ae_int32x2 *)x;
  px1 = (ae_int32x2 *)x + stride;
  px2 = (ae_int32x2 *)x + 2 * stride;
  py0 = (ae_int32x2 *)y;

  NASSERT_ALIGN16(x);
  NASSERT_ALIGN16(y);
  NASSERT_ALIGN16(px1);
  NASSERT_ALIGN16(px2);
  NASSERT(tw_step == 1);
  NASSERT(v[0] == 4);

  WUR_AE_SAR(shift);

  for (i = 0; i< (stride >> 2); i++)
  {
      
      ae_int16x4 tw1;
      ae_int32x2 x00, x10, x20;
      ae_int32x2 x01, x11, x21;
      ae_int32x2 x02, x12, x22;
      ae_int32x2 x03, x13, x23;

      AE_L16X4_IP(tw1, ptwd, sizeof(tw1));

      AE_L32X2X2_IP(x00, x01, castxcc(ae_int32x4, px0), 2 * sizeof(ae_int32x2));
      AE_L32X2X2_IP(x10, x11, castxcc(ae_int32x4, px1), 2 * sizeof(ae_int32x2));
      AE_L32X2X2_IP(x20, x21, castxcc(ae_int32x4, px2), 2 * sizeof(ae_int32x2));
      AE_L32X2X2_IP(x02, x03, castxcc(ae_int32x4, px0), 2 * sizeof(ae_int32x2));
      AE_L32X2X2_IP(x12, x13, castxcc(ae_int32x4, px1), 2 * sizeof(ae_int32x2));
      AE_L32X2X2_IP(x22, x23, castxcc(ae_int32x4, px2), 2 * sizeof(ae_int32x2));

      DFT3X1_RNG_shift(x00, x10, x20, shift);
      DFT3X1_RNG_shift(x01, x11, x21, shift);
      DFT3X1_RNG_shift(x02, x12, x22, shift);
      DFT3X1_RNG_shift(x03, x13, x23, shift);

      x10 = AE_MULFC32X16RAS_H(x10, tw1);
      x11 = AE_MULFC32X16RAS_H(x11, tw1);
      x12 = AE_MULFC32X16RAS_H(x12, tw1);
      x13 = AE_MULFC32X16RAS_H(x13, tw1);

      x20 = AE_MULFC32X16RAS_L(x20, tw1);
      x21 = AE_MULFC32X16RAS_L(x21, tw1);
      x22 = AE_MULFC32X16RAS_L(x22, tw1);
      x23 = AE_MULFC32X16RAS_L(x23, tw1);

      AE_S32X2X2RNG_IP(x00, x01, castxcc(ae_int32x4, py0), 2 * sizeof(ae_int32x2));
      AE_S32X2X2RNG_IP(x02, x03, castxcc(ae_int32x4, py0), 2 * sizeof(ae_int32x2));
      AE_S32X2X2RNG_IP(x10, x11, castxcc(ae_int32x4, py0), 2 * sizeof(ae_int32x2));
      AE_S32X2X2RNG_IP(x12, x13, castxcc(ae_int32x4, py0), 2 * sizeof(ae_int32x2));
      AE_S32X2X2RNG_IP(x20, x21, castxcc(ae_int32x4, py0), 2 * sizeof(ae_int32x2));
      AE_S32X2X2RNG_IP(x22, x23, castxcc(ae_int32x4, py0), 2 * sizeof(ae_int32x2));
  }

  AE_CALCRNG3();
  *bexp = 3 - RUR_AE_SAR();
  *v *= R;
  return shift;
} /* fft_32x16_stage_inner_scl2_DFT3() */
