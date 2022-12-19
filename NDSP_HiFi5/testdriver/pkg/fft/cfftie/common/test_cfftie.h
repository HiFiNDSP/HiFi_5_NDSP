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
* Test procedures for complex FFT functions.
*/
#ifndef __TEST_CFFTIE_H__
#define __TEST_CFFTIE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Cross-platform data type definitions. */
#include "types.h"
/* Test environment configuration. */
#include "config.h"
#include "packages.h"
/* DSP Library API. */
#include LIBRARY_HEADER(fft)
/* Fixed point arithmetics. */
#include "NatureDSP_Math.h"
/* Test engine API. */
#include "testeng.h"
/* Test engine extension for FFT. */
#include "../../common/testeng_fft.h"
/* Test data vectors tools and SEQ-file reader. */
#include "vectools.h"

#if !defined(COMPILER_MSVC)
/* C99 compatible type */
#include <complex.h>
#endif

#define MIN(a,b)   ( (a)<(b) ? (a) : (b) )
#define MAX(a,b)   ( (a)>(b) ? (a) : (b) )

/* Suppress Visual C warnings on +/-HUGE_VALF macros. */
#ifdef COMPILER_MSVC
#pragma warning(disable:4056)
#pragma warning(disable:4756)
#endif

#define sz_fr16c    sizeof(complex_fract16)
#define sz_fl64c    sizeof(complex_double)

/* Period, in frames, between reallocation of in/out buffers consumed by the
* target FFT routine. */
#define XY_VEC_REALLOC_PERIOD     16

/* FFT attributes  */
#define ATTR_FAST               (TE_FFT_CPLX|TE_FFT_OPT_REUSE_INBUF)

typedef struct tagTestDef
{
    int                phaseNum;
    const char *       seqFilename;
    tTestEngTarget     target;
    tTestEngDesc_fft   desc;
} TestDef_t;

/* Perform all tests for fft_cplx*_ie, ifft_cplx*_ie API functions. */
int main_cfftie(int phaseNum, int isFull, int isVerbose, int breakOnError, const TestDef_t *tbl);

#endif  /* __TEST_CFFTIE_H__ */
