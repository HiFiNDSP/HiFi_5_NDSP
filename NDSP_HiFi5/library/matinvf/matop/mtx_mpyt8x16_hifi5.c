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

/* Library API */
/* Code optimized for HiFi5 core */
#include "NatureDSP_Signal_matop.h"
#include "NatureDSP_types.h"
#include "common.h"
#include "mtx_mpy8x16_helper.h"

#if USE_NN_EXTENSION_8X16==0
static const uint64_t dsel_rephhll_tbl=0x40516273c8d9eafbULL;

/* copy y with transpose and padding */
static void mtx_mpy8x16_copyy(  
                     int16_t* restrict z,
               const int16_t* restrict y,
               int N, int P )
#if 0
{
    int n,p,P0,N0;
    P0=(P+3)&~3;
    N0=(N+7)&~7;
    NASSERT(P0>0);
    NASSERT(N0>0);
    for (n=0; n<N0*P0; n++) z[n]=0;
    for (p=0; p<P; p++)
    {
        for (n=0; n<N; n++)
        {
            z[p*N0+n]=y[p*N+n];
        }
    }
}
#else
{
    ae_int16* restrict pZ;
    int n,p,P0,N0;
    P0=(P+3)&~3;
    N0=(N+7)&~7;
    NASSERT(P0>0);
    NASSERT(N0>0);
    pZ=(ae_int16*)z;
    for (p=0; p<((P0*N0)>>3); p++) AE_S16X4X2_IP(0,0,castxcc(ae_int16x8,pZ),sizeof(ae_int16x8));
    pZ=(ae_int16*)z;
    if (N<8)
    {
        for (p=0; p<P; p++)
        {
            ae_int16x4 x;
            __Pragma("loop_count min=1,max=7")
            for (n=0; n<N ; n++)
            {
                AE_L16_XP(x,castxcc(ae_int16,y),2);
                AE_S16_0_IP(x,pZ,2);
            }
            pZ+=N0-N;
        }
    }
    else
    {
        for (p=0; p<P; p++)
        {
            ae_int16x4 x;
            ae_valign aY;
            aY=AE_LA64_PP(y);
            for (n=0; n<(N>>2) ; n++)
            {
                AE_LA16X4_IP(x,aY,castxcc(ae_int16x4,y));
                AE_S16X4_IP(x,castxcc(ae_int16x4,pZ),sizeof(ae_int16x4));
            }
            __Pragma("loop_count max=7")
            for (n=0; n<(N&3) ; n++)
            {
                AE_L16_XP(x,castxcc(ae_int16,y),2);
                AE_S16_0_IP(x,pZ,2);
            }
            pZ+=N0-N;
        }
    }
}
#endif

