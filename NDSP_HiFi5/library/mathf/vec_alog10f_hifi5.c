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

/* DSP Library API */
/*    Code optimized for HiFi5 core */
#include "NatureDSP_Signal_math.h" 
/* Common helper macros. */
#include "common_fpu.h"
/* Tables */
#include "expf_tbl.h"
#include "alog10f_tbl.h"
/* +/-Infinity, single precision */
#include "inff_tbl.h"
/* sNaN/qNaN, single precision. */
#include "nanf_tbl.h"

#if !HAVE_VFPU && !HAVE_FPU
DISCARD_FUN(void,vec_antilog10f,( float32_t * restrict y, const float32_t* restrict x, int N ))
#elif HAVE_VFPU
/*
  NatureDSP Signal Processing Library. Vector Mathematics
   Antilogarithm, base 10
    Code optimized for HiFi5
  IntegrIT, 2006-2019
*/

/*-------------------------------------------------------------------------
  Antilogarithm
  These routines calculate antilogarithm (base2, natural and base10). 
  Fixed-point functions accept inputs in Q25 and form outputs in Q16.15 
  format and return 0x7FFFFFFF in case of overflow and 0 in case of 
  underflow.

  Precision:
  32x32  32-bit inputs, 32-bit outputs. Accuracy: 8*e-6*y+1LSB
  f      floating point: Accuracy: 2 ULP
  NOTE:
  1.  Although 32 and 24 bit functions provide the similar accuracy, 32-bit
      functions have better input/output resolution (dynamic range).
  2.  Floating point functions are compatible with standard ANSI C routines 
      and set errno and exception flags accordingly.

  Input:
  x[N]  input data,Q25 or floating point 
  N     length of vectors
  Output:
  y[N]  output data,Q16.15 or floating point  

  Restriction:
  x,y should not overlap

  Scalar versions:
  ----------------
  fixed point functions return result in Q16.15

  PERFORMANCE NOTE:
  for optimum performance follow rules:
  x,y - aligned on 16-byte boundary
  N   - multiple of 2
-------------------------------------------------------------------------*/

