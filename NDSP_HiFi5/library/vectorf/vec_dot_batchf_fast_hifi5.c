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
  NatureDSP Signal Processing Library. Vector Operations
    Common Exponent 
    C code optimized for HiFi5
*/
/* Signal Processing Library API. */
#include "NatureDSP_Signal_vector.h"
#include "common.h"
#include "common_fpu.h"

#if (HAVE_VFPU==0 && HAVE_FPU==0)
DISCARD_FUN(void,vec_dot_batchf_fast,(float32_t *z, const float32_t   * x,const cfloat32ptr_t * y, int N, int M))
#elif (HAVE_VFPU)
/*-------------------------------------------------------------------------
  Batch Computation of Vector Dot products
  These routines take a set of input vectors and compute their dot product 
  with specific reference data.
  Two versions of routines are available: 
  - regular versions (vec_dot_batch8x8, vec_dot_batch8x16, vec_dot_batch16x16, 
    vec_dot_batchf). They work with arbitratry arguments
  - fast versions (vec_dot_batch8x8_fast, vec_dot_batch8x16_fast, 
    vec_dot_batch16x16_fast, vec_dot_batchf_fast) apply some restrictions.  

  Precision: 
  8x8    8x8-bit data, 16-bit output (fractional multiply Q7xQ7->Q15)
  8x16   8x16-bit data, 16-bit output (fractional multiply Q7xQ15->Q15)
  16x16  16x16-bit data, 32-bit output (fractional multiply Q15xQ15->Q31)
  f      single precision floating point
  fp16   half precision floating point

  Input:
  x[N]     input (reference) data, Q7, Q15 or floating point
  y[M][N]  pointers to M input vectors, Q7, Q15 or floating point
  N        length of vectors
  M        number of vectors
  rsh      right shift for output (for fixed point API only!)
  Output:
  z[M]     dot products between references and M inputs, Q15, Q31 or 
           floating point

  Restrictions:
  Regular versions:
    none
  Faster versions:
    x,y[m] - aligned on 16-byte boundary
    N      - multiple of 8
    M        multiple of 4
-------------------------------------------------------------------------*/
void vec_dot_batchf_fast    (float32_t *z, const float32_t   * x,const cfloat32ptr_t * y        , int N, int M)
{
  xtfloatx2 x0, y0, y1, y2, y3, vacc0, vacc1, vacc2, vacc3;
  xtfloatx2 x1, y4, y5, y6, y7, vacc4, vacc5, vacc6, vacc7;
  xtfloat acc0, acc1, acc2, acc3;
  const xtfloatx4 * restrict px = (const xtfloatx4 *)x;
  const xtfloatx4 * restrict py0;
  const xtfloatx4 * restrict py1;
  const xtfloatx4 * restrict py2;
  const xtfloatx4 * restrict py3;
  xtfloat * restrict pz = (xtfloat *)z;
  int m, n;
  NASSERT(z);  NASSERT_ALIGN(z, HIFI_SIMD_WIDTH);
  NASSERT(x);  NASSERT_ALIGN(x, HIFI_SIMD_WIDTH);
  NASSERT(y);
  NASSERT(M % 4 == 0 && N % 8 == 0);
  if (M <= 0) return;
  if (N == 0)
  {
    for (m = 0; m<M; m++) z[m] = 0.f;
    return;
  }
  for (m = 0; m < (M>>2); m++)
  {
    px  = (const xtfloatx4 *)x;
    py0 = (const xtfloatx4* )(y[4*m+0]);
    py1 = (const xtfloatx4* )(y[4*m+1]);
    py2 = (const xtfloatx4*)(y[4*m+2]);
    py3 = (const xtfloatx4*)(y[4*m+3]);
    MOV_SX2X2(vacc0, vacc1, 0.f, 0.f);
    MOV_SX2X2(vacc2, vacc3, 0.f, 0.f);
    MOV_SX2X2(vacc4, vacc5, 0.f, 0.f);
    MOV_SX2X2(vacc6, vacc7, 0.f, 0.f);
    for (n = 0; n < (N >> 2); n++)
    {
      AE_LSX2X2_IP(x0, x1, px, 4*sizeof(float32_t));           
      AE_LSX2X2_IP(y0, y4, py0, 4 * sizeof(float32_t));
      AE_LSX2X2_IP(y1, y5, py1, 4 * sizeof(float32_t));
      AE_LSX2X2_IP(y2, y6, py2, 4 * sizeof(float32_t));
      AE_LSX2X2_IP(y3, y7, py3, 4 * sizeof(float32_t));
      MADDQ_S(vacc0, vacc1, y0, y1, x0);
      MADDQ_S(vacc2, vacc3, y2, y3, x0);
      MADDQ_S(vacc4, vacc5, y4, y5, x1);
      MADDQ_S(vacc6, vacc7, y6, y7, x1);
    }
    ADD_SX2X2(vacc0, vacc1, vacc0, vacc1, vacc4, vacc5);
    ADD_SX2X2(vacc2, vacc3, vacc2, vacc3, vacc6, vacc7);
    acc0 = XT_RADD_SX2(vacc0);
    XT_SSIP(acc0, pz, sizeof(xtfloat));
    acc1 = XT_RADD_SX2(vacc1);
    XT_SSIP(acc1, pz, sizeof(xtfloat));
    acc2 = XT_RADD_SX2(vacc2);
    XT_SSIP(acc2, pz, sizeof(xtfloat));
    acc3 = XT_RADD_SX2(vacc3);
    XT_SSIP(acc3, pz, sizeof(xtfloat));
  }
}
#else //SFPU
void vec_dot_batchf_fast(float32_t *z, const float32_t   * x, const cfloat32ptr_t * y, int N, int M)
{
  xtfloat acc0, acc1, acc2, acc3, x0, y0, y1, y2, y3;
  int m, n;
  NASSERT(z);
  NASSERT(x);
  NASSERT(y);
  NASSERT(M % 4 == 0 && N % 8 == 0);
  const xtfloat * restrict px = (const xtfloat*)x;
  const xtfloat * restrict py0;
  const xtfloat * restrict py1;
  const xtfloat * restrict py2;
  const xtfloat * restrict py3;
  xtfloat * restrict pz = (xtfloat *)z;
  if (M <= 0) return;
  if (N == 0)
  {
    for (m = 0; m<M; m++) z[m] = 0.f;
    return;
  }
  for (m = 0; m<M; m += 4)
  {
    px = (const xtfloat *)x;
    py0 = (const xtfloat*)(y[m]);
    py1 = (const xtfloat*)(y[m + 1]);
    py2 = (const xtfloat*)(y[m + 2]);
    py3 = (const xtfloat*)(y[m + 3]);
    acc0 = acc1 = XT_CONST_S(0);
    acc2 = acc3 = XT_CONST_S(0);
    for (n = 0; n<N; n++)
    {
      XT_LSIP(x0, px, sizeof(xtfloat));
      XT_LSIP(y0, py0, sizeof(xtfloat));
      XT_LSIP(y1, py1, sizeof(xtfloat));
      XT_MADD_S(acc0, x0, y0);
      XT_MADD_S(acc1, x0, y1);
      XT_LSIP(y2, py2, sizeof(xtfloat));
      XT_LSIP(y3, py3, sizeof(xtfloat));
      XT_MADD_S(acc2, x0, y2);
      XT_MADD_S(acc3, x0, y3);
    }
    XT_SSIP(acc0, pz, sizeof(xtfloat));
    XT_SSIP(acc1, pz, sizeof(xtfloat));
    XT_SSIP(acc2, pz, sizeof(xtfloat));
    XT_SSIP(acc3, pz, sizeof(xtfloat));
  }
}

#endif