/* --------------------------------------
special code for multiplication of small 
matrices without scratch 
variant for N<8
-------------------------------------- */
static void mtx_mpyt8x16_small_Nsmall(
                     int16_t* restrict z,
               const  int8_t* restrict x,
               const int16_t* restrict y,
               int M, int N, int P, int lsh )
{
    int sa=lsh+41-2;
          ae_int16 * restrict pZ;
    const ae_int8  * restrict pX;
    const ae_int16 * restrict pY;
    int m,n,p;
    pX=(const ae_int8*)x;
    pZ=(ae_int16*)(z);
    NASSERT(N<8);
    for (m=0; m<(M&~1);m+=2)
    {
        pY=(const ae_int16*)y;
        for (p=0; p<(P&~1); p+=2)
        {
            ae_int16x4 r0,r1,r2,r3;
            ae_int64 A0,A1,A2,A3;
            AE_MOVDX2(A0,A1,0,0);AE_MOVDX2(A2,A3,0,0);
            __Pragma("loop_count min=1,max=7")
            for (n=0; n<(N); n++)
            {
                ae_int8x8 x0,x1;
                ae_int16x4 y0,y1;
                y1=AE_L16_X (pY,2*N);
                AE_L16_IP(y0,pY,2  );
                x1=AE_L8_X(pX,N);
                AE_L8_IP(x0,pX,1);
                AE_MULAAAA2Q16X8(A0,A2,y0,y1,x0);
                AE_MULAAAA2Q16X8(A1,A3,y0,y1,x1);
            }
            r0=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A0,A0, sa), AE_TRUNCA32X2F64S(A0,A0, sa));
            r1=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A1,A1, sa), AE_TRUNCA32X2F64S(A1,A1, sa));
            r2=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A2,A2, sa), AE_TRUNCA32X2F64S(A2,A2, sa));
            r3=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A3,A3, sa), AE_TRUNCA32X2F64S(A3,A3, sa));
            AE_S16_0_X (r3,pZ,2*P+2);
            AE_S16_0_I (r2,pZ,2    );
            AE_S16_0_X (r1,pZ,2*P  );
            AE_S16_0_IP(r0,pZ,4);
            pY+=N;
            pX-=N;
        }
        if (P&1)
        {
            ae_int16x4 r0,r1;
            ae_int64 A0,A1,A2,A3;
            AE_MOVDX2(A0,A1,0,0);AE_MOVDX2(A2,A3,0,0);
            __Pragma("loop_count min=1")
            for (n=0; n<N; n++)
            {
                ae_int8x8 x0,x1;
                ae_int16x4 y0;
                AE_L16_IP(y0,pY,2);
                x1=AE_L8_X(pX,N);
                AE_L8_IP(x0,pX,1);
                AE_MULAAAA2Q16X8(A0,A2,y0,y0,x0);
                AE_MULAAAA2Q16X8(A1,A3,y0,y0,x1);
            }
            r0=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A0,A0, sa), AE_TRUNCA32X2F64S(A0,A0, sa));
            r1=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A1,A1, sa), AE_TRUNCA32X2F64S(A1,A1, sa));
            AE_S16_0_X (r1,pZ,2*P  );
            AE_S16_0_IP(r0,pZ,2);
            pX-=N;
        }
        pZ+=P;
        pX=(const ae_int8*)XT_ADDX2(N,(uintptr_t)pX);
    }
    if (M&1)
    {
        pY=(const ae_int16*)y;
        for (p=0; p<P; p++)
        {
            ae_int16x4 r0;
            ae_int64 A0,A1;
            AE_MOVDX2(A0,A1,0,0);
            __Pragma("loop_count min=1,max=8")
            for (n=0; n<N; n++)
            {
                ae_int8x8 x0;
                ae_int16x4 y0;
                AE_L16_IP(y0,pY,2);
                AE_L8_IP(x0,pX,1);
                AE_MULAAAA2Q16X8(A0,A1,y0,y0,x0);
            }
            r0=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(A0,A0, sa), AE_TRUNCA32X2F64S(A0,A0, sa));
            AE_S16_0_IP(r0,pZ,2);
            pX-=N;
        }
    }
}