void vec_antilog10f (float32_t * restrict y, const float32_t* restrict x, int N)
{
    const xtfloatx4 *          X = (const xtfloatx4*)x;
    const xtfloatx4 *          X1 = (const xtfloatx4*)x;
          xtfloatx4 * restrict Y = (        xtfloatx4*)y;
    const ae_int32  *          TBL = (ae_int32 *)expftbl_Q30;
    ae_valignx2 X_va, X1_va, Y_va;

    xtfloatx2 x0, x1, y0, y1, z0, z1, x2, x3;
    ae_int32x2 tb0, tb1, tb2, tb3, tb4, tb5, tb6;
    ae_int32x2 u0, u1, n0, n1;
    ae_int32x2 t0_0, t0_1, t1_0, t1_1, t2_0, t2_1;
    ae_int32x2 e0_0, e0_1, e1_0, e1_1, e0, e1;
    ae_f32x2 f0, f1;
    xtbool2 b0_nan, b1_nan;
    int n;
    ae_int64 wh0, wl0, wh1, wl1;

    if (N <= 0) return;

    X_va = AE_LA128_PP(X);
    X1_va = AE_LA128_PP(X1);
    Y_va = AE_ZALIGN128();
    for (n = 0; n<(N >> 2); n++)
    {
        AE_LASX2X2_IP(x0, x1, X_va, X);

        x0 = XT_MAX_SX2(alog10fminmax[0].f, x0);
        x0 = XT_MIN_SX2(alog10fminmax[1].f, x0);
        x1 = XT_MAX_SX2(alog10fminmax[0].f, x1);
        x1 = XT_MIN_SX2(alog10fminmax[1].f, x1);

        u0 = XT_TRUNC_SX2(x0, 25);
        u1 = XT_TRUNC_SX2(x1, 25);

        /* scale input to 1/log10(2) and convert to Q31 */
        AE_MUL32X2S_HH_LL(wh0, wl0, u0, invlog10_2_Q29);
        AE_MUL32X2S_HH_LL(wh1, wl1, u1, invlog10_2_Q29);
        e0 = AE_TRUNCA32X2F64S(wh0, wl0, -22);
        e1 = AE_TRUNCA32X2F64S(wh1, wl1, -22);
        wh0 = AE_SLLI64(wh0, 32 - 22);
        wl0 = AE_SLLI64(wl0, 32 - 22);
        wh1 = AE_SLLI64(wh1, 32 - 22);
        wl1 = AE_SLLI64(wl1, 32 - 22);
        u0 = AE_TRUNCI32X2F64S(wh0, wl0, 0);
        u1 = AE_TRUNCI32X2F64S(wh1, wl1, 0);
        u0 = AE_SRLI32(u0, 1);
        u1 = AE_SRLI32(u1, 1);

        tb0 = AE_L32_I(TBL, 0 * 4);
        tb1 = AE_L32_I(TBL, 1 * 4);
        tb2 = AE_L32_I(TBL, 2 * 4);
        tb3 = AE_L32_I(TBL, 3 * 4);
        tb4 = AE_L32_I(TBL, 4 * 4);
        tb5 = AE_L32_I(TBL, 5 * 4);
        tb6 = AE_L32_I(TBL, 6 * 4);

        /*
        * Compute 2^t in Q30 where t is in Q31. Use a combination of Estrin's
        * method and Horner's scheme to evaluate the fixed-point polynomial.
        */

        t0_0 = tb2; t0_1 = tb2;
        AE_MULAF2P32X4RAS(t0_0, t0_1, u0, u1, tb1, tb1);
        t1_0 = tb4; t1_1 = tb4;
        AE_MULAF2P32X4RAS(t1_0, t1_1, u0, u1, tb3, tb3);
        t2_0 = tb6; t2_1 = tb6;
        AE_MULAF2P32X4RAS(t2_0, t2_1, u0, u1, tb5, tb5);

        AE_MULF2P32X4RAS(u0, u1, u0, u1, u0, u1);

        f0 = t0_0; f1 = t0_1;
        AE_MULAF2P32X4RAS(f0, f1, u0, u1, tb0, tb0);
        n0 = f0; n1 = f1;
        f0 = t1_0; f1 = t1_1;
        AE_MULAF2P32X4RAS(f0, f1, u0, u1, n0, n1);
        n0 = f0; n1 = f1;
        f0 = t2_0; f1 = t2_1;
        AE_MULAF2P32X4RAS(f0, f1, u0, u1, n0, n1);

        /* convert back to the floating point  */
        x0 = XT_FLOAT_SX2(f0, 30);
        x1 = XT_FLOAT_SX2(f1, 30);

        e0_0 = AE_SRAI32(e0, 1);
        e0_1 = AE_SRAI32(e1, 1);
        e1_0 = AE_SUB32(e0,e0_0);
        e1_1 = AE_SUB32(e1,e0_1);
        y0=FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e0_0));
        y1=FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e0_1));
        z0=FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e1_0));
        z1=FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e1_1));

        AE_LASX2X2_IP(x2, x3, X1_va, X1);
        b0_nan = XT_UN_SX2(x2, x2);
        b1_nan = XT_UN_SX2(x3, x3);
        XT_MOVT_SX2(x0, x2, b0_nan);
        XT_MOVT_SX2(x1, x3, b1_nan);

        MUL_SX2X2(y0, y1, y0, y1, z0, z1);
        MUL_SX2X2(y0, y1, x0, x1, y0, y1);

        AE_SASX2X2_IP(y0, y1, Y_va, Y);
  }

    AE_SA128POS_FP(Y_va, Y);
    x += (N&~3);
    y += (N&~3);
    N &= 3;
    if (N>0)
    {
      xtfloatx2 x0, x1, e0, e1;
      int32_t ALIGN(32) scratch[4];
      xtfloatx4 *pScr;
      pScr = (xtfloatx4*)scratch;
      X = (const xtfloatx4*)x;
      Y = (      xtfloatx4*)y;
      AE_SSX2X2_IP(0, 0, pScr, 0);
      __Pragma("no_unroll")
      for (n = 0; n<N; n++)
      {
        xtfloat t;
        AE_LSIP(t, castxcc(xtfloat, X), sizeof(xtfloat));
        AE_SSIP(t, castxcc(xtfloat, pScr), sizeof(xtfloat));
      }
      pScr = (xtfloatx4*)scratch;
      AE_LSX2X2_IP(x0, x1, pScr, 0);
      
      x0 = XT_MAX_SX2(alog10fminmax[0].f, x0);
      x0 = XT_MIN_SX2(alog10fminmax[1].f, x0);
      x1 = XT_MAX_SX2(alog10fminmax[0].f, x1);
      x1 = XT_MIN_SX2(alog10fminmax[1].f, x1);

      u0 = XT_TRUNC_SX2(x0, 25);
      u1 = XT_TRUNC_SX2(x1, 25);

      /* scale input to 1/log10(2) and convert to Q31 */
      AE_MUL32X2S_HH_LL(wh0, wl0, u0, invlog10_2_Q29);
      AE_MUL32X2S_HH_LL(wh1, wl1, u1, invlog10_2_Q29);
      e0 = AE_TRUNCA32X2F64S(wh0, wl0, -22);
      e1 = AE_TRUNCA32X2F64S(wh1, wl1, -22);
      wh0 = AE_SLLI64(wh0, 32 - 22);
      wl0 = AE_SLLI64(wl0, 32 - 22);
      wh1 = AE_SLLI64(wh1, 32 - 22);
      wl1 = AE_SLLI64(wl1, 32 - 22);
      u0 = AE_TRUNCI32X2F64S(wh0, wl0, 0);
      u1 = AE_TRUNCI32X2F64S(wh1, wl1, 0);
      u0 = AE_SRLI32(u0, 1);
      u1 = AE_SRLI32(u1, 1);

      tb0 = AE_L32_I(TBL, 0 * 4);
      tb1 = AE_L32_I(TBL, 1 * 4);
      tb2 = AE_L32_I(TBL, 2 * 4);
      tb3 = AE_L32_I(TBL, 3 * 4);
      tb4 = AE_L32_I(TBL, 4 * 4);
      tb5 = AE_L32_I(TBL, 5 * 4);
      tb6 = AE_L32_I(TBL, 6 * 4);

      /*
      * Compute 2^t in Q30 where t is in Q31. Use a combination of Estrin's
      * method and Horner's scheme to evaluate the fixed-point polynomial.
      */

      t0_0 = tb2; t0_1 = tb2;
      AE_MULAF2P32X4RAS(t0_0, t0_1, u0, u1, tb1, tb1);
      t1_0 = tb4; t1_1 = tb4;
      AE_MULAF2P32X4RAS(t1_0, t1_1, u0, u1, tb3, tb3);
      t2_0 = tb6; t2_1 = tb6;
      AE_MULAF2P32X4RAS(t2_0, t2_1, u0, u1, tb5, tb5);

      AE_MULF2P32X4RAS(u0, u1, u0, u1, u0, u1);

      f0 = t0_0; f1 = t0_1;
      AE_MULAF2P32X4RAS(f0, f1, u0, u1, tb0, tb0);
      n0 = f0; n1 = f1;
      f0 = t1_0; f1 = t1_1;
      AE_MULAF2P32X4RAS(f0, f1, u0, u1, n0, n1);
      n0 = f0; n1 = f1;
      f0 = t2_0; f1 = t2_1;
      AE_MULAF2P32X4RAS(f0, f1, u0, u1, n0, n1);

      /* convert back to the floating point  */
      x0 = XT_FLOAT_SX2(f0, 30);
      x1 = XT_FLOAT_SX2(f1, 30);

      e0_0 = AE_SRAI32(e0, 1);
      e0_1 = AE_SRAI32(e1, 1);
      e1_0 = AE_SUB32(e0, e0_0);
      e1_1 = AE_SUB32(e1, e0_1);
      y0 = FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e0_0));
      y1 = FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e0_1));
      z0 = FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e1_0));
      z1 = FLOATEXP_SX2(AE_MOVINT8X8_FROMINT32X2(e1_1));

      AE_LASX2X2_IP(x2, x3, X1_va, X1);
      b0_nan = XT_UN_SX2(x2, x2);
      b1_nan = XT_UN_SX2(x3, x3);
      XT_MOVT_SX2(x0, x2, b0_nan);
      XT_MOVT_SX2(x1, x3, b1_nan);

      MUL_SX2X2(y0, y1, y0, y1, z0, z1);
      MUL_SX2X2(y0, y1, x0, x1, y0, y1);
      AE_SSX2X2_I(y0, y1, pScr, 0 * sizeof(xtfloatx4));
      __Pragma("no_unroll")
      for (n = 0; n<N; n++)
      {
        xtfloat t;
        AE_LSIP(t, castxcc(xtfloat, pScr), sizeof(xtfloat));
        AE_SSIP(t, castxcc(xtfloat, Y), sizeof(xtfloat));
       }
    }
} /* vec_antilog10f() */
#elif HAVE_FPU
#define sz_i32 (int)sizeof(int32_t)
#define sz_f32 (int)sizeof(float32_t)

