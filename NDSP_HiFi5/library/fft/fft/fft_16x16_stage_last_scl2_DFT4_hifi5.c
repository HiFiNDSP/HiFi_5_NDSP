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
    Complex-valued FFT stages with butterflies radix-4, radix-8
    with dynamic data scaling: 16-bit data, 16-bit twiddle factors
    C code optimized for HiFi5
  IntegrIT, 2006-2019
*/
#include "NatureDSP_types.h"
#include "common.h"
#include "fft_16x16_stages.h"

/*
    Set scaling for DFT4
    Range of the 'scale' is 0...3 
*/
#define SetDFT4_Scaling(scale)                \
{                                             \
    int sar;                                  \
    NASSERT(scale>=0 && scale<=3);            \
    /*(!"DFT4XI2: scale is out of range"); */ \
    if (scale == 3)        sar = 0x285;       \
    else if (scale == 2)   sar = 0x183;       \
    else if (scale == 1)   sar = 0x102;       \
    else sar = 0;                             \
    WUR_AE_SAR(sar);                          \
}

/*  16-bit radix-4 butterfly with scaling. 
    Call SetDFT4_Scaling() before */
#define DFT4XI2(_x0, _x1, _x2, _x3)               \
{                                                 \
    ae_int16x4 s0, s1, d0, d1;                    \
    s0 = _x0;    s1 = _x1;                        \
    d0 = _x2;    d1 = _x3;                        \
    AE_ADDANDSUBRNG16RAS_S1(s0, d0);              \
    AE_ADDANDSUBRNG16RAS_S1(s1, d1);              \
    d1 = AE_MUL16JS(d1);                          \
    AE_ADDANDSUBRNG16RAS_S2(s0, s1);              \
    AE_ADDANDSUBRNG16RAS_S2(d0, d1);              \
    _x0 = s0;    _x2 = s1;                        \
    _x3 = d0;    _x1 = d1;                        \
}

/*
 *  Last stage of FFT/IFFT 16x16, radix-4, dynamic scaling
 */
int fft_16x16_stage_last_scl2_DFT4(const int16_t *tw, const int16_t *x, int16_t *y, int N, int *v, int tw_step, int *bexp)
{
  const ae_int16x4 * restrict px0;
  const ae_int16x4 * restrict px1;
  const ae_int16x4 * restrict px2;
  const ae_int16x4 * restrict px3;
        ae_int16x4 * restrict py0;
        ae_int16x4 * restrict py1;
        ae_int16x4 * restrict py2;
        ae_int16x4 * restrict py3;
  const int stride = (N >> 2);
  int j, shift;
  NASSERT_ALIGN16(x);
  NASSERT_ALIGN16(y);
  NASSERT(N%8==0);
  shift = XT_MAX(0,2 - *bexp);

  px0 = (const ae_int16x4 *)x;
  px1 = (const ae_int16x4 *)((complex_fract16*)px0 + stride);
  px2 = (const ae_int16x4 *)((complex_fract16*)px1 + stride);
  px3 = (const ae_int16x4 *)((complex_fract16*)px2 + stride);
  py0 = (      ae_int16x4 *)y;
  py1 = (      ae_int16x4 *)((complex_fract16*)py0 + stride);
  py2 = (      ae_int16x4 *)((complex_fract16*)py1 + stride);
  py3 = (      ae_int16x4 *)((complex_fract16*)py2 + stride);

  SetDFT4_Scaling(shift);

  if ((stride&3)==0)
  {
      __Pragma("loop_count min=1");
      for (j = 0; j < (stride>>2); j++)
      {
          /* 8 cycles per pipeline stage in steady state with unroll=2 */
        ae_int16x4 x0[2], x1[2], x2[2], x3[2];

        AE_L16X4X2_IP(x0[0],x0[1], castxcc(ae_int16x8,px0), sizeof(ae_int16x8));
        AE_L16X4X2_IP(x1[0],x1[1], castxcc(ae_int16x8,px1), sizeof(ae_int16x8));
        AE_L16X4X2_IP(x2[0],x2[1], castxcc(ae_int16x8,px2), sizeof(ae_int16x8));
        AE_L16X4X2_IP(x3[0],x3[1], castxcc(ae_int16x8,px3), sizeof(ae_int16x8));

        DFT4XI2(x0[0], x1[0], x2[0], x3[0]);
        DFT4XI2(x0[1], x1[1], x2[1], x3[1]);

        AE_S16X4X2_IP(x0[0],x0[1], castxcc(ae_int16x8,py0), sizeof(ae_int16x8));
        AE_S16X4X2_IP(x1[0],x1[1], castxcc(ae_int16x8,py1), sizeof(ae_int16x8));
        AE_S16X4X2_IP(x2[0],x2[1], castxcc(ae_int16x8,py2), sizeof(ae_int16x8));
        AE_S16X4X2_IP(x3[0],x3[1], castxcc(ae_int16x8,py3), sizeof(ae_int16x8));
      }
  }
  else
  { // unaligned variant
        ae_int16x4 x0[2], x1[2], x2[2], x3[2];
        for (j = 0; j < (stride>>2); j++)
        {
            AE_L16X4X2_IP(x0[0],x0[1], castxcc(ae_int16x8,px0), sizeof(ae_int16x8));
            AE_L16X4_IP  (x1[0],px1,sizeof(ae_int16x4)); 
            AE_L16X4_IP(x1[1],px1,sizeof(ae_int16x4));
            AE_L16X4X2_IP(x2[0],x2[1], castxcc(ae_int16x8,px2), sizeof(ae_int16x8));
            AE_L16X4_IP  (x3[0],px3,sizeof(ae_int16x4)); 
            AE_L16X4_IP  (x3[1],px3,sizeof(ae_int16x4));

            DFT4XI2(x0[0], x1[0], x2[0], x3[0]);
            DFT4XI2(x0[1], x1[1], x2[1], x3[1]);

            AE_S16X4X2_IP(x0[0],x0[1], castxcc(ae_int16x8,py0), sizeof(ae_int16x8));
            AE_S16X4_IP  (x1[0],py1, sizeof(ae_int16x4)); 
            AE_S16X4_IP  (x1[1],py1, sizeof(ae_int16x4));
            AE_S16X4X2_IP(x2[0],x2[1], castxcc(ae_int16x8,py2), sizeof(ae_int16x8));
            AE_S16X4_IP  (x3[0],py3, sizeof(ae_int16x4)); 
            AE_S16X4_IP  (x3[1],py3, sizeof(ae_int16x4));
        }
        x0[0]=AE_L16X4_I(px0,0);
        x1[0]=AE_L16X4_I(px1,0);
        x2[0]=AE_L16X4_I(px2,0);
        x3[0]=AE_L16X4_I(px3,0);
        DFT4XI2(x0[0], x1[0], x2[0], x3[0]);
        AE_S16X4_I(x0[0],py0,0);
        AE_S16X4_I(x1[0],py1,0); 
        AE_S16X4_I(x2[0],py2,0);
        AE_S16X4_I(x3[0],py3,0); 
  }
  return shift;

} /* fft_16x16_stage_last_scl2_DFT4() */
