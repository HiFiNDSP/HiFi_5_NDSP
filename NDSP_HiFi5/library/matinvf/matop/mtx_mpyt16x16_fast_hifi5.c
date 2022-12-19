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
/* Code optimized for HiFi5 core */
#include "NatureDSP_Signal_matop.h"
#include "NatureDSP_types.h"
#include "common.h"

/*-------------------------------------------------------------------------
  Matrix Multiply
  These functions compute the expression z = 2^lsh * x * y for the matrices 
  x and y. The columnar dimension of x must match the row dimension of y. 
  The resulting matrix has the same number of rows as x and the same number 
  of columns as y.
  Transposing API allows to interpret input yt as transposed matrix y.

  NOTE: lsh factor is not relevant for floating point routines.

  Functions require scratch memory for storing intermediate data. This 
  scratch memory area should be aligned on 16 byte boundary and its size is 
  calculated by dedicated scratch allocation functions.

  Two versions of functions available: regular version (mtx_mpy[t]32x32, 
  mtx_mpy[t]16x16, mtx_mpy[t]8x16, mtx_mpy[t]8x8, mtx[t]_mpyf) with 
  arbitrary arguments and faster version (mtx_mpy[t]32x32_fast, 
  mtx_mpy[t]16x16_fast, mtx_mpy[t]8x16_fast, mtx_mpy[t]8x8_fast, 
  mtx_mpy[t]f_fast, cntx_mpyt32x32_fast, cntx_mpytf_fast) that apply 
  some restrictions

  Precision:
  32x32 32-bit inputs, 32-bit output
  16x16 16-bit inputs, 16-bit output
  8x8   8-bit inputs, 8-bit output
  8x16  8/16-bit inputs, 16-bit output
  f     floating point

  Input:
  x[M*N]      input matrix x, Q7, Q15, Q31 or floating point
  y[N*P]      input matrix y, Q7, Q15, Q31 or floating point
  yt[P*N]     transposed input matrix y. Q31,Q15, Q7 floating point. (for 
              transposing API only)
  M           number of rows in matrix x and z
  N           number of columns in matrix x and number of rows in matrix y
  P           number of columns in matrices y and z
  lsh         left shift applied to the result (applied to the fixed-
              point functions only) 
  Output:
  z[M*P]      output matrix z, Q7, Q15, Q31 or floating point 
  Scratch:
  pScr        size in bytes defined by corresponding scratch allocation 
              functions

  Restrictions:
  For regular routines mpy[t]32x32, mtx_mpy[t]16x16, mtx_mpy[t]8x8, 
  mtx_mpy[t]8x16, mtx_mpy[t]f):
  pScr    aligned on 16-byte boundary
  x,y,z   should not overlap

  For faster routines (mtx_mpy[t]32x32_fast, mtx_mpy[t]16x16_fast, 
  mtx_mpy[t]8x8_fast, mtx_mpy[t]8x16_fast, 
  mtx_mpy[t]f_fast):
  x,y,z       should not overlap
  x,y,z,pScr  aligned on 16-byte boundary
  M,N,P       multiplies of 4 for mtx_mpy[t]32x32_fast, mtx_mpy[t]16x16_fast, 
              mtx_mpy[t]8x8_fast, mtx_mpy[t]8x16_fast, mtx_mpy[t]f_fast
              multiplies of 32 for cntx_mpyt32x32_fast, cntx_mpytf_fast
  lsh         should be in range:
              -31...31 for mtx_mpy32x32, mtx_mpy32x32_fast, cntx_mpyt32x32_fast, 
                       cntx_mpytf_fast
              -15...15 for mtx_mpy16x16, mtx_mpy16x16_fast, mtx_mpy[t]8x8, 
                       mtx_mpy[t]8x8_fast, mtx_mpy[t]8x16, 
                       mtx_mpy[t]8x16_fast 

-------------------------------------------------------------------------*/
void mtx_mpyt16x16_fast(void* pScr,
    int16_t* restrict z,
    const int16_t* restrict x,
    const int16_t* restrict yt,
    int M, int N, int P, int lsh)
{
    int m, n, p;
    const ae_int16x4 * restrict px;
    const ae_int16x4 * restrict px0;
    const ae_int16x4 * restrict px1;
    const ae_int16x4 * restrict px2;
    const ae_int16x4 * restrict px3;
    const ae_int16x4 * restrict py0;
    const ae_int16x4 * restrict py1;
    const ae_int16x4 * restrict py2;
    const ae_int16x4 * restrict py3;
    const ae_int16x8 * restrict px_;
    const ae_int16x8 * restrict px0_;
    const ae_int16x8 * restrict px1_;
    const ae_int16x8 * restrict px2_;
    const ae_int16x8 * restrict px3_;
    const ae_int16x8 * restrict py0_;
    const ae_int16x8 * restrict py1_;
    const ae_int16x8 * restrict py2_;
    const ae_int16x8 * restrict py3_;
    ae_int32   * restrict pz;

    NASSERT(N % 4 == 0);
    NASSERT(M % 4 == 0);
    NASSERT(P % 4 == 0);
    NASSERT_ALIGN(x, HIFI_SIMD_WIDTH);
    NASSERT_ALIGN(yt, HIFI_SIMD_WIDTH);
    NASSERT_ALIGN(z, HIFI_SIMD_WIDTH);
    if ((M <= 0) || (P <= 0)) return;
    if (N <= 0)
    {
        for (m = 0; m < M * P; m++) z[m] = 0;
        return;
    }

    if (N & 4)
    {
        __Pragma("loop_count min=1");
        for (p = 0; p < (P >> 2); p++)
        {
            px = (const ae_int16x4 *)x;
            pz = (ae_int32   *)z;
            __Pragma("loop_count min=1");
            for (m = 0; m < (M >> 2); m++)
            {
                ae_int16x4 y0,y1,y2,y3;
                ae_int16x4 x0,x1,x2,x3;
                ae_int32x2 T0,T2,T1,T3;
                ae_int64 B0, B1, B2, B3, B4, B5, B6, B7;
                ae_int64 C0, C1, C2, C3, C4, C5, C6, C7;
                py0 = (const ae_int16x4 *)yt;
                py1 = (const ae_int16x4 *)XT_ADDX2(N, (uintptr_t)py0);
                py2 = (const ae_int16x4 *)XT_ADDX4(N, (uintptr_t)py0);
                py3 = (const ae_int16x4 *)XT_ADDX2(N, (uintptr_t)py2);
                px0 = (const ae_int16x4 *)px;
                px1 = (const ae_int16x4 *)XT_ADDX2(N, (uintptr_t)px0);
                px2 = (const ae_int16x4 *)XT_ADDX4(N, (uintptr_t)px0);
                px3 = (const ae_int16x4 *)XT_ADDX4(N, (uintptr_t)px1);

                AE_MOVDX2(B0,B1,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(B2,B3,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(B4,B5,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(B6,B7,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C0,C1,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C2,C3,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C4,C5,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C6,C7,AE_ZERO64(),AE_ZERO64());

                __Pragma("loop_count min=1");
                for (n = 0; n < (N >> 2); n++)
                {
                    AE_L16X4_IP(y0, py0, sizeof(ae_int16x4));
                    AE_L16X4_IP(y1, py1, sizeof(ae_int16x4));
                    AE_L16X4_IP(y2, py2, sizeof(ae_int16x4));
                    AE_L16X4_IP(y3, py3, sizeof(ae_int16x4));

                    AE_L16X4_IP(x0, px0, sizeof(ae_int16x4));
                    AE_L16X4_IP(x1, px1, sizeof(ae_int16x4));
                    AE_L16X4_IP(x2, px2, sizeof(ae_int16x4));
                    AE_L16X4_IP(x3, px3, sizeof(ae_int16x4));

                    AE_MULAAAA2Q16(B0, B1, x0, x0, y0, y1);
                    AE_MULAAAA2Q16(B2, B3, x1, x1, y0, y1);
                    AE_MULAAAA2Q16(B4, B5, x2, x2, y0, y1);
                    AE_MULAAAA2Q16(B6, B7, x3, x3, y0, y1);
                    AE_MULAAAA2Q16(C0, C1, x0, x0, y2, y3);
                    AE_MULAAAA2Q16(C2, C3, x1, x1, y2, y3);
                    AE_MULAAAA2Q16(C4, C5, x2, x2, y2, y3);
                    AE_MULAAAA2Q16(C6, C7, x3, x3, y2, y3);
                }
                T0 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B1, B0, lsh + 33), AE_TRUNCA32X2F64S(C1, C0, lsh + 33))) ;
                T1 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B3, B2, lsh + 33), AE_TRUNCA32X2F64S(C3, C2, lsh + 33))) ;
                T2 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B5, B4, lsh + 33), AE_TRUNCA32X2F64S(C5, C4, lsh + 33))) ;
                T3 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B7, B6, lsh + 33), AE_TRUNCA32X2F64S(C7, C6, lsh + 33))) ;
                AE_S32X2_XP(T0,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                AE_S32X2_XP(T1,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                AE_S32X2_XP(T2,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                AE_S32X2_XP(T3,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                px = (const ae_int16x4 *)XT_ADDX8(N, (uintptr_t)px);
            }
            z += 4;
            yt += 4 * N;
        }
    }
    else
    {
        __Pragma("loop_count min=1");
        for (p = 0; p < (P >> 2); p++)
        {
            px_ = (const ae_int16x8 *)x;
            pz = (ae_int32   *)z;
            __Pragma("loop_count min=1");
            for (m = 0; m < (M >> 2); m++)
            {
                ae_int64 B0, B1, B2, B3, B4, B5, B6, B7;
                ae_int64 C0, C1, C2, C3, C4, C5, C6, C7;
                ae_int32x2 T0,T1,T2,T3;
                ae_int16x4 x0,x1,x2,x3,x4,x5,x6,x7,y0,y1,y2,y3,y4,y5,y6,y7;
                py0_ = (const ae_int16x8 *)yt;
                py1_ = (const ae_int16x8 *)XT_ADDX2(N, (uintptr_t)py0_);
                py2_ = (const ae_int16x8 *)XT_ADDX4(N, (uintptr_t)py0_);
                py3_ = (const ae_int16x8 *)XT_ADDX2(N, (uintptr_t)py2_);
                px0_ = (const ae_int16x8 *)px_;
                px1_ = (const ae_int16x8 *)XT_ADDX2(N, (uintptr_t)px0_);
                px2_ = (const ae_int16x8 *)XT_ADDX4(N, (uintptr_t)px0_);
                px3_ = (const ae_int16x8 *)XT_ADDX4(N, (uintptr_t)px1_);

                AE_MOVDX2(B0,B1,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(B2,B3,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(B4,B5,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(B6,B7,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C0,C1,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C2,C3,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C4,C5,AE_ZERO64(),AE_ZERO64());
                AE_MOVDX2(C6,C7,AE_ZERO64(),AE_ZERO64());

                __Pragma("loop_count min=1");
                for (n = 0; n < (N >> 3); n++)
                {
                    AE_L16X4X2_IP(y0, y1, py0_, 2 * sizeof(ae_int16x4));
                    AE_L16X4X2_IP(y2, y3, py1_, 2 * sizeof(ae_int16x4));
                    AE_L16X4X2_IP(y4, y5, py2_, 2 * sizeof(ae_int16x4));
                    AE_L16X4X2_IP(y6, y7, py3_, 2 * sizeof(ae_int16x4));

                    AE_L16X4X2_IP(x0, x1, px0_, 2 * sizeof(ae_int16x4));
                    AE_L16X4X2_IP(x2, x3, px1_, 2 * sizeof(ae_int16x4));
                    AE_L16X4X2_IP(x4, x5, px2_, 2 * sizeof(ae_int16x4));
                    AE_L16X4X2_IP(x6, x7, px3_, 2 * sizeof(ae_int16x4));

                    AE_MULAAAA2Q16(B0, B1, x0, x0, y0, y2);
                    AE_MULAAAA2Q16(B0, B1, x1, x1, y1, y3);
                    AE_MULAAAA2Q16(B2, B3, x2, x2, y0, y2);
                    AE_MULAAAA2Q16(B2, B3, x3, x3, y1, y3);
                    AE_MULAAAA2Q16(B4, B5, x4, x4, y0, y2);
                    AE_MULAAAA2Q16(B4, B5, x5, x5, y1, y3);
                    AE_MULAAAA2Q16(B6, B7, x6, x6, y0, y2);
                    AE_MULAAAA2Q16(B6, B7, x7, x7, y1, y3);
                    AE_MULAAAA2Q16(C0, C1, x0, x0, y4, y6);
                    AE_MULAAAA2Q16(C0, C1, x1, x1, y5, y7);
                    AE_MULAAAA2Q16(C2, C3, x2, x2, y4, y6);
                    AE_MULAAAA2Q16(C2, C3, x3, x3, y5, y7);
                    AE_MULAAAA2Q16(C4, C5, x4, x4, y4, y6);
                    AE_MULAAAA2Q16(C4, C5, x5, x5, y5, y7);
                    AE_MULAAAA2Q16(C6, C7, x6, x6, y4, y6);
                    AE_MULAAAA2Q16(C6, C7, x7, x7, y5, y7);
                }
                T0 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B1, B0, lsh + 33), AE_TRUNCA32X2F64S(C1, C0, lsh + 33))) ;
                T1 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B3, B2, lsh + 33), AE_TRUNCA32X2F64S(C3, C2, lsh + 33))) ;
                T2 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B5, B4, lsh + 33), AE_TRUNCA32X2F64S(C5, C4, lsh + 33))) ;
                T3 = AE_MOVINT32X2_FROMINT16X4(AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B7, B6, lsh + 33), AE_TRUNCA32X2F64S(C7, C6, lsh + 33))) ;
                AE_S32X2_XP(T0,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                AE_S32X2_XP(T1,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                AE_S32X2_XP(T2,castxcc(ae_int32x2,pz), P*sizeof(int16_t));
                AE_S32X2_XP(T3,castxcc(ae_int32x2,pz), P*sizeof(int16_t));

                px_ = (const ae_int16x8 *)XT_ADDX8(N, (uintptr_t)px_);
            }
            z += 4;
            yt += 4 * N;
        }
    }
} /* mtx_mpy16x16_fast() */

size_t mtx_mpyt16x16_fast_getScratchSize(int M, int N, int P)
{
    return 0;
} /* mtx_mpyt16x16_fast_getScratchSize() */
