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
    (int32_t)0x7FFFFFFF, (int32_t)0x00000000, (int32_t)0x7FFFD886, (int32_t)0xFF9B781D, (int32_t)0x7FFF6216, (int32_t)0xFF36F078, (int32_t)0x7FFE9CB2, (int32_t)0xFED2694F,
    (int32_t)0x7FFD885A, (int32_t)0xFE6DE2E0, (int32_t)0x7FFC250F, (int32_t)0xFE095D69, (int32_t)0x7FFA72D1, (int32_t)0xFDA4D929, (int32_t)0x7FF871A2, (int32_t)0xFD40565C,
    (int32_t)0x7FF62182, (int32_t)0xFCDBD541, (int32_t)0x7FF38274, (int32_t)0xFC775616, (int32_t)0x7FF09478, (int32_t)0xFC12D91A, (int32_t)0x7FED5791, (int32_t)0xFBAE5E89,
    (int32_t)0x7FE9CBC0, (int32_t)0xFB49E6A3, (int32_t)0x7FE5F108, (int32_t)0xFAE571A4, (int32_t)0x7FE1C76B, (int32_t)0xFA80FFCB, (int32_t)0x7FDD4EEC, (int32_t)0xFA1C9157,
    (int32_t)0x7FD8878E, (int32_t)0xF9B82684, (int32_t)0x7FD37153, (int32_t)0xF953BF91, (int32_t)0x7FCE0C3E, (int32_t)0xF8EF5CBB, (int32_t)0x7FC85854, (int32_t)0xF88AFE42,
    (int32_t)0x7FC25596, (int32_t)0xF826A462, (int32_t)0x7FBC040A, (int32_t)0xF7C24F59, (int32_t)0x7FB563B3, (int32_t)0xF75DFF66, (int32_t)0x7FAE7495, (int32_t)0xF6F9B4C6,
    (int32_t)0x7FA736B4, (int32_t)0xF6956FB7, (int32_t)0x7F9FAA15, (int32_t)0xF6313077, (int32_t)0x7F97CEBD, (int32_t)0xF5CCF743, (int32_t)0x7F8FA4B0, (int32_t)0xF568C45B,
    (int32_t)0x7F872BF3, (int32_t)0xF50497FB, (int32_t)0x7F7E648C, (int32_t)0xF4A07261, (int32_t)0x7F754E80, (int32_t)0xF43C53CB, (int32_t)0x7F6BE9D4, (int32_t)0xF3D83C77,
    (int32_t)0x7F62368F, (int32_t)0xF3742CA2, (int32_t)0x7F5834B7, (int32_t)0xF310248A, (int32_t)0x7F4DE451, (int32_t)0xF2AC246E, (int32_t)0x7F434563, (int32_t)0xF2482C8A,
    (int32_t)0x7F3857F6, (int32_t)0xF1E43D1C, (int32_t)0x7F2D1C0E, (int32_t)0xF1805662, (int32_t)0x7F2191B4, (int32_t)0xF11C789A, (int32_t)0x7F15B8EE, (int32_t)0xF0B8A401,
    (int32_t)0x7F0991C4, (int32_t)0xF054D8D5, (int32_t)0x7EFD1C3C, (int32_t)0xEFF11753, (int32_t)0x7EF05860, (int32_t)0xEF8D5FB8, (int32_t)0x7EE34636, (int32_t)0xEF29B243,
    (int32_t)0x7ED5E5C6, (int32_t)0xEEC60F31, (int32_t)0x7EC8371A, (int32_t)0xEE6276BF, (int32_t)0x7EBA3A39, (int32_t)0xEDFEE92B, (int32_t)0x7EABEF2C, (int32_t)0xED9B66B2,
    (int32_t)0x7E9D55FC, (int32_t)0xED37EF91, (int32_t)0x7E8E6EB2, (int32_t)0xECD48407, (int32_t)0x7E7F3957, (int32_t)0xEC71244F, (int32_t)0x7E6FB5F4, (int32_t)0xEC0DD0A8,
    (int32_t)0x7E5FE493, (int32_t)0xEBAA894F, (int32_t)0x7E4FC53E, (int32_t)0xEB474E81, (int32_t)0x7E3F57FF, (int32_t)0xEAE4207A, (int32_t)0x7E2E9CDF, (int32_t)0xEA80FF7A,
    (int32_t)0x7E1D93EA, (int32_t)0xEA1DEBBB, (int32_t)0x7E0C3D29, (int32_t)0xE9BAE57D, (int32_t)0x7DFA98A8, (int32_t)0xE957ECFB, (int32_t)0x7DE8A670, (int32_t)0xE8F50273,
    (int32_t)0x7DD6668F, (int32_t)0xE8922622, (int32_t)0x7DC3D90D, (int32_t)0xE82F5844, (int32_t)0x7DB0FDF8, (int32_t)0xE7CC9917, (int32_t)0x7D9DD55A, (int32_t)0xE769E8D8,
    (int32_t)0x7D8A5F40, (int32_t)0xE70747C4, (int32_t)0x7D769BB5, (int32_t)0xE6A4B616, (int32_t)0x7D628AC6, (int32_t)0xE642340D, (int32_t)0x7D4E2C7F, (int32_t)0xE5DFC1E5,
    (int32_t)0x7D3980EC, (int32_t)0xE57D5FDA, (int32_t)0x7D24881B, (int32_t)0xE51B0E2A, (int32_t)0x7D0F4218, (int32_t)0xE4B8CD11, (int32_t)0x7CF9AEF0, (int32_t)0xE4569CCB,
    (int32_t)0x7CE3CEB2, (int32_t)0xE3F47D96, (int32_t)0x7CCDA169, (int32_t)0xE3926FAD, (int32_t)0x7CB72724, (int32_t)0xE330734D, (int32_t)0x7CA05FF1, (int32_t)0xE2CE88B3,
    (int32_t)0x7C894BDE, (int32_t)0xE26CB01B, (int32_t)0x7C71EAF9, (int32_t)0xE20AE9C1, (int32_t)0x7C5A3D50, (int32_t)0xE1A935E2, (int32_t)0x7C4242F2, (int32_t)0xE14794BA,
    (int32_t)0x7C29FBEE, (int32_t)0xE0E60685, (int32_t)0x7C116853, (int32_t)0xE0848B7F, (int32_t)0x7BF88830, (int32_t)0xE02323E5, (int32_t)0x7BDF5B94, (int32_t)0xDFC1CFF3,
    (int32_t)0x7BC5E290, (int32_t)0xDF608FE4, (int32_t)0x7BAC1D31, (int32_t)0xDEFF63F4, (int32_t)0x7B920B89, (int32_t)0xDE9E4C60, (int32_t)0x7B77ADA8, (int32_t)0xDE3D4964,
    (int32_t)0x7B5D039E, (int32_t)0xDDDC5B3B, (int32_t)0x7B420D7A, (int32_t)0xDD7B8220, (int32_t)0x7B26CB4F, (int32_t)0xDD1ABE51, (int32_t)0x7B0B3D2C, (int32_t)0xDCBA1008,
    (int32_t)0x7AEF6323, (int32_t)0xDC597781, (int32_t)0x7AD33D45, (int32_t)0xDBF8F4F8, (int32_t)0x7AB6CBA4, (int32_t)0xDB9888A8, (int32_t)0x7A9A0E50, (int32_t)0xDB3832CD,
    (int32_t)0x7A7D055B, (int32_t)0xDAD7F3A2, (int32_t)0x7A5FB0D8, (int32_t)0xDA77CB63, (int32_t)0x7A4210D8, (int32_t)0xDA17BA4A, (int32_t)0x7A24256F, (int32_t)0xD9B7C094,
    (int32_t)0x7A05EEAD, (int32_t)0xD957DE7A, (int32_t)0x79E76CA7, (int32_t)0xD8F81439, (int32_t)0x79C89F6E, (int32_t)0xD898620C, (int32_t)0x79A98715, (int32_t)0xD838C82D,
    (int32_t)0x798A23B1, (int32_t)0xD7D946D8, (int32_t)0x796A7554, (int32_t)0xD779DE47, (int32_t)0x794A7C12, (int32_t)0xD71A8EB5, (int32_t)0x792A37FE, (int32_t)0xD6BB585E,
    (int32_t)0x7909A92D, (int32_t)0xD65C3B7B, (int32_t)0x78E8CFB2, (int32_t)0xD5FD3848, (int32_t)0x78C7ABA2, (int32_t)0xD59E4EFF, (int32_t)0x78A63D11, (int32_t)0xD53F7FDA,
    (int32_t)0x78848414, (int32_t)0xD4E0CB15, (int32_t)0x786280BF, (int32_t)0xD48230E9, (int32_t)0x78403329, (int32_t)0xD423B191, (int32_t)0x781D9B65, (int32_t)0xD3C54D47,
    (int32_t)0x77FAB989, (int32_t)0xD3670446, (int32_t)0x77D78DAA, (int32_t)0xD308D6C7, (int32_t)0x77B417DF, (int32_t)0xD2AAC504, (int32_t)0x7790583E, (int32_t)0xD24CCF39,
    (int32_t)0x776C4EDB, (int32_t)0xD1EEF59E, (int32_t)0x7747FBCE, (int32_t)0xD191386E, (int32_t)0x77235F2D, (int32_t)0xD13397E2, (int32_t)0x76FE790E, (int32_t)0xD0D61434,
    (int32_t)0x76D94989, (int32_t)0xD078AD9E, (int32_t)0x76B3D0B4, (int32_t)0xD01B6459, (int32_t)0x768E0EA6, (int32_t)0xCFBE389F, (int32_t)0x76680376, (int32_t)0xCF612AAA,
    (int32_t)0x7641AF3D, (int32_t)0xCF043AB3, (int32_t)0x761B1211, (int32_t)0xCEA768F2, (int32_t)0x75F42C0B, (int32_t)0xCE4AB5A2, (int32_t)0x75CCFD42, (int32_t)0xCDEE20FC,
    (int32_t)0x75A585CF, (int32_t)0xCD91AB39, (int32_t)0x757DC5CA, (int32_t)0xCD355491, (int32_t)0x7555BD4C, (int32_t)0xCCD91D3D, (int32_t)0x752D6C6C, (int32_t)0xCC7D0578,
    (int32_t)0x7504D345, (int32_t)0xCC210D79, (int32_t)0x74DBF1EF, (int32_t)0xCBC53579, (int32_t)0x74B2C884, (int32_t)0xCB697DB0, (int32_t)0x7489571C, (int32_t)0xCB0DE658,
    (int32_t)0x745F9DD1, (int32_t)0xCAB26FA9, (int32_t)0x74359CBD, (int32_t)0xCA5719DB, (int32_t)0x740B53FB, (int32_t)0xC9FBE527, (int32_t)0x73E0C3A3, (int32_t)0xC9A0D1C5,
    (int32_t)0x73B5EBD1, (int32_t)0xC945DFEC, (int32_t)0x738ACC9E, (int32_t)0xC8EB0FD6, (int32_t)0x735F6626, (int32_t)0xC89061BA, (int32_t)0x7333B883, (int32_t)0xC835D5D0,
    (int32_t)0x7307C3D0, (int32_t)0xC7DB6C50, (int32_t)0x72DB8828, (int32_t)0xC7812572, (int32_t)0x72AF05A7, (int32_t)0xC727016D, (int32_t)0x72823C67, (int32_t)0xC6CD0079,
    (int32_t)0x72552C85, (int32_t)0xC67322CE, (int32_t)0x7227D61C, (int32_t)0xC61968A2, (int32_t)0x71FA3949, (int32_t)0xC5BFD22E, (int32_t)0x71CC5626, (int32_t)0xC5665FA9,
    (int32_t)0x719E2CD2, (int32_t)0xC50D1149, (int32_t)0x716FBD68, (int32_t)0xC4B3E746, (int32_t)0x71410805, (int32_t)0xC45AE1D7, (int32_t)0x71120CC5, (int32_t)0xC4020133,
    (int32_t)0x70E2CBC6, (int32_t)0xC3A94590, (int32_t)0x70B34525, (int32_t)0xC350AF26, (int32_t)0x708378FF, (int32_t)0xC2F83E2A, (int32_t)0x70536771, (int32_t)0xC29FF2D4,
    (int32_t)0x7023109A, (int32_t)0xC247CD5A, (int32_t)0x6FF27497, (int32_t)0xC1EFCDF3, (int32_t)0x6FC19385, (int32_t)0xC197F4D4, (int32_t)0x6F906D84, (int32_t)0xC1404233,
    (int32_t)0x6F5F02B2, (int32_t)0xC0E8B648, (int32_t)0x6F2D532C, (int32_t)0xC0915148, (int32_t)0x6EFB5F12, (int32_t)0xC03A1368, (int32_t)0x6EC92683, (int32_t)0xBFE2FCDF,
    (int32_t)0x6E96A99D, (int32_t)0xBF8C0DE3, (int32_t)0x6E63E87F, (int32_t)0xBF3546A8, (int32_t)0x6E30E34A, (int32_t)0xBEDEA765, (int32_t)0x6DFD9A1C, (int32_t)0xBE88304F,
    (int32_t)0x6DCA0D14, (int32_t)0xBE31E19B, (int32_t)0x6D963C54, (int32_t)0xBDDBBB7F, (int32_t)0x6D6227FA, (int32_t)0xBD85BE30, (int32_t)0x6D2DD027, (int32_t)0xBD2FE9E2,
    (int32_t)0x6CF934FC, (int32_t)0xBCDA3ECB, (int32_t)0x6CC45698, (int32_t)0xBC84BD1F, (int32_t)0x6C8F351C, (int32_t)0xBC2F6513, (int32_t)0x6C59D0A9, (int32_t)0xBBDA36DD,
    (int32_t)0x6C242960, (int32_t)0xBB8532B0, (int32_t)0x6BEE3F62, (int32_t)0xBB3058C0, (int32_t)0x6BB812D1, (int32_t)0xBADBA943, (int32_t)0x6B81A3CD, (int32_t)0xBA87246D,
    (int32_t)0x6B4AF279, (int32_t)0xBA32CA71, (int32_t)0x6B13FEF5, (int32_t)0xB9DE9B83, (int32_t)0x6ADCC964, (int32_t)0xB98A97D8, (int32_t)0x6AA551E9, (int32_t)0xB936BFA4,
    (int32_t)0x6A6D98A4, (int32_t)0xB8E31319, (int32_t)0x6A359DB9, (int32_t)0xB88F926D, (int32_t)0x69FD614A, (int32_t)0xB83C3DD1, (int32_t)0x69C4E37A, (int32_t)0xB7E9157A,
    (int32_t)0x698C246C, (int32_t)0xB796199B, (int32_t)0x69532442, (int32_t)0xB7434A67, (int32_t)0x6919E320, (int32_t)0xB6F0A812, (int32_t)0x68E06129, (int32_t)0xB69E32CD,
    (int32_t)0x68A69E81, (int32_t)0xB64BEACD, (int32_t)0x686C9B4B, (int32_t)0xB5F9D043, (int32_t)0x683257AB, (int32_t)0xB5A7E362, (int32_t)0x67F7D3C5, (int32_t)0xB556245E,
    (int32_t)0x67BD0FBD, (int32_t)0xB5049368, (int32_t)0x67820BB7, (int32_t)0xB4B330B3, (int32_t)0x6746C7D8, (int32_t)0xB461FC70, (int32_t)0x670B4444, (int32_t)0xB410F6D3,
    (int32_t)0x66CF8120, (int32_t)0xB3C0200C, (int32_t)0x66937E91, (int32_t)0xB36F784F, (int32_t)0x66573CBB, (int32_t)0xB31EFFCC, (int32_t)0x661ABBC5, (int32_t)0xB2CEB6B5,
    (int32_t)0x65DDFBD3, (int32_t)0xB27E9D3C, (int32_t)0x65A0FD0B, (int32_t)0xB22EB392, (int32_t)0x6563BF92, (int32_t)0xB1DEF9E9, (int32_t)0x6526438F, (int32_t)0xB18F7071,
    (int32_t)0x64E88926, (int32_t)0xB140175B, (int32_t)0x64AA907F, (int32_t)0xB0F0EEDA, (int32_t)0x646C59BF, (int32_t)0xB0A1F71D, (int32_t)0x642DE50D, (int32_t)0xB0533055,
    (int32_t)0x63EF3290, (int32_t)0xB0049AB3, (int32_t)0x63B0426D, (int32_t)0xAFB63667, (int32_t)0x637114CC, (int32_t)0xAF6803A2, (int32_t)0x6331A9D4, (int32_t)0xAF1A0293,
    (int32_t)0x62F201AC, (int32_t)0xAECC336C, (int32_t)0x62B21C7B, (int32_t)0xAE7E965B, (int32_t)0x6271FA69, (int32_t)0xAE312B92, (int32_t)0x62319B9D, (int32_t)0xADE3F33E,
    (int32_t)0x61F1003F, (int32_t)0xAD96ED92, (int32_t)0x61B02876, (int32_t)0xAD4A1ABA, (int32_t)0x616F146C, (int32_t)0xACFD7AE8, (int32_t)0x612DC447, (int32_t)0xACB10E4B,
    (int32_t)0x60EC3830, (int32_t)0xAC64D510, (int32_t)0x60AA7050, (int32_t)0xAC18CF69, (int32_t)0x60686CCF, (int32_t)0xABCCFD83, (int32_t)0x60262DD6, (int32_t)0xAB815F8D,
    (int32_t)0x5FE3B38D, (int32_t)0xAB35F5B5, (int32_t)0x5FA0FE1F, (int32_t)0xAAEAC02C, (int32_t)0x5F5E0DB3, (int32_t)0xAA9FBF1E, (int32_t)0x5F1AE274, (int32_t)0xAA54F2BA,
    (int32_t)0x5ED77C8A, (int32_t)0xAA0A5B2E, (int32_t)0x5E93DC1F, (int32_t)0xA9BFF8A8, (int32_t)0x5E50015D, (int32_t)0xA975CB57, (int32_t)0x5E0BEC6E, (int32_t)0xA92BD367,
    (int32_t)0x5DC79D7C, (int32_t)0xA8E21106, (int32_t)0x5D8314B1, (int32_t)0xA8988463, (int32_t)0x5D3E5237, (int32_t)0xA84F2DAA, (int32_t)0x5CF95638, (int32_t)0xA8060D08,
    (int32_t)0x5CB420E0, (int32_t)0xA7BD22AC, (int32_t)0x5C6EB258, (int32_t)0xA7746EC0, (int32_t)0x5C290ACC, (int32_t)0xA72BF174, (int32_t)0x5BE32A67, (int32_t)0xA6E3AAF2,
    (int32_t)0x5B9D1154, (int32_t)0xA69B9B68, (int32_t)0x5B56BFBD, (int32_t)0xA653C303, (int32_t)0x5B1035CF, (int32_t)0xA60C21EE, (int32_t)0x5AC973B5, (int32_t)0xA5C4B855,
    (int32_t)0x5A82799A, (int32_t)0xA57D8666, (int32_t)0x5A3B47AB, (int32_t)0xA5368C4B, (int32_t)0x59F3DE12, (int32_t)0xA4EFCA31, (int32_t)0x59AC3CFD, (int32_t)0xA4A94043,
    (int32_t)0x59646498, (int32_t)0xA462EEAC, (int32_t)0x591C550E, (int32_t)0xA41CD599, (int32_t)0x58D40E8C, (int32_t)0xA3D6F534, (int32_t)0x588B9140, (int32_t)0xA3914DA8,
    (int32_t)0x5842DD54, (int32_t)0xA34BDF20, (int32_t)0x57F9F2F8, (int32_t)0xA306A9C8, (int32_t)0x57B0D256, (int32_t)0xA2C1ADC9, (int32_t)0x57677B9D, (int32_t)0xA27CEB4F,
    (int32_t)0x571DEEFA, (int32_t)0xA2386284, (int32_t)0x56D42C99, (int32_t)0xA1F41392, (int32_t)0x568A34A9, (int32_t)0xA1AFFEA3, (int32_t)0x56400758, (int32_t)0xA16C23E1,
    (int32_t)0x55F5A4D2, (int32_t)0xA1288376, (int32_t)0x55AB0D46, (int32_t)0xA0E51D8C, (int32_t)0x556040E2, (int32_t)0xA0A1F24D, (int32_t)0x55153FD4, (int32_t)0xA05F01E1,
    (int32_t)0x54CA0A4B, (int32_t)0xA01C4C73, (int32_t)0x547EA073, (int32_t)0x9FD9D22A, (int32_t)0x5433027D, (int32_t)0x9F979331, (int32_t)0x53E73097, (int32_t)0x9F558FB0,
    (int32_t)0x539B2AF0, (int32_t)0x9F13C7D0, (int32_t)0x534EF1B5, (int32_t)0x9ED23BB9, (int32_t)0x53028518, (int32_t)0x9E90EB94, (int32_t)0x52B5E546, (int32_t)0x9E4FD78A,
    (int32_t)0x5269126E, (int32_t)0x9E0EFFC1, (int32_t)0x521C0CC2, (int32_t)0x9DCE6463, (int32_t)0x51CED46E, (int32_t)0x9D8E0597, (int32_t)0x518169A5, (int32_t)0x9D4DE385,
    (int32_t)0x5133CC94, (int32_t)0x9D0DFE54, (int32_t)0x50E5FD6D, (int32_t)0x9CCE562C, (int32_t)0x5097FC5E, (int32_t)0x9C8EEB34, (int32_t)0x5049C999, (int32_t)0x9C4FBD93,
    (int32_t)0x4FFB654D, (int32_t)0x9C10CD70, (int32_t)0x4FACCFAB, (int32_t)0x9BD21AF3, (int32_t)0x4F5E08E3, (int32_t)0x9B93A641, (int32_t)0x4F0F1126, (int32_t)0x9B556F81,
    (int32_t)0x4EBFE8A5, (int32_t)0x9B1776DA, (int32_t)0x4E708F8F, (int32_t)0x9AD9BC71, (int32_t)0x4E210617, (int32_t)0x9A9C406E, (int32_t)0x4DD14C6E, (int32_t)0x9A5F02F5,
    (int32_t)0x4D8162C4, (int32_t)0x9A22042D, (int32_t)0x4D31494B, (int32_t)0x99E5443B, (int32_t)0x4CE10034, (int32_t)0x99A8C345, (int32_t)0x4C9087B1, (int32_t)0x996C816F,
    (int32_t)0x4C3FDFF4, (int32_t)0x99307EE0, (int32_t)0x4BEF092D, (int32_t)0x98F4BBBC, (int32_t)0x4B9E0390, (int32_t)0x98B93828, (int32_t)0x4B4CCF4D, (int32_t)0x987DF449,
    (int32_t)0x4AFB6C98, (int32_t)0x9842F043, (int32_t)0x4AA9DBA2, (int32_t)0x98082C3B, (int32_t)0x4A581C9E, (int32_t)0x97CDA855, (int32_t)0x4A062FBD, (int32_t)0x979364B5,
    (int32_t)0x49B41533, (int32_t)0x9759617F, (int32_t)0x4961CD33, (int32_t)0x971F9ED7, (int32_t)0x490F57EE, (int32_t)0x96E61CE0, (int32_t)0x48BCB599, (int32_t)0x96ACDBBE,
    (int32_t)0x4869E665, (int32_t)0x9673DB94, (int32_t)0x4816EA86, (int32_t)0x963B1C86, (int32_t)0x47C3C22F, (int32_t)0x96029EB6, (int32_t)0x47706D93, (int32_t)0x95CA6247,
    (int32_t)0x471CECE7, (int32_t)0x9592675C, (int32_t)0x46C9405C, (int32_t)0x955AAE17, (int32_t)0x46756828, (int32_t)0x9523369C, (int32_t)0x4621647D, (int32_t)0x94EC010B,
    (int32_t)0x45CD358F, (int32_t)0x94B50D87, (int32_t)0x4578DB93, (int32_t)0x947E5C33, (int32_t)0x452456BD, (int32_t)0x9447ED2F, (int32_t)0x44CFA740, (int32_t)0x9411C09E,
    (int32_t)0x447ACD50, (int32_t)0x93DBD6A0, (int32_t)0x4425C923, (int32_t)0x93A62F57, (int32_t)0x43D09AED, (int32_t)0x9370CAE4, (int32_t)0x437B42E1, (int32_t)0x933BA968,
    (int32_t)0x4325C135, (int32_t)0x9306CB04, (int32_t)0x42D0161E, (int32_t)0x92D22FD9, (int32_t)0x427A41D0, (int32_t)0x929DD806, (int32_t)0x42244481, (int32_t)0x9269C3AC,
    (int32_t)0x41CE1E65, (int32_t)0x9235F2EC, (int32_t)0x4177CFB1, (int32_t)0x920265E4, (int32_t)0x4121589B, (int32_t)0x91CF1CB6, (int32_t)0x40CAB958, (int32_t)0x919C1781,
    (int32_t)0x4073F21D, (int32_t)0x91695663, (int32_t)0x401D0321, (int32_t)0x9136D97D, (int32_t)0x3FC5EC98, (int32_t)0x9104A0EE, (int32_t)0x3F6EAEB8, (int32_t)0x90D2ACD4,
    (int32_t)0x3F1749B8, (int32_t)0x90A0FD4E, (int32_t)0x3EBFBDCD, (int32_t)0x906F927C, (int32_t)0x3E680B2C, (int32_t)0x903E6C7B, (int32_t)0x3E10320D, (int32_t)0x900D8B69,
    (int32_t)0x3DB832A6, (int32_t)0x8FDCEF66, (int32_t)0x3D600D2C, (int32_t)0x8FAC988F, (int32_t)0x3D07C1D6, (int32_t)0x8F7C8701, (int32_t)0x3CAF50DA, (int32_t)0x8F4CBADB,
    (int32_t)0x3C56BA70, (int32_t)0x8F1D343A, (int32_t)0x3BFDFECD, (int32_t)0x8EEDF33B, (int32_t)0x3BA51E29, (int32_t)0x8EBEF7FB, (int32_t)0x3B4C18BA, (int32_t)0x8E904298,
    (int32_t)0x3AF2EEB7, (int32_t)0x8E61D32E, (int32_t)0x3A99A057, (int32_t)0x8E33A9DA, (int32_t)0x3A402DD2, (int32_t)0x8E05C6B7, (int32_t)0x39E6975E, (int32_t)0x8DD829E4,
    (int32_t)0x398CDD32, (int32_t)0x8DAAD37B, (int32_t)0x3932FF87, (int32_t)0x8D7DC399, (int32_t)0x38D8FE93, (int32_t)0x8D50FA59, (int32_t)0x387EDA8E, (int32_t)0x8D2477D8,
    (int32_t)0x382493B0, (int32_t)0x8CF83C30, (int32_t)0x37CA2A30, (int32_t)0x8CCC477D, (int32_t)0x376F9E46, (int32_t)0x8CA099DA, (int32_t)0x3714F02A, (int32_t)0x8C753362,
    (int32_t)0x36BA2014, (int32_t)0x8C4A142F, (int32_t)0x365F2E3B, (int32_t)0x8C1F3C5D, (int32_t)0x36041AD9, (int32_t)0x8BF4AC05, (int32_t)0x35A8E625, (int32_t)0x8BCA6343,
    (int32_t)0x354D9057, (int32_t)0x8BA0622F, (int32_t)0x34F219A8, (int32_t)0x8B76A8E4, (int32_t)0x34968250, (int32_t)0x8B4D377C, (int32_t)0x343ACA87, (int32_t)0x8B240E11,
    (int32_t)0x33DEF287, (int32_t)0x8AFB2CBB, (int32_t)0x3382FA88, (int32_t)0x8AD29394, (int32_t)0x3326E2C3, (int32_t)0x8AAA42B4, (int32_t)0x32CAAB6F, (int32_t)0x8A823A36,
    (int32_t)0x326E54C7, (int32_t)0x8A5A7A31, (int32_t)0x3211DF04, (int32_t)0x8A3302BE, (int32_t)0x31B54A5E, (int32_t)0x8A0BD3F5, (int32_t)0x3158970E, (int32_t)0x89E4EDEF,
    (int32_t)0x30FBC54D, (int32_t)0x89BE50C3, (int32_t)0x309ED556, (int32_t)0x8997FC8A, (int32_t)0x3041C761, (int32_t)0x8971F15A, (int32_t)0x2FE49BA7, (int32_t)0x894C2F4C,
    (int32_t)0x2F875262, (int32_t)0x8926B677, (int32_t)0x2F29EBCC, (int32_t)0x890186F2, (int32_t)0x2ECC681E, (int32_t)0x88DCA0D3, (int32_t)0x2E6EC792, (int32_t)0x88B80432,
    (int32_t)0x2E110A62, (int32_t)0x8893B125, (int32_t)0x2DB330C7, (int32_t)0x886FA7C2, (int32_t)0x2D553AFC, (int32_t)0x884BE821, (int32_t)0x2CF72939, (int32_t)0x88287256,
    (int32_t)0x2C98FBBA, (int32_t)0x88054677, (int32_t)0x2C3AB2B9, (int32_t)0x87E2649B, (int32_t)0x2BDC4E6F, (int32_t)0x87BFCCD7, (int32_t)0x2B7DCF17, (int32_t)0x879D7F41,
    (int32_t)0x2B1F34EB, (int32_t)0x877B7BEC, (int32_t)0x2AC08026, (int32_t)0x8759C2EF, (int32_t)0x2A61B101, (int32_t)0x8738545E, (int32_t)0x2A02C7B8, (int32_t)0x8717304E,
    (int32_t)0x29A3C485, (int32_t)0x86F656D3, (int32_t)0x2944A7A2, (int32_t)0x86D5C802, (int32_t)0x28E5714B, (int32_t)0x86B583EE, (int32_t)0x288621B9, (int32_t)0x86958AAC,
    (int32_t)0x2826B928, (int32_t)0x8675DC4F, (int32_t)0x27C737D3, (int32_t)0x865678EB, (int32_t)0x27679DF4, (int32_t)0x86376092, (int32_t)0x2707EBC7, (int32_t)0x86189359,
    (int32_t)0x26A82186, (int32_t)0x85FA1153, (int32_t)0x26483F6C, (int32_t)0x85DBDA91, (int32_t)0x25E845B6, (int32_t)0x85BDEF28, (int32_t)0x2588349D, (int32_t)0x85A04F28,
    (int32_t)0x25280C5E, (int32_t)0x8582FAA5, (int32_t)0x24C7CD33, (int32_t)0x8565F1B0, (int32_t)0x24677758, (int32_t)0x8549345C, (int32_t)0x24070B08, (int32_t)0x852CC2BB,
    (int32_t)0x23A6887F, (int32_t)0x85109CDD, (int32_t)0x2345EFF8, (int32_t)0x84F4C2D4, (int32_t)0x22E541AF, (int32_t)0x84D934B1, (int32_t)0x22847DE0, (int32_t)0x84BDF286,
    (int32_t)0x2223A4C5, (int32_t)0x84A2FC62, (int32_t)0x21C2B69C, (int32_t)0x84885258, (int32_t)0x2161B3A0, (int32_t)0x846DF477, (int32_t)0x21009C0C, (int32_t)0x8453E2CF,
    (int32_t)0x209F701C, (int32_t)0x843A1D70, (int32_t)0x203E300D, (int32_t)0x8420A46C, (int32_t)0x1FDCDC1B, (int32_t)0x840777D0, (int32_t)0x1F7B7481, (int32_t)0x83EE97AD,
    (int32_t)0x1F19F97B, (int32_t)0x83D60412, (int32_t)0x1EB86B46, (int32_t)0x83BDBD0E, (int32_t)0x1E56CA1E, (int32_t)0x83A5C2B0, (int32_t)0x1DF5163F, (int32_t)0x838E1507,
    (int32_t)0x1D934FE5, (int32_t)0x8376B422, (int32_t)0x1D31774D, (int32_t)0x835FA00F, (int32_t)0x1CCF8CB3, (int32_t)0x8348D8DC, (int32_t)0x1C6D9053, (int32_t)0x83325E97,
    (int32_t)0x1C0B826A, (int32_t)0x831C314E, (int32_t)0x1BA96335, (int32_t)0x83065110, (int32_t)0x1B4732EF, (int32_t)0x82F0BDE8, (int32_t)0x1AE4F1D6, (int32_t)0x82DB77E5,
    (int32_t)0x1A82A026, (int32_t)0x82C67F14, (int32_t)0x1A203E1B, (int32_t)0x82B1D381, (int32_t)0x19BDCBF3, (int32_t)0x829D753A, (int32_t)0x195B49EA, (int32_t)0x8289644B,
    (int32_t)0x18F8B83C, (int32_t)0x8275A0C0, (int32_t)0x18961728, (int32_t)0x82622AA6, (int32_t)0x183366E9, (int32_t)0x824F0208, (int32_t)0x17D0A7BC, (int32_t)0x823C26F3,
    (int32_t)0x176DD9DE, (int32_t)0x82299971, (int32_t)0x170AFD8D, (int32_t)0x82175990, (int32_t)0x16A81305, (int32_t)0x82056758, (int32_t)0x16451A83, (int32_t)0x81F3C2D7,
    (int32_t)0x15E21445, (int32_t)0x81E26C16, (int32_t)0x157F0086, (int32_t)0x81D16321, (int32_t)0x151BDF86, (int32_t)0x81C0A801, (int32_t)0x14B8B17F, (int32_t)0x81B03AC2,
    (int32_t)0x145576B1, (int32_t)0x81A01B6D, (int32_t)0x13F22F58, (int32_t)0x81904A0C, (int32_t)0x138EDBB1, (int32_t)0x8180C6A9, (int32_t)0x132B7BF9, (int32_t)0x8171914E,
    (int32_t)0x12C8106F, (int32_t)0x8162AA04, (int32_t)0x1264994E, (int32_t)0x815410D4, (int32_t)0x120116D5, (int32_t)0x8145C5C7, (int32_t)0x119D8941, (int32_t)0x8137C8E6,
    (int32_t)0x1139F0CF, (int32_t)0x812A1A3A, (int32_t)0x10D64DBD, (int32_t)0x811CB9CA, (int32_t)0x1072A048, (int32_t)0x810FA7A0, (int32_t)0x100EE8AD, (int32_t)0x8102E3C4,
    (int32_t)0x0FAB272B, (int32_t)0x80F66E3C, (int32_t)0x0F475BFF, (int32_t)0x80EA4712, (int32_t)0x0EE38766, (int32_t)0x80DE6E4C, (int32_t)0x0E7FA99E, (int32_t)0x80D2E3F2,
    (int32_t)0x0E1BC2E4, (int32_t)0x80C7A80A, (int32_t)0x0DB7D376, (int32_t)0x80BCBA9D, (int32_t)0x0D53DB92, (int32_t)0x80B21BAF, (int32_t)0x0CEFDB76, (int32_t)0x80A7CB49,
    (int32_t)0x0C8BD35E, (int32_t)0x809DC971, (int32_t)0x0C27C389, (int32_t)0x8094162C, (int32_t)0x0BC3AC35, (int32_t)0x808AB180, (int32_t)0x0B5F8D9F, (int32_t)0x80819B74,
    (int32_t)0x0AFB6805, (int32_t)0x8078D40D, (int32_t)0x0A973BA5, (int32_t)0x80705B50, (int32_t)0x0A3308BD, (int32_t)0x80683143, (int32_t)0x09CECF89, (int32_t)0x806055EB,
    (int32_t)0x096A9049, (int32_t)0x8058C94C, (int32_t)0x09064B3A, (int32_t)0x80518B6B, (int32_t)0x08A2009A, (int32_t)0x804A9C4D, (int32_t)0x083DB0A7, (int32_t)0x8043FBF6,
    (int32_t)0x07D95B9E, (int32_t)0x803DAA6A, (int32_t)0x077501BE, (int32_t)0x8037A7AC, (int32_t)0x0710A345, (int32_t)0x8031F3C2, (int32_t)0x06AC406F, (int32_t)0x802C8EAD,
    (int32_t)0x0647D97C, (int32_t)0x80277872, (int32_t)0x05E36EA9, (int32_t)0x8022B114, (int32_t)0x057F0035, (int32_t)0x801E3895, (int32_t)0x051A8E5C, (int32_t)0x801A0EF8,
    (int32_t)0x04B6195D, (int32_t)0x80163440, (int32_t)0x0451A177, (int32_t)0x8012A86F, (int32_t)0x03ED26E6, (int32_t)0x800F6B88, (int32_t)0x0388A9EA, (int32_t)0x800C7D8C,
    (int32_t)0x03242ABF, (int32_t)0x8009DE7E, (int32_t)0x02BFA9A4, (int32_t)0x80078E5E, (int32_t)0x025B26D7, (int32_t)0x80058D2F, (int32_t)0x01F6A297, (int32_t)0x8003DAF1,
    (int32_t)0x01921D20, (int32_t)0x800277A6, (int32_t)0x012D96B1, (int32_t)0x8001634E, (int32_t)0x00C90F88, (int32_t)0x80009DEA, (int32_t)0x006487E3, (int32_t)0x8000277A,
};


static const fft_real32x32_descr_t __rfft_descr =
{
    (const fft_handle_t)&__cfft_descr1024_32x32,
    __fft_real_tw
};
static const fft_real32x32_descr_t __rifft_descr =
{
    (const fft_handle_t)&__cifft_descr1024_32x32,
    __fft_real_tw
};

const fft_handle_t rfft32_2048 = (const fft_handle_t)&__rfft_descr;
const fft_handle_t rifft32_2048 = (const fft_handle_t)&__rifft_descr;