/*===========================================================================
  Vector matematics:
  vec_antilog          Antilogarithm         
===========================================================================*/

/*-------------------------------------------------------------------------
  Antilogarithm
  These routines calculate antilogarithm (base2, natural and base10). 
  Fixed-point functions accept inputs in Q25 and form outputs in Q16.15 
  format and return 0x7FFFFFFF in case of overflow and 0 in case of 
  underflow.

  Precision:
  32x32  32-bit inputs, 32-bit outputs. Accuracy: 4*e-5*y+1LSB
  f      floating point: Accuracy: 2 ULP
  NOTE:
  1.  Floating point functions are compatible with standard ANSI C routines 
      and set errno and exception flags accordingly

  Input:
  x[N]  input data,Q25 or floating point 
  N     length of vectors
  Output:
  y[N]  output data,Q16.15 or floating point  

  Restriction:
  x,y should not overlap

  Scalar versions:
  ----------------
  fixed point functions return result in Q16.15

-------------------------------------------------------------------------*/

void vec_antilog10f( float32_t * restrict y, const float32_t* restrict x, int N )
{
  const xtfloat  *          X0  = (xtfloat*)x;
  const xtfloat  *          X1  = (xtfloat*)x;
  const ae_int32 *          TBL = (ae_int32*)expftbl_Q30;
        xtfloat  * restrict Y   = (xtfloat*)y;

  xtfloat    x0, x1, x0_, x1_, y0, y1, z0, z1;
  ae_int32x2 tb0, tb1, tb2, tb3, tb4, tb5, tb6;
  int32_t    e0, e1, n0, n1, u0, u1;
  xtbool     b_nan0, b_nan1;
  ae_int64   w0, w1, r0, r1;

  int n;

  if ( N<=0 ) return;

  for ( n=0; n<(N>>1); n++ )
  {
    ae_f32x2   v01, s01, f01, m01;
    ae_int32x2 e01, g01, u01, t01;

	XT_LSIP(x0, X0, sz_f32);
	XT_LSIP(x1, X0, sz_f32);

	/* Check borders. */
	x0 = XT_MAX_S(alog10fminmax[0].f, x0);
	x0 = XT_MIN_S(alog10fminmax[1].f, x0);
	x1 = XT_MAX_S(alog10fminmax[0].f, x1);
	x1 = XT_MIN_S(alog10fminmax[1].f, x1);

    u0 = XT_TRUNC_S( XT_MUL_S( x0, XT_FLOAT_S( 1<<10, 0 ) ), 15 );
    u1 = XT_TRUNC_S( XT_MUL_S( x1, XT_FLOAT_S( 1<<10, 0 ) ), 15 );

    /* scale input to 1/log10(2) and convert to Q31 */
    w0 = AE_MUL32_HH(u0, invlog10_2_Q29);
    w1 = AE_MUL32_HH(u1, invlog10_2_Q29);
    e0 = ae_int32x2_rtor_int32(AE_TRUNCA32X2F64S(w0, w0, -22));
    e1 = ae_int32x2_rtor_int32(AE_TRUNCA32X2F64S(w1, w1, -22));
    r0 = AE_SLLI64(w0, 32-22);
    r1 = AE_SLLI64(w1, 32-22);
    u0 = ae_int32x2_rtor_int32(AE_TRUNCI32X2F64S(r0, r0, 0));
    u1 = ae_int32x2_rtor_int32(AE_TRUNCI32X2F64S(r1, r1, 0));
    u01 = AE_MOVDA32X2(u1, u0);
	u01 = AE_SRLI32(u01, 1);

    tb0 = AE_L32_I(TBL, 0*sz_f32);
    tb1 = AE_L32_I(TBL, 1*sz_f32);
    tb2 = AE_L32_I(TBL, 2*sz_f32);
    tb3 = AE_L32_I(TBL, 3*sz_f32);
    tb4 = AE_L32_I(TBL, 4*sz_f32);
    tb5 = AE_L32_I(TBL, 5*sz_f32);
    tb6 = AE_L32_I(TBL, 6*sz_f32);

	/*
     * Compute 2^t in Q30 where t is in Q31. Use a combination of Estrin's
     * method and Horner's scheme to evaluate the fixed-point polynomial.                                     
     */

	v01 = AE_MOVF32X2_FROMINT32X2(u01);

	f01 = tb2; AE_MULAFP32X2RAS(f01, tb1, v01); tb1 = f01;
	f01 = tb4; AE_MULAFP32X2RAS(f01, tb3, v01); tb2 = f01;
	f01 = tb6; AE_MULAFP32X2RAS(f01, tb5, v01); tb3 = f01;

	s01 = AE_MULFP32X2RAS(v01, v01);

	f01 = tb0;                                  m01 = f01;
	f01 = tb1; AE_MULAFP32X2RAS(f01, s01, m01); m01 = f01;
	f01 = tb2; AE_MULAFP32X2RAS(f01, s01, m01); m01 = f01;
	f01 = tb3; AE_MULAFP32X2RAS(f01, s01, m01); m01 = f01;

    n0 = AE_MOVAD32_L(m01);
    n1 = AE_MOVAD32_H(m01);

    /* convert back to the floating point  */
    x0 = XT_MUL_S( XT_FLOAT_S( n0, 15 ), XT_FLOAT_S( 1, 15 ) );
    x1 = XT_MUL_S( XT_FLOAT_S( n1, 15 ), XT_FLOAT_S( 1, 15 ) );

    e01 = AE_MOVDA32X2(e1, e0);
    e01 = AE_ADD32(e01, 254);
    g01 = AE_SRAI32(e01, 1);
    e01 = AE_SUB32(e01, g01);
    u01 = AE_SLLI32(e01, 23);
    t01 = AE_SLLI32(g01, 23);
    y0 = XT_WFR(AE_MOVAD32_L(u01));
    y1 = XT_WFR(AE_MOVAD32_H(u01));
    z0 = XT_WFR(AE_MOVAD32_L(t01));
    z1 = XT_WFR(AE_MOVAD32_H(t01));

	XT_LSIP(x0_, X1, sz_f32);
	XT_LSIP(x1_, X1, sz_f32);
	b_nan0 = XT_UN_S(x0_, x0_);
	b_nan1 = XT_UN_S(x1_, x1_);
    XT_MOVT_S(x0, x0_, b_nan0);
    XT_MOVT_S(x1, x1_, b_nan1);

    y0 = XT_MUL_S(y0, z0);
    y1 = XT_MUL_S(y1, z1);
    y0 = XT_MUL_S(x0, y0);
    y1 = XT_MUL_S(x1, y1);

	XT_SSIP(y0, Y, sz_f32);
	XT_SSIP(y1, Y, sz_f32);
  }
  if (N & 1)
  {
    ae_f32x2 f0;
    ae_f32   v0, s0, m0;
    ae_int32 g0, t0;

	XT_LSIP(x0, X0, sz_f32);

	/* Check borders. */
	x0 = XT_MAX_S(alog10fminmax[0].f, x0);
	x0 = XT_MIN_S(alog10fminmax[1].f, x0);

    u0 = XT_TRUNC_S( XT_MUL_S( x0, XT_FLOAT_S( 1<<10, 0 ) ), 15 );

    /* scale input to 1/log10(2) and convert to Q31 */
    w0 = AE_MUL32_HH(u0, invlog10_2_Q29);
    e0 = ae_int32x2_rtor_int32(AE_TRUNCA32X2F64S(w0, w0, -22));
    r0 = AE_SLLI64(w0, 32-22);
    u0 = ae_int32x2_rtor_int32(AE_TRUNCI32X2F64S(r0, r0, 0));
    u0 = XT_SRLI(u0,1);

    tb0 = AE_L32_I(TBL, 0*sz_f32);
    tb1 = AE_L32_I(TBL, 1*sz_f32);
    tb2 = AE_L32_I(TBL, 2*sz_f32);
    tb3 = AE_L32_I(TBL, 3*sz_f32);
    tb4 = AE_L32_I(TBL, 4*sz_f32);
    tb5 = AE_L32_I(TBL, 5*sz_f32);
    tb6 = AE_L32_I(TBL, 6*sz_f32);

    /*
     * Compute 2^t in Q30 where t is in Q31. Use a combination of Estrin's
     * method and Horner's scheme to evaluate the fixed-point polynomial.                                     
     */

    v0 = u0;

    f0 = tb2; AE_MULAFP32X2RAS(f0, tb1, v0); tb1 = f0;
    f0 = tb4; AE_MULAFP32X2RAS(f0, tb3, v0); tb2 = f0;
    f0 = tb6; AE_MULAFP32X2RAS(f0, tb5, v0); tb3 = f0;

    s0 = AE_MULFP32X2RAS(v0, v0);

    f0 = tb0;                               m0 = f0;
    f0 = tb1; AE_MULAFP32X2RAS(f0, s0, m0); m0 = f0;
    f0 = tb2; AE_MULAFP32X2RAS(f0, s0, m0); m0 = f0;
    f0 = tb3; AE_MULAFP32X2RAS(f0, s0, m0); m0 = f0;

    n0 = ae_f32_rtor_int32(m0);

    /* convert back to the floating point  */
    x0 = XT_MUL_S( XT_FLOAT_S( n0, 15 ), XT_FLOAT_S( 1, 15 ) );

    e0 = XT_ADD(e0, 254);
    g0 = XT_SRAI(e0, 1);
    e0 = XT_SUB(e0, g0);
    u0 = XT_SLLI(e0, 23);
    t0 = XT_SLLI(g0, 23);
    y0 = XT_WFR(u0);
    z0 = XT_WFR(t0);

	XT_LSIP(x0_, X1, sz_f32);
	b_nan0 = XT_UN_S(x0_, x0_);
    XT_MOVT_S(x0, x0_, b_nan0);

    y0 = XT_MUL_S(y0, z0);
    y0 = XT_MUL_S(x0, y0);

	XT_SSIP(y0, Y, sz_f32);
  }
} /* vec_antilog10f() */
#endif
