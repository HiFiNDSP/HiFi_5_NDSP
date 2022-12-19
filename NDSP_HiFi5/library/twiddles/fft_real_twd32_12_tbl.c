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
#include "NatureDSP_Signal_fft.h"
#include "fft_twiddles32x32.h"
#include "common.h"

ALIGN(32) static const int32_t __fft_real_tw[] =
{
    (int32_t)0x7FFFFFFF, (int32_t)0x00000000,
    (int32_t)0x6ED9EBA1, (int32_t)0xC0000000,
    (int32_t)0x40000000, (int32_t)0x9126145F,
};

/****************** stage 1 radix 3 ******************/
ALIGN(32) static const int32_t __fft6_tw1[] =
{
    (int32_t)0x7FFFFFFF, (int32_t)0x00000000, (int32_t)0x7FFFFFFF, (int32_t)0x00000000, (int32_t)0x40000000, (int32_t)0x9126145F, (int32_t)0xC0000000, (int32_t)0x9126145F,
};
#define N 6
static const fft_cplx32x32_stage_t s2_tab[] =
{
    fft_stageS2_DFT3_first_32x32,
    fft_stageS2_DFT2_last_32x32,
    NULL
};
static const fft_cplx32x32_stage_t s3_tab[] =
{
    fft_stageS3_DFT3_first_32x32,
    fft_stageS3_DFT2_last_32x32,
    NULL
};
static const fft_cplx32x32_stage_t is2_tab[] =
{
    ifft_stageS2_DFT3_first_32x32,
    ifft_stageS2_DFT2_last_32x32,
    NULL
};
static const fft_cplx32x32_stage_t is3_tab[] =
{
    ifft_stageS3_DFT3_first_32x32,
    ifft_stageS3_DFT2_last_32x32,
    NULL
};
static const int tw_step_tab[] =
{
    1, 1,
};
static const cint32ptr_t tw_tab[] =
{
    __fft6_tw1, NULL
};
static const fft_cplx32x32_descr_t __cfft_descr =
{
    N, 
    s2_tab,
    s3_tab,
    tw_step_tab,
    tw_tab
};
static const fft_cplx32x32_descr_t __cifft_descr =
{
    N,
    is2_tab,
    is3_tab,
    tw_step_tab,
    tw_tab
};

static const fft_real32x32_descr_t __rfft_descr =
{
    (const fft_handle_t)&__cfft_descr,
    __fft_real_tw
};

static const fft_real32x32_descr_t __rifft_descr =
{
    (const fft_handle_t)&__cifft_descr,
    __fft_real_tw
};

const fft_handle_t rnfft32_12 =  (const fft_handle_t)&__rfft_descr;
const fft_handle_t rinfft32_12 = (const fft_handle_t)&__rifft_descr;