/*------------------------------------------
the same as above but for N>=8: in that case 
some vectorization is getting possible
------------------------------------------*/
static void mtx_mpyt8x16_small_Nbig(
                     int16_t* restrict z,
               const  int8_t* restrict x,
               const int16_t* restrict y,
               int M, int N, int P, int lsh )
{
    ae_int8x8 dsel_rephhll;
    int sa=lsh+41;
          ae_int16 * restrict pZ;
    const ae_int8  * restrict pX;
    const ae_int16 * restrict pY;
    ae_valignx2 aY0,aY1;
    ae_valign aX0,aX1;
    const ae_int8x8*  restrict pX0;
    const ae_int8x8*  restrict pX1;
    const ae_int16x8* restrict pY0;
    const ae_int16x8* restrict pY1;
    int m,n,p;
    dsel_rephhll=AE_L8X8_I((const ae_int8x8*)&dsel_rephhll_tbl,0);
    pX=(const ae_int8*)x;
    pZ=(ae_int16*)(z);
    NASSERT(N>=8);
    for (m=0; m<(M>>1);m++)
    {
        pY=(const ae_int16*)y;
        for (p=0; p<(P>>1); p++)
        {
            ae_int64 B0,B1,B2,B3;
            ae_int16x4 r0,r1,r2,r3;
            pX0=(const ae_int8x8*)(pX  );
            pX1=(const ae_int8x8*)(pX+N);
            pY0=(const ae_int16x8*)(pY  );
            pY1=(const ae_int16x8*)XT_ADDX2(N,(uintptr_t)pY);
            aY0=AE_LA128_PP(pY0);
            aY1=AE_LA128_PP(pY1);
            aX0=AE_LA64_PP(pX0);
            aX1=AE_LA64_PP(pX1);
            AE_MOVDX2(B0,B1,0,0); AE_MOVDX2(B2,B3,0,0);
            __Pragma("loop_count min=1")
            for (n=0; n<(N>>3); n++)
            {
                ae_int8x8 x0,x1;
                ae_int16x4 y0,y1,y2,y3;
                AE_LA8X8_IP(x0,aX0,pX0);
                AE_LA8X8_IP(x1,aX1,pX1);
                AE_LA16X4X2_IP(y0,y1,aY0,pY0);
                AE_LA16X4X2_IP(y2,y3,aY1,pY1);
                AE_MULAAAA2Q16X8(B0,B2,y0,y3,x0);
                AE_MULAAAA2Q16X8(B2,B0,y2,y1,x0);
                AE_MULAAAA2Q16X8(B1,B3,y0,y3,x1);
                AE_MULAAAA2Q16X8(B3,B1,y2,y1,x1);
            }
            __Pragma("loop_count min=0,max=7")
            __Pragma("no_unroll")
            for (n=0; n<(N&7); n++)
            {
                ae_int8x8 x0,x1;
                ae_int16x4 y0,y1;
                AE_L16_IP(y0,castxcc(ae_int16,pY0),2  );
                AE_L16_IP(y1,castxcc(ae_int16,pY1),2  );
                AE_L8_IP(x0,castxcc(ae_int8,pX0),1);
                AE_L8_IP(x1,castxcc(ae_int8,pX1),1);
                AE_MOVF16X4(y0,0,AE_MOVBA4(1));
                AE_MOVF16X4(y1,0,AE_MOVBA4(1));
                AE_MULAAAA2Q16X8(B0,B2,y0,y1,x0);
                AE_MULAAAA2Q16X8(B1,B3,y0,y1,x1);
            }
            r0=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B0,B0, sa), AE_TRUNCA32X2F64S(B0,B0, sa));
            r1=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B1,B1, sa), AE_TRUNCA32X2F64S(B1,B1, sa));
            r2=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B2,B2, sa), AE_TRUNCA32X2F64S(B2,B2, sa));
            r3=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B3,B3, sa), AE_TRUNCA32X2F64S(B3,B3, sa));
            AE_S16_0_X (r3,pZ,2*P+2);
            AE_S16_0_I (r2,pZ,2    );
            AE_S16_0_X (r1,pZ,2*P  );
            AE_S16_0_IP(r0,pZ,4);
            pY=((const ae_int16*)pY0)+N;
            pX=((const ae_int8*)pX0)-N;
        }
        if (P&1)
        {
            ae_int16x4 r0,r1;
            ae_int64 B0,B1,B2,B3;
            pX0=(const ae_int8x8* )(pX  );
            pX1=(const ae_int8x8* )(pX+N);
            pY0=(const ae_int16x8*)(pY  );
            aY0=AE_LA128_PP(pY0);
            aX0=AE_LA64_PP(pX0);
            aX1=AE_LA64_PP(pX1);
            AE_MOVDX2(B0,B1,0,0);AE_MOVDX2(B2,B3,0,0);
            __Pragma("loop_count min=1")
            for (n=0; n<(N>>3); n++)
            {
                ae_int8x8 x0,x1,x00,x11;
                ae_int16x4 y0,y1;
                AE_LA8X8_IP(x0,aX0,pX0);
                AE_LA8X8_IP(x1,aX1,pX1);
                AE_LA16X4X2_IP(y0,y1,aY0,pY0);
                AE_DSEL8X8(x00,x11,x0,x1,dsel_rephhll);
                AE_MULAAAA2Q16X8(B0,B1,y0,y0,x00);
                AE_MULAAAA2Q16X8(B0,B1,y1,y1,x11);
            }
            __Pragma("loop_count min=0,max=7")
            __Pragma("no_unroll")
            for (n=0; n<(N&7); n++)
            {
                ae_int8x8 x0,x1;
                ae_int16x4 y0;
                AE_L16_IP(y0,castxcc(ae_int16,pY0),2  );
                AE_L8_IP(x0,castxcc(ae_int8,pX0),1);
                AE_L8_IP(x1,castxcc(ae_int8,pX1),1);
                AE_MOVF16X4(y0,0,AE_MOVBA4(1));
                AE_MULAAAA2Q16X8(B0,B2,y0,y0,x0);
                AE_MULAAAA2Q16X8(B1,B3,y0,y0,x1);
            }
            r0=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B0,B0, sa), AE_TRUNCA32X2F64S(B0,B0, sa));
            r1=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B1,B1, sa), AE_TRUNCA32X2F64S(B1,B1, sa));
            AE_S16_0_X (r1,pZ,2*P  );
            AE_S16_0_IP(r0,pZ,2);
            pX=((const ae_int8 *)pX0)-N;
            pY=(const ae_int16*)pY0;
        }
        pZ+=P;
        pX=(const ae_int8*)XT_ADDX2(N,(uintptr_t)pX);
    }
    if (M&1)
    {
        pY=(const ae_int16*)y;
        for (p=0; p<P; p++)
        {
            ae_int16x4 r0;
            ae_int64 B0,B1,B2,B3;
            pX0=(const ae_int8x8* )(pX  );
            pY0=(const ae_int16x8*)(pY  );
            aY0=AE_LA128_PP(pY0);
            aX0=AE_LA64_PP(pX0);
            AE_MOVDX2(B0,B1,0,0);AE_MOVDX2(B2,B3,0,0);
            __Pragma("loop_count min=1")
            for (n=0; n<(N>>3); n++)
            {
                ae_int8x8 x0;
                ae_int16x4 y0,y1;
                AE_LA8X8_IP(x0,aX0,pX0);
                AE_LA16X4X2_IP(y0,y1,aY0,pY0);
                AE_MULAAAA2Q16X8(B0,B1,y0,y1,x0);
            }
            __Pragma("loop_count min=0,max=7")
            __Pragma("no_unroll")
            for (n=0; n<(N&7); n++)
            {
                ae_int8x8 x0;
                ae_int16x4 y0;
                AE_L16_IP(y0,castxcc(ae_int16,pY0),2);
                AE_L8_IP (x0,castxcc(ae_int8,pX0),1);
                AE_MOVF16X4(y0,0,AE_MOVBA4(1));
                AE_MULAAAA2Q16X8(B0,B1,y0,0,x0);
            }
            B0=B0+B1;
            r0=AE_ROUND16X4F32SASYM(AE_TRUNCA32X2F64S(B0,B0, sa), AE_TRUNCA32X2F64S(B0,B0, sa));
            AE_S16_0_IP(r0,pZ,2);
            pX=((const ae_int8*)pX0)-N;
            pY=((const ae_int16*)pY0);
        }
    }
}
#endif

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
void mtx_mpyt8x16(  void* pScr,
                     int16_t* restrict z,
               const  int8_t* restrict x,
               const int16_t* restrict y,
               int M, int N, int P, int lsh )
{
#if USE_NN_EXTENSION_8X16==0
    NASSERT(lsh >= -15 && lsh <= 15);
    NASSERT_ALIGN(pScr,HIFI_SIMD_WIDTH);
    if (M<=0 || P<=0) return;
    if (N<=0)    /* exceptional situation */
    {
        int m;
        for (m=0; m<M*P; m++) z[m]=0;
        return ;
    }
    /* if matrix sizes are very small it is better to multiply them element-wise */
    if (M*N*P<=128 || M<4)  
    {
        void (*fn)(int16_t* ,const  int8_t* ,const int16_t* ,int , int , int , int );
        fn=(N<8)?mtx_mpyt8x16_small_Nsmall:mtx_mpyt8x16_small_Nbig;
        fn(z,x,y,M, N, P, lsh );
        return;
    }
    /* copy matrices with padding to the scratch */
    int P0=((P+3)&~3);
    int N0=((N+7)&~7);
    int M0=((M+3)&~3);
    int16_t* v=(int16_t*)pScr;
    pScr=(v+ M0*P0);
    mtx_mpy8x16_copyy((int16_t*)pScr,y,N,P);
    y=(int16_t*)pScr;
    pScr=(void*)(y+ N0*P0);
    if (N<8) 
    {
        mtx_mpy8x16_copysmallx((int8_t*)pScr,x,M,N);
        x=(const int8_t*)pScr;
        N=8;
    }

    mtx_mpyt8x16_partiallyaligned (v,x,y,M, N,P, lsh );
    mtx_mpyt8x16_copyz(z,v,M,P);
#else
    static const int16_t ALIGN(16) dsel_interleave1_tbl[]={6|(7<<8), 4|(5<<8), 2|(3<<8), 0|(1<<8)};
    int N0;
    int m,n,p;
    int8_t *xp;
    ae_valignx2 alY0, alY1, alY2, alY3;
    ae_valignx2 alZ0, alZ1, alZ2, alZ3;
    int16_t * restrict pZ0;
    int16_t * restrict pZ1;
    int16_t * restrict pZ2;
    int16_t * restrict pZ3;
    const int8_t * restrict pX0;
    const int8_t * restrict pX1;
    const int8_t * restrict pX2;
    const int8_t * restrict pX3;
          int8_t * restrict pXwr;
    const int16_t * restrict pY0;
    const int16_t * restrict pY1;
    const int16_t * restrict pY2;
    const int16_t * restrict pY3;

    ae_int32x2 Z00, Z01, Z10, Z11, Z20, Z21, Z30, Z31;
    ae_int8x8 x00, x01, x10, x11, x20, x21, x30, x31;
    ae_int16x4 y00, y01, y10, y11, y20, y21, y30, y31;
    ae_int16x4 z0, z1, z2, z3;
    ae_int16x4 zero, dsel_interleave1;

    NASSERT(lsh >= -15 && lsh <= 15);
    NASSERT_ALIGN(pScr,HIFI_SIMD_WIDTH);

    if (M<=0 || P<=0) return;
    zero = AE_ZERO16();
    if (N<=0)    /* exceptional situation */
    {
        pZ0 = z;
        alZ0 = AE_ZALIGN128();
        for (m=0; m<((M*P)>>3); m++)  AE_SA16X4X2_IP(zero, zero, alZ0, castxcc(ae_int16x8,pZ0));
        AE_SAV16X4X2_XP(zero, zero, alZ0, castxcc(ae_int16x8,pZ0), ((M*P)&7)*sizeof(int16_t));
        AE_SA128POS_FP(alZ0, pZ0);
        return;
    }

    /* Copy matrix X to buffer with zero-padded columns      *
     * to make the matrix with all rows aligned;             *
     * number of columns is rounded to a next multiple of 16 */
    xp = (int8_t *)pScr;
    N0 = (N+15)&~15;
    {
        ae_valignx2 alX0, alX1, alX2, alX3;
        pX3 = x;
        pXwr = xp;
        for (m = 0; m < (M>>2); m++)
        {
            pX0 = pX3;
            pX1 = pX0 + N;
            pX2 = pX1 + N;
            pX3 = pX2 + N;
            alX0 = AE_LA128_PP(pX0);
            alX1 = AE_LA128_PP(pX1);
            alX2 = AE_LA128_PP(pX2);
            alX3 = AE_LA128_PP(pX3);
            for (n = 0; n < ((N-1)>>4); n++)
            {
                AE_LA8X8X2_IP(x00, x01, alX0, castxcc(ae_int8x16,pX0));
                AE_LA8X8X2_IP(x10, x11, alX1, castxcc(ae_int8x16,pX1));
                AE_LA8X8X2_IP(x20, x21, alX2, castxcc(ae_int8x16,pX2));
                AE_LA8X8X2_IP(x30, x31, alX3, castxcc(ae_int8x16,pX3));
                AE_S8X8X2_IP(x00, x10, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
                AE_S8X8X2_IP(x20, x30, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
                AE_S8X8X2_IP(x01, x11, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
                AE_S8X8X2_IP(x21, x31, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
            }
            AE_LAV8X8X2_XP(x00, x01, alX0, castxcc(ae_int8x16,pX0), ((N-1)&15)+1);
            AE_LAV8X8X2_XP(x10, x11, alX1, castxcc(ae_int8x16,pX1), ((N-1)&15)+1);
            AE_LAV8X8X2_XP(x20, x21, alX2, castxcc(ae_int8x16,pX2), ((N-1)&15)+1);
            AE_LAV8X8X2_XP(x30, x31, alX3, castxcc(ae_int8x16,pX3), ((N-1)&15)+1);
            AE_S8X8X2_IP(x00, x10, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
            AE_S8X8X2_IP(x20, x30, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
            AE_S8X8X2_IP(x01, x11, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
            AE_S8X8X2_IP(x21, x31, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
        }
        /* "Align" last M%4 rows of matrix X */
        pX0 = pX3;
        alX0 = AE_LA128_PP(pX0);
        for (m = 0; m < (M&3); m++)
        {
            __Pragma("no_unroll");
            for (n = 0; n < ((N-1)>>4); n++)
            {
                AE_LA8X8X2_IP(x00, x01, alX0, castxcc(ae_int8x16,pX0));
                AE_S8X8X2_IP(x00, x01, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
            }
            AE_LAV8X8X2_XP(x00, x01, alX0, castxcc(ae_int8x16,pX0), ((N-1)&15)+1);
            AE_S8X8X2_IP(x00, x01, castxcc(ae_int8x16,pXwr), sizeof(ae_int8x16));
        }
    }

    dsel_interleave1 = AE_L16X4_I((const ae_int16x4*)dsel_interleave1_tbl, 0);
    WUR_AE_SAR(9+lsh);
    WUR_AE_CBEGIN0((uintptr_t)(y));
    WUR_AE_CEND0((uintptr_t)(y + N*P));
    pZ3 = z;
    for (m=0; m<(M>>2); m++)
    {
        pZ0 = pZ3;
        pZ1 = pZ0 + P;
        pZ2 = pZ1 + P;
        pZ3 = pZ2 + P;
        __Pragma("loop_count min=1");
        for (p=0; p<P; p+=4)
        {
            pX0 = xp + m*N0*4;
            pY0 = y + p*N;
            pY1 = pY0;  AE_ADDCIRC_XC(castxcc(ae_int64,pY1), N*sizeof(int16_t));
            pY2 = pY1;  AE_ADDCIRC_XC(castxcc(ae_int64,pY2), N*sizeof(int16_t));
            pY3 = pY2;  AE_ADDCIRC_XC(castxcc(ae_int64,pY3), N*sizeof(int16_t));
            alY0 = AE_LA128_PP(pY0);
            alY1 = AE_LA128_PP(pY1);
            alY2 = AE_LA128_PP(pY2);
            alY3 = AE_LA128_PP(pY3);

            Z00 = Z01 = Z10 = Z11 = Z20 = Z21 = Z30 = Z31 = AE_ZERO32();

            for (n=0; n<((N-1)>>3); n++)
            {
                /* load x matrix, 4x8 values, 8-bit */
                AE_L8X8X2_IP(x00, x10, castxcc(ae_int8x16,pX0), sizeof(ae_int8x16));
                AE_L8X8X2_IP(x20, x30, castxcc(ae_int8x16,pX0), sizeof(ae_int8x16));

                /* load y matrix, 4x8 values, 16-bit */
                AE_LA16X4X2_IP(y00, y01, alY0, castxcc(ae_int16x8,pY0));
                AE_LA16X4X2_IP(y10, y11, alY1, castxcc(ae_int16x8,pY1));
                AE_LA16X4X2_IP(y20, y21, alY2, castxcc(ae_int16x8,pY2));
                AE_LA16X4X2_IP(y30, y31, alY3, castxcc(ae_int16x8,pY3));

                /* make multiply, using 4-way 32-bit output octal mac, 8X16 bit */
                AE_MULA8Q8X16(Z00, Z01, x00, x10, x20, x30, y00, y01);
                AE_MULA8Q8X16(Z10, Z11, x00, x10, x20, x30, y10, y11);
                AE_MULA8Q8X16(Z20, Z21, x00, x10, x20, x30, y20, y21);
                AE_MULA8Q8X16(Z30, Z31, x00, x10, x20, x30, y30, y31);
            }
            /* Process last 1...16 multiplications of each output value */
            {
                /* load x matrix, 4x8 values, 8-bit */
                AE_L8X8X2_IP(x00, x10, castxcc(ae_int8x16,pX0), sizeof(ae_int8x16));
                AE_L8X8X2_IP(x20, x30, castxcc(ae_int8x16,pX0), sizeof(ae_int8x16));

                /* load y matrix, 4x8 values, 16-bit */
                AE_LAV16X4X2_XP(y00, y01, alY0, castxcc(ae_int16x8,pY0), (((N-1)&7)+1)*sizeof(int16_t));
                AE_LAV16X4X2_XP(y10, y11, alY1, castxcc(ae_int16x8,pY1), (((N-1)&7)+1)*sizeof(int16_t));
                AE_LAV16X4X2_XP(y20, y21, alY2, castxcc(ae_int16x8,pY2), (((N-1)&7)+1)*sizeof(int16_t));
                AE_LAV16X4X2_XP(y30, y31, alY3, castxcc(ae_int16x8,pY3), (((N-1)&7)+1)*sizeof(int16_t));

                /* make multiply, using 4-way 32-bit output octal mac, 8X16 bit */
                AE_MULA8Q8X16(Z00, Z01, x00, x10, x20, x30, y00, y01);
                AE_MULA8Q8X16(Z10, Z11, x00, x10, x20, x30, y10, y11);
                AE_MULA8Q8X16(Z20, Z21, x00, x10, x20, x30, y20, y21);
                AE_MULA8Q8X16(Z30, Z31, x00, x10, x20, x30, y30, y31);
            }
            /* Q15 + lsh <- Q22 - 7 + lsh w/ rounding and saturation */
            Z00 = AE_SLAS32S(Z00);    Z01 = AE_SLAS32S(Z01);
            Z10 = AE_SLAS32S(Z10);    Z11 = AE_SLAS32S(Z11);
            Z20 = AE_SLAS32S(Z20);    Z21 = AE_SLAS32S(Z21);
            Z30 = AE_SLAS32S(Z30);    Z31 = AE_SLAS32S(Z31);
            z0 = AE_ROUND16X4F32SASYM(Z00, Z10);
            z1 = AE_ROUND16X4F32SASYM(Z20, Z30);
            z2 = AE_ROUND16X4F32SASYM(Z01, Z11);
            z3 = AE_ROUND16X4F32SASYM(Z21, Z31);
            AE_DSEL16X4(z0, z1, z0, z1, dsel_interleave1);  
            AE_DSEL16X4(z2, z3, z2, z3, dsel_interleave1);  
            alZ0 = alZ1 = alZ2 = alZ3 = AE_ZALIGN128();
            AE_SAV16X4X2_XP(z0, z0, alZ0, castxcc(ae_int16x8,pZ0), XT_MIN(4, P-p)*sizeof(int16_t));
            AE_SAV16X4X2_XP(z1, z1, alZ1, castxcc(ae_int16x8,pZ1), XT_MIN(4, P-p)*sizeof(int16_t));
            AE_SAV16X4X2_XP(z2, z2, alZ2, castxcc(ae_int16x8,pZ2), XT_MIN(4, P-p)*sizeof(int16_t));
            AE_SAV16X4X2_XP(z3, z3, alZ3, castxcc(ae_int16x8,pZ3), XT_MIN(4, P-p)*sizeof(int16_t));
            AE_SA128POS_FP(alZ0, pZ0);
            AE_SA128POS_FP(alZ1, pZ1);
            AE_SA128POS_FP(alZ2, pZ2);
            AE_SA128POS_FP(alZ3, pZ3);
        }
    }
    /* Process last M%4 rows */
    pZ0 = pZ3;
    for (m = (M&~3); m < M; m++)
    {
        __Pragma("loop_count min=1");
        for (p=0; p<P; p+=4)
        {
            pX0 = xp + m*N0;
            pY0 = y + p*N;
            pY1 = pY0;  AE_ADDCIRC_XC(castxcc(ae_int64,pY1), N*sizeof(int16_t));
            pY2 = pY1;  AE_ADDCIRC_XC(castxcc(ae_int64,pY2), N*sizeof(int16_t));
            pY3 = pY2;  AE_ADDCIRC_XC(castxcc(ae_int64,pY3), N*sizeof(int16_t));
            alY0 = AE_LA128_PP(pY0);
            alY1 = AE_LA128_PP(pY1);
            alY2 = AE_LA128_PP(pY2);
            alY3 = AE_LA128_PP(pY3);

            Z00 = Z01 = Z10 = Z11 = Z20 = Z21 = Z30 = Z31 = AE_ZERO32();

            for (n=0; n<((N-1)>>3); n++)
            {
                /* load x matrix, 1x8 values, 8-bit */
                AE_L8X8_IP(x00, castxcc(ae_int8x8,pX0), sizeof(ae_int8x8));

                /* load y matrix, 4x8 values, 16-bit */
                AE_LA16X4X2_IP(y00, y01, alY0, castxcc(ae_int16x8,pY0));
                AE_LA16X4X2_IP(y10, y11, alY1, castxcc(ae_int16x8,pY1));
                AE_LA16X4X2_IP(y20, y21, alY2, castxcc(ae_int16x8,pY2));
                AE_LA16X4X2_IP(y30, y31, alY3, castxcc(ae_int16x8,pY3));

                /* make multiply, using 4-way 32-bit output octal mac, 8X16 bit */
                AE_MULA8Q8X16(Z00, Z01, x00, x00, x00, x00, y00, y01);
                AE_MULA8Q8X16(Z10, Z11, x00, x00, x00, x00, y10, y11);
                AE_MULA8Q8X16(Z20, Z21, x00, x00, x00, x00, y20, y21);
                AE_MULA8Q8X16(Z30, Z31, x00, x00, x00, x00, y30, y31);
            }
            /* Process last 1...16 multiplications of each output value */
            {
                /* load x matrix, 1x8 values, 8-bit */
                AE_L8X8_IP(x00, castxcc(ae_int8x8,pX0), sizeof(ae_int8x8));

                /* load y matrix, 4x8 values, 16-bit */
                AE_LAV16X4X2_XP(y00, y01, alY0, castxcc(ae_int16x8,pY0), (((N-1)&7)+1)*sizeof(int16_t));
                AE_LAV16X4X2_XP(y10, y11, alY1, castxcc(ae_int16x8,pY1), (((N-1)&7)+1)*sizeof(int16_t));
                AE_LAV16X4X2_XP(y20, y21, alY2, castxcc(ae_int16x8,pY2), (((N-1)&7)+1)*sizeof(int16_t));
                AE_LAV16X4X2_XP(y30, y31, alY3, castxcc(ae_int16x8,pY3), (((N-1)&7)+1)*sizeof(int16_t));

                /* make multiply, using 4-way 32-bit output octal mac, 8X16 bit */
                AE_MULA8Q8X16(Z00, Z01, x00, x00, x00, x00, y00, y01);
                AE_MULA8Q8X16(Z10, Z11, x00, x00, x00, x00, y10, y11);
                AE_MULA8Q8X16(Z20, Z21, x00, x00, x00, x00, y20, y21);
                AE_MULA8Q8X16(Z30, Z31, x00, x00, x00, x00, y30, y31);
            }
            /* Q15 + lsh <- Q22 - 7 + lsh w/ rounding and saturation */
            Z00 = AE_SLAS32S(Z00);
            Z10 = AE_SLAS32S(Z10);
            Z20 = AE_SLAS32S(Z20);
            Z30 = AE_SLAS32S(Z30);
            z0 = AE_ROUND16X4F32SASYM(Z00, Z10);
            z1 = AE_ROUND16X4F32SASYM(Z20, Z30);
            z0 = AE_SEL16_7531(z0, z1);
            alZ0 = AE_ZALIGN128();
            AE_SAV16X4X2_XP(z0, z0, alZ0, castxcc(ae_int16x8,pZ0), XT_MIN(4, P-p)*sizeof(int16_t));
            AE_SA128POS_FP(alZ0, pZ0);
        }
    }
#endif
}

size_t mtx_mpyt8x16_getScratchSize(int M, int N, int P)
{
#if USE_NN_EXTENSION_8X16==0
    if (M<=0 || N<=0 || P<=0) return 0;
    /* allocate matrices of size (N+7)&~7 x (P+3)&~3 and ((M+3)&~3) x (P+3)&~3 */
    size_t ysz,zsz,xsz;
    if (M<=0 || N<=0 || P<=0) return 0;
    int P0=((P+3)&~3);
    int N0=((N+7)&~7);
    int M0=((M+3)&~3);
    ysz=(N0 * P0)*sizeof(int16_t);
    zsz=(M0 * P0)*sizeof(int16_t);
    xsz=(N<8) ? M*8 : 0;
    return ysz+zsz+xsz;
#else
    if (N<=0 || M<=0 || P<=0) return 0;
    N = (N+15)&~15;
    return M*N;
#endif
}
