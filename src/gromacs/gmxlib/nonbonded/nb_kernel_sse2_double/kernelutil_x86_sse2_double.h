/*
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 * Copyright (c) 2011-2012, The GROMACS Development Team
 *
 * Gromacs is a library for molecular simulation and trajectory analysis,
 * written by Erik Lindahl, David van der Spoel, Berk Hess, and others - for
 * a full list of developers and information, check out http://www.gromacs.org
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU Lesser General Public License.
 *
 * In plain-speak: do not worry about classes/macros/templates either - only
 * changes to the library have to be LGPL, not an application linking with it.
 *
 * To help fund GROMACS development, we humbly ask that you cite
 * the papers people have written on it - you can find them on the website!
 */
#ifndef _kernelutil_x86_sse2_double_h_
#define _kernelutil_x86_sse2_double_h_

#include <math.h>

#include "gmx_x86_sse2.h"

#include <stdio.h>


/* Normal sum of four ymm registers */
#define gmx_mm_sum4_pd(t0,t1,t2,t3)  _mm_add_pd(_mm_add_pd(t0,t1),_mm_add_pd(t2,t3))

static int
gmx_mm_any_lt(__m128d a, __m128d b)
{
    return _mm_movemask_pd(_mm_cmplt_pd(a,b));
}

static gmx_inline __m128d
gmx_mm_calc_rsq_pd(__m128d dx, __m128d dy, __m128d dz)
{
    return _mm_add_pd( _mm_add_pd( _mm_mul_pd(dx,dx), _mm_mul_pd(dy,dy) ), _mm_mul_pd(dz,dz) );
}


/* Load a double value from 1-2 places, merge into xmm register */
static gmx_inline __m128d
gmx_mm_load_2real_swizzle_pd(const double * gmx_restrict ptrA,
                             const double * gmx_restrict ptrB)
{
    return _mm_unpacklo_pd(_mm_load_sd(ptrA),_mm_load_sd(ptrB));
}

static gmx_inline __m128d
gmx_mm_load_1real_pd(const double * gmx_restrict ptrA)
{
    return _mm_load_sd(ptrA);
}


static gmx_inline void
gmx_mm_store_2real_swizzle_pd(double * gmx_restrict ptrA,
                              double * gmx_restrict ptrB,
                              __m128d xmm1)
{
    __m128d t2;

    t2       = _mm_unpackhi_pd(xmm1,xmm1);
    _mm_store_sd(ptrA,xmm1);
    _mm_store_sd(ptrB,t2);
}

static gmx_inline void
gmx_mm_store_1real_pd(double * gmx_restrict ptrA, __m128d xmm1)
{
    _mm_store_sd(ptrA,xmm1);
}


/* Similar to store, but increments value in memory */
static gmx_inline void
gmx_mm_increment_2real_swizzle_pd(double * gmx_restrict ptrA,
                                  double * gmx_restrict ptrB, __m128d xmm1)
{
    __m128d t1;

    t1   = _mm_unpackhi_pd(xmm1,xmm1);
    xmm1 = _mm_add_sd(xmm1,_mm_load_sd(ptrA));
    t1   = _mm_add_sd(t1,_mm_load_sd(ptrB));
    _mm_store_sd(ptrA,xmm1);
    _mm_store_sd(ptrB,t1);
}

static gmx_inline void
gmx_mm_increment_1real_pd(double * gmx_restrict ptrA, __m128d xmm1)
{
    __m128d tmp;

    tmp = gmx_mm_load_1real_pd(ptrA);
    tmp = _mm_add_sd(tmp,xmm1);
    gmx_mm_store_1real_pd(ptrA,tmp);
}


static gmx_inline void
gmx_mm_load_2pair_swizzle_pd(const double * gmx_restrict p1,
                             const double * gmx_restrict p2,
                             __m128d * gmx_restrict c6,
                             __m128d * gmx_restrict c12)
{
    __m128d t1,t2,t3;

    t1   = _mm_loadu_pd(p1);
    t2   = _mm_loadu_pd(p2);
    *c6  = _mm_unpacklo_pd(t1,t2);
    *c12 = _mm_unpackhi_pd(t1,t2);
}

static gmx_inline void
gmx_mm_load_1pair_swizzle_pd(const double * gmx_restrict p1,
                             __m128d * gmx_restrict c6,
                             __m128d * gmx_restrict c12)
{
    *c6     = _mm_load_sd(p1);
    *c12    = _mm_load_sd(p1+1);
}



static gmx_inline void
gmx_mm_load_shift_and_1rvec_broadcast_pd(const double * gmx_restrict xyz_shift,
                                         const double * gmx_restrict xyz,
                                         __m128d * gmx_restrict x1,
                                         __m128d * gmx_restrict y1,
                                         __m128d * gmx_restrict z1)
{
    __m128d mem_xy,mem_z,mem_sxy,mem_sz;

    mem_xy  = _mm_loadu_pd(xyz);
    mem_z   = _mm_load_sd(xyz+2);
    mem_sxy = _mm_loadu_pd(xyz_shift);
    mem_sz  = _mm_load_sd(xyz_shift+2);

    mem_xy  = _mm_add_pd(mem_xy,mem_sxy);
    mem_z   = _mm_add_pd(mem_z,mem_sz);

    *x1  = _mm_shuffle_pd(mem_xy,mem_xy,_MM_SHUFFLE2(0,0));
    *y1  = _mm_shuffle_pd(mem_xy,mem_xy,_MM_SHUFFLE2(1,1));
    *z1  = _mm_shuffle_pd(mem_z,mem_z,_MM_SHUFFLE2(0,0));
}


static gmx_inline void
gmx_mm_load_shift_and_3rvec_broadcast_pd(const double * gmx_restrict xyz_shift,
                                         const double * gmx_restrict xyz,
                                         __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1,
                                         __m128d * gmx_restrict x2, __m128d * gmx_restrict y2, __m128d * gmx_restrict z2,
                                         __m128d * gmx_restrict x3, __m128d * gmx_restrict y3, __m128d * gmx_restrict z3)
{
    __m128d t1,t2,t3,t4,t5,sxy,sz,szx,syz;

    t1  = _mm_loadu_pd(xyz);
    t2  = _mm_loadu_pd(xyz+2);
    t3  = _mm_loadu_pd(xyz+4);
    t4  = _mm_loadu_pd(xyz+6);
    t5  = _mm_load_sd(xyz+8);

    sxy = _mm_loadu_pd(xyz_shift);
    sz  = _mm_load_sd(xyz_shift+2);
    szx = _mm_shuffle_pd(sz,sxy,_MM_SHUFFLE2(0,0));
    syz = _mm_shuffle_pd(sxy,sz,_MM_SHUFFLE2(0,1));

    t1  = _mm_add_pd(t1,sxy);
    t2  = _mm_add_pd(t2,szx);
    t3  = _mm_add_pd(t3,syz);
    t4  = _mm_add_pd(t4,sxy);
    t5  = _mm_add_sd(t5,sz);

    *x1  = _mm_shuffle_pd(t1,t1,_MM_SHUFFLE2(0,0));
    *y1  = _mm_shuffle_pd(t1,t1,_MM_SHUFFLE2(1,1));
    *z1  = _mm_shuffle_pd(t2,t2,_MM_SHUFFLE2(0,0));
    *x2  = _mm_shuffle_pd(t2,t2,_MM_SHUFFLE2(1,1));
    *y2  = _mm_shuffle_pd(t3,t3,_MM_SHUFFLE2(0,0));
    *z2  = _mm_shuffle_pd(t3,t3,_MM_SHUFFLE2(1,1));
    *x3  = _mm_shuffle_pd(t4,t4,_MM_SHUFFLE2(0,0));
    *y3  = _mm_shuffle_pd(t4,t4,_MM_SHUFFLE2(1,1));
    *z3  = _mm_shuffle_pd(t5,t5,_MM_SHUFFLE2(0,0));
}


static gmx_inline void
gmx_mm_load_shift_and_4rvec_broadcast_pd(const double * gmx_restrict xyz_shift,
                                         const double * gmx_restrict xyz,
                                         __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1,
                                         __m128d * gmx_restrict x2, __m128d * gmx_restrict y2, __m128d * gmx_restrict z2,
                                         __m128d * gmx_restrict x3, __m128d * gmx_restrict y3, __m128d * gmx_restrict z3,
                                         __m128d * gmx_restrict x4, __m128d * gmx_restrict y4, __m128d * gmx_restrict z4)
{
    __m128d t1,t2,t3,t4,t5,t6,sxy,sz,szx,syz;

    t1  = _mm_loadu_pd(xyz);
    t2  = _mm_loadu_pd(xyz+2);
    t3  = _mm_loadu_pd(xyz+4);
    t4  = _mm_loadu_pd(xyz+6);
    t5  = _mm_loadu_pd(xyz+8);
    t6  = _mm_loadu_pd(xyz+10);

    sxy = _mm_loadu_pd(xyz_shift);
    sz  = _mm_load_sd(xyz_shift+2);
    szx = _mm_shuffle_pd(sz,sxy,_MM_SHUFFLE2(0,0));
    syz = _mm_shuffle_pd(sxy,sz,_MM_SHUFFLE2(0,1));

    t1  = _mm_add_pd(t1,sxy);
    t2  = _mm_add_pd(t2,szx);
    t3  = _mm_add_pd(t3,syz);
    t4  = _mm_add_pd(t4,sxy);
    t5  = _mm_add_pd(t5,szx);
    t6  = _mm_add_pd(t6,syz);

    *x1  = _mm_shuffle_pd(t1,t1,_MM_SHUFFLE2(0,0));
    *y1  = _mm_shuffle_pd(t1,t1,_MM_SHUFFLE2(1,1));
    *z1  = _mm_shuffle_pd(t2,t2,_MM_SHUFFLE2(0,0));
    *x2  = _mm_shuffle_pd(t2,t2,_MM_SHUFFLE2(1,1));
    *y2  = _mm_shuffle_pd(t3,t3,_MM_SHUFFLE2(0,0));
    *z2  = _mm_shuffle_pd(t3,t3,_MM_SHUFFLE2(1,1));
    *x3  = _mm_shuffle_pd(t4,t4,_MM_SHUFFLE2(0,0));
    *y3  = _mm_shuffle_pd(t4,t4,_MM_SHUFFLE2(1,1));
    *z3  = _mm_shuffle_pd(t5,t5,_MM_SHUFFLE2(0,0));
    *x4  = _mm_shuffle_pd(t5,t5,_MM_SHUFFLE2(1,1));
    *y4  = _mm_shuffle_pd(t6,t6,_MM_SHUFFLE2(0,0));
    *z4  = _mm_shuffle_pd(t6,t6,_MM_SHUFFLE2(1,1));
}




static gmx_inline void
gmx_mm_load_1rvec_1ptr_swizzle_pd(const double * gmx_restrict p1,
                                  __m128d * gmx_restrict x, __m128d * gmx_restrict y, __m128d * gmx_restrict z)
{
	 *x            = _mm_load_sd(p1);
     *y            = _mm_load_sd(p1+1);
     *z            = _mm_load_sd(p1+2);
}

static gmx_inline void
gmx_mm_load_3rvec_1ptr_swizzle_pd(const double * gmx_restrict p1,
                                  __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1,
                                  __m128d * gmx_restrict x2, __m128d * gmx_restrict y2, __m128d * gmx_restrict z2,
                                  __m128d * gmx_restrict x3, __m128d * gmx_restrict y3, __m128d * gmx_restrict z3)
{
	 *x1            = _mm_load_sd(p1);
     *y1            = _mm_load_sd(p1+1);
     *z1            = _mm_load_sd(p1+2);
	 *x2            = _mm_load_sd(p1+3);
     *y2            = _mm_load_sd(p1+4);
     *z2            = _mm_load_sd(p1+5);
	 *x3            = _mm_load_sd(p1+6);
     *y3            = _mm_load_sd(p1+7);
     *z3            = _mm_load_sd(p1+8);
}

static gmx_inline void
gmx_mm_load_4rvec_1ptr_swizzle_pd(const double * gmx_restrict p1,
                                  __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1,
                                  __m128d * gmx_restrict x2, __m128d * gmx_restrict y2, __m128d * gmx_restrict z2,
                                  __m128d * gmx_restrict x3, __m128d * gmx_restrict y3, __m128d * gmx_restrict z3,
                                  __m128d * gmx_restrict x4, __m128d * gmx_restrict y4, __m128d * gmx_restrict z4)
{
    *x1            = _mm_load_sd(p1);
    *y1            = _mm_load_sd(p1+1);
    *z1            = _mm_load_sd(p1+2);
    *x2            = _mm_load_sd(p1+3);
    *y2            = _mm_load_sd(p1+4);
    *z2            = _mm_load_sd(p1+5);
    *x3            = _mm_load_sd(p1+6);
    *y3            = _mm_load_sd(p1+7);
    *z3            = _mm_load_sd(p1+8);
    *x4            = _mm_load_sd(p1+9);
    *y4            = _mm_load_sd(p1+10);
    *z4            = _mm_load_sd(p1+11);
}


static gmx_inline void
gmx_mm_load_1rvec_2ptr_swizzle_pd(const double * gmx_restrict ptrA,
                                  const double * gmx_restrict ptrB,
                                  __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1)
{
    __m128d t1,t2,t3,t4;
    t1           = _mm_loadu_pd(ptrA);
    t2           = _mm_loadu_pd(ptrB);
    t3           = _mm_load_sd(ptrA+2);
    t4           = _mm_load_sd(ptrB+2);
    GMX_MM_TRANSPOSE2_PD(t1,t2);
    *x1          = t1;
    *y1          = t2;
    *z1          = _mm_unpacklo_pd(t3,t4);
}


static gmx_inline void
gmx_mm_load_3rvec_2ptr_swizzle_pd(const double * gmx_restrict ptrA, const double * gmx_restrict ptrB,
                                  __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1,
                                  __m128d * gmx_restrict x2, __m128d * gmx_restrict y2, __m128d * gmx_restrict z2,
                                  __m128d * gmx_restrict x3, __m128d * gmx_restrict y3, __m128d * gmx_restrict z3)
{
    __m128d t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
    t1           = _mm_loadu_pd(ptrA);
    t2           = _mm_loadu_pd(ptrB);
    t3           = _mm_loadu_pd(ptrA+2);
    t4           = _mm_loadu_pd(ptrB+2);
    t5           = _mm_loadu_pd(ptrA+4);
    t6           = _mm_loadu_pd(ptrB+4);
    t7           = _mm_loadu_pd(ptrA+6);
    t8           = _mm_loadu_pd(ptrB+6);
    t9           = _mm_load_sd(ptrA+8);
    t10          = _mm_load_sd(ptrB+8);
    GMX_MM_TRANSPOSE2_PD(t1,t2);
    GMX_MM_TRANSPOSE2_PD(t3,t4);
    GMX_MM_TRANSPOSE2_PD(t5,t6);
    GMX_MM_TRANSPOSE2_PD(t7,t8);
    *x1          = t1;
    *y1          = t2;
    *z1          = t3;
    *x2          = t4;
    *y2          = t5;
    *z2          = t6;
    *x3          = t7;
    *y3          = t8;
    *z3          = _mm_unpacklo_pd(t9,t10);
}


static gmx_inline void
gmx_mm_load_4rvec_2ptr_swizzle_pd(const double * gmx_restrict ptrA, const double * gmx_restrict ptrB,
                                  __m128d * gmx_restrict x1, __m128d * gmx_restrict y1, __m128d * gmx_restrict z1,
                                  __m128d * gmx_restrict x2, __m128d * gmx_restrict y2, __m128d * gmx_restrict z2,
                                  __m128d * gmx_restrict x3, __m128d * gmx_restrict y3, __m128d * gmx_restrict z3,
                                  __m128d * gmx_restrict x4, __m128d * gmx_restrict y4, __m128d * gmx_restrict z4)
{
    __m128d t1,t2,t3,t4,t5,t6;
    t1           = _mm_loadu_pd(ptrA);
    t2           = _mm_loadu_pd(ptrB);
    t3           = _mm_loadu_pd(ptrA+2);
    t4           = _mm_loadu_pd(ptrB+2);
    t5           = _mm_loadu_pd(ptrA+4);
    t6           = _mm_loadu_pd(ptrB+4);
    GMX_MM_TRANSPOSE2_PD(t1,t2);
    GMX_MM_TRANSPOSE2_PD(t3,t4);
    GMX_MM_TRANSPOSE2_PD(t5,t6);
    *x1          = t1;
    *y1          = t2;
    *z1          = t3;
    *x2          = t4;
    *y2          = t5;
    *z2          = t6;
    t1           = _mm_loadu_pd(ptrA+6);
    t2           = _mm_loadu_pd(ptrB+6);
    t3           = _mm_loadu_pd(ptrA+8);
    t4           = _mm_loadu_pd(ptrB+8);
    t5           = _mm_loadu_pd(ptrA+10);
    t6           = _mm_loadu_pd(ptrB+10);
    GMX_MM_TRANSPOSE2_PD(t1,t2);
    GMX_MM_TRANSPOSE2_PD(t3,t4);
    GMX_MM_TRANSPOSE2_PD(t5,t6);
    *x3          = t1;
    *y3          = t2;
    *z3          = t3;
    *x4          = t4;
    *y4          = t5;
    *z4          = t6;
}


/* Routines to decrement rvec in memory, typically use for j particle force updates */
static gmx_inline void
gmx_mm_decrement_1rvec_1ptr_noswizzle_pd(double * gmx_restrict ptrA,
                                         __m128d xy, __m128d z)
{
    __m128d t1,t2;

    t1 = _mm_loadu_pd(ptrA);
    t2 = _mm_load_sd(ptrA+2);

    t1 = _mm_sub_pd(t1,xy);
    t2 = _mm_sub_sd(t2,z);

    _mm_storeu_pd(ptrA,t1);
    _mm_store_sd(ptrA+2,t2);
}

static gmx_inline void
gmx_mm_decrement_3rvec_1ptr_noswizzle_pd(double * gmx_restrict ptrA,
                                         __m128d xy1, __m128d z1,
                                         __m128d xy2, __m128d z2,
                                         __m128d xy3, __m128d z3)
{
    __m128d t1,t2;
    __m128d tA,tB,tC,tD,tE;

    tA   = _mm_loadu_pd(ptrA);
    tB   = _mm_loadu_pd(ptrA+2);
    tC   = _mm_loadu_pd(ptrA+4);
    tD   = _mm_loadu_pd(ptrA+6);
    tE   = _mm_load_sd(ptrA+8);

    /* xy1: y1 x1 */
    t1   = _mm_shuffle_pd(z1,xy2,_MM_SHUFFLE2(0,1)); /* x2 z1 */
    t2   = _mm_shuffle_pd(xy2,z2,_MM_SHUFFLE2(0,1)); /* z2 y2 */
    /* xy3: y3 x3 */

    tA   = _mm_sub_pd(tA,xy1);
    tB   = _mm_sub_pd(tB,t1);
    tC   = _mm_sub_pd(tC,t2);
    tD   = _mm_sub_pd(tD,xy3);
    tE   = _mm_sub_sd(tE,z3);

    _mm_storeu_pd(ptrA,tA);
    _mm_storeu_pd(ptrA+2,tB);
    _mm_storeu_pd(ptrA+4,tC);
    _mm_storeu_pd(ptrA+6,tD);
    _mm_store_sd(ptrA+8,tE);
}

static gmx_inline void
gmx_mm_decrement_4rvec_1ptr_noswizzle_pd(double * gmx_restrict ptrA,
                                         __m128d xy1, __m128d z1,
                                         __m128d xy2, __m128d z2,
                                         __m128d xy3, __m128d z3,
                                         __m128d xy4, __m128d z4)
{
    __m128d t1,t2,t3,t4;
    __m128d tA,tB,tC,tD,tE,tF;

    tA   = _mm_loadu_pd(ptrA);
    tB   = _mm_loadu_pd(ptrA+2);
    tC   = _mm_loadu_pd(ptrA+4);
    tD   = _mm_loadu_pd(ptrA+6);
    tE   = _mm_loadu_pd(ptrA+8);
    tF   = _mm_loadu_pd(ptrA+10);

    /* xy1: y1 x1 */
    t1   = _mm_shuffle_pd(z1,xy2,_MM_SHUFFLE2(0,0)); /* x2 z1 */
    t2   = _mm_shuffle_pd(xy2,z2,_MM_SHUFFLE2(0,1)); /* z2 y2 */
    /* xy3: y3 x3 */
    t3   = _mm_shuffle_pd(z3,xy4,_MM_SHUFFLE2(0,0)); /* x4 z3 */
    t4   = _mm_shuffle_pd(xy4,z4,_MM_SHUFFLE2(0,1)); /* z4 y4 */

    tA   = _mm_sub_pd(tA,xy1);
    tB   = _mm_sub_pd(tB,t1);
    tC   = _mm_sub_pd(tC,t2);
    tD   = _mm_sub_pd(tD,xy3);
    tE   = _mm_sub_pd(tE,t3);
    tF   = _mm_sub_pd(tF,t4);

    _mm_storeu_pd(ptrA,tA);
    _mm_storeu_pd(ptrA+2,tB);
    _mm_storeu_pd(ptrA+4,tC);
    _mm_storeu_pd(ptrA+6,tD);
    _mm_storeu_pd(ptrA+8,tE);
    _mm_storeu_pd(ptrA+10,tF);
}

static gmx_inline void
gmx_mm_decrement_1rvec_1ptr_swizzle_pd(double * gmx_restrict ptrA,
                                       __m128d x1, __m128d y1, __m128d z1)
{
    __m128d t1,t2,t3;

    t1           = _mm_load_sd(ptrA);
    t2           = _mm_load_sd(ptrA+1);
    t3           = _mm_load_sd(ptrA+2);

    t1           = _mm_sub_sd(t1,x1);
    t2           = _mm_sub_sd(t2,y1);
    t3           = _mm_sub_sd(t3,z1);
    _mm_store_sd(ptrA,t1);
    _mm_store_sd(ptrA+1,t2);
    _mm_store_sd(ptrA+2,t3);
}


static gmx_inline void
gmx_mm_decrement_3rvec_1ptr_swizzle_pd(double * gmx_restrict ptrA,
                                       __m128d x1, __m128d y1, __m128d z1,
                                       __m128d x2, __m128d y2, __m128d z2,
                                       __m128d x3, __m128d y3, __m128d z3)
{
    __m128d t1,t2,t3,t4,t5;

    t1          = _mm_loadu_pd(ptrA);
    t2          = _mm_loadu_pd(ptrA+2);
    t3          = _mm_loadu_pd(ptrA+4);
    t4          = _mm_loadu_pd(ptrA+6);
    t5          = _mm_load_sd(ptrA+8);

    x1          = _mm_unpacklo_pd(x1,y1);
    z1          = _mm_unpacklo_pd(z1,x2);
    y2          = _mm_unpacklo_pd(y2,z2);
    x3          = _mm_unpacklo_pd(x3,y3);
    /* nothing to be done for z3 */

    t1          = _mm_sub_pd(t1,x1);
    t2          = _mm_sub_pd(t2,z1);
    t3          = _mm_sub_pd(t3,y2);
    t4          = _mm_sub_pd(t4,x3);
    t5          = _mm_sub_sd(t5,z3);
    _mm_storeu_pd(ptrA,t1);
    _mm_storeu_pd(ptrA+2,t2);
    _mm_storeu_pd(ptrA+4,t3);
    _mm_storeu_pd(ptrA+6,t4);
    _mm_store_sd(ptrA+8,t5);
}


static gmx_inline void
gmx_mm_decrement_4rvec_1ptr_swizzle_pd(double * gmx_restrict ptrA,
                                       __m128d x1, __m128d y1, __m128d z1,
                                       __m128d x2, __m128d y2, __m128d z2,
                                       __m128d x3, __m128d y3, __m128d z3,
                                       __m128d x4, __m128d y4, __m128d z4)
{
    __m128d t1,t2,t3,t4,t5,t6;

    t1          = _mm_loadu_pd(ptrA);
    t2          = _mm_loadu_pd(ptrA+2);
    t3          = _mm_loadu_pd(ptrA+4);
    t4          = _mm_loadu_pd(ptrA+6);
    t5          = _mm_loadu_pd(ptrA+8);
    t6          = _mm_loadu_pd(ptrA+10);

    x1          = _mm_unpacklo_pd(x1,y1);
    z1          = _mm_unpacklo_pd(z1,x2);
    y2          = _mm_unpacklo_pd(y2,z2);
    x3          = _mm_unpacklo_pd(x3,y3);
    z3          = _mm_unpacklo_pd(z3,x4);
    y4          = _mm_unpacklo_pd(y4,z4);

    _mm_storeu_pd(ptrA,    _mm_sub_pd( t1,x1 ));
    _mm_storeu_pd(ptrA+2,  _mm_sub_pd( t2,z1 ));
    _mm_storeu_pd(ptrA+4,  _mm_sub_pd( t3,y2 ));
    _mm_storeu_pd(ptrA+6,  _mm_sub_pd( t4,x3 ));
    _mm_storeu_pd(ptrA+8,  _mm_sub_pd( t5,z3 ));
    _mm_storeu_pd(ptrA+10, _mm_sub_pd( t6,y4 ));
}

static gmx_inline void
gmx_mm_decrement_1rvec_2ptr_swizzle_pd(double * gmx_restrict ptrA, double * gmx_restrict ptrB,
                                       __m128d x1, __m128d y1, __m128d z1)
{
    __m128d t1,t2,t3,t4,t5,t6,t7;

    t1          = _mm_loadu_pd(ptrA);
    t2          = _mm_load_sd(ptrA+2);
    t3          = _mm_loadu_pd(ptrB);
    t4          = _mm_load_sd(ptrB+2);

    t5          = _mm_unpacklo_pd(x1,y1);
    t6          = _mm_unpackhi_pd(x1,y1);
    t7          = _mm_unpackhi_pd(z1,z1);

    t1          = _mm_sub_pd(t1,t5);
    t2          = _mm_sub_sd(t2,z1);

    t3          = _mm_sub_pd(t3,t6);
    t4          = _mm_sub_sd(t4,t7);

    _mm_storeu_pd(ptrA,t1);
    _mm_store_sd(ptrA+2,t2);
    _mm_storeu_pd(ptrB,t3);
    _mm_store_sd(ptrB+2,t4);
}

static gmx_inline void
gmx_mm_decrement_3rvec_2ptr_swizzle_pd(double * gmx_restrict ptrA, double * gmx_restrict ptrB,
                                       __m128d x1, __m128d y1, __m128d z1,
                                       __m128d x2, __m128d y2, __m128d z2,
                                       __m128d x3, __m128d y3, __m128d z3)
{
    __m128d t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
    __m128d tA,tB,tC,tD,tE,tF,tG,tH,tI;

    t1          = _mm_loadu_pd(ptrA);
    t2          = _mm_loadu_pd(ptrA+2);
    t3          = _mm_loadu_pd(ptrA+4);
    t4          = _mm_loadu_pd(ptrA+6);
    t5          = _mm_load_sd(ptrA+8);
    t6          = _mm_loadu_pd(ptrB);
    t7          = _mm_loadu_pd(ptrB+2);
    t8          = _mm_loadu_pd(ptrB+4);
    t9          = _mm_loadu_pd(ptrB+6);
    t10         = _mm_load_sd(ptrB+8);

    tA          = _mm_unpacklo_pd(x1,y1);
    tB          = _mm_unpackhi_pd(x1,y1);
    tC          = _mm_unpacklo_pd(z1,x2);
    tD          = _mm_unpackhi_pd(z1,x2);
    tE          = _mm_unpacklo_pd(y2,z2);
    tF          = _mm_unpackhi_pd(y2,z2);
    tG          = _mm_unpacklo_pd(x3,y3);
    tH          = _mm_unpackhi_pd(x3,y3);
    tI          = _mm_unpackhi_pd(z3,z3);

    t1          = _mm_sub_pd(t1,tA);
    t2          = _mm_sub_pd(t2,tC);
    t3          = _mm_sub_pd(t3,tE);
    t4          = _mm_sub_pd(t4,tG);
    t5          = _mm_sub_sd(t5,z3);

    t6          = _mm_sub_pd(t6,tB);
    t7          = _mm_sub_pd(t7,tD);
    t8          = _mm_sub_pd(t8,tF);
    t9          = _mm_sub_pd(t9,tH);
    t10         = _mm_sub_sd(t10,tI);

    _mm_storeu_pd(ptrA,t1);
    _mm_storeu_pd(ptrA+2,t2);
    _mm_storeu_pd(ptrA+4,t3);
    _mm_storeu_pd(ptrA+6,t4);
    _mm_store_sd(ptrA+8,t5);
    _mm_storeu_pd(ptrB,t6);
    _mm_storeu_pd(ptrB+2,t7);
    _mm_storeu_pd(ptrB+4,t8);
    _mm_storeu_pd(ptrB+6,t9);
    _mm_store_sd(ptrB+8,t10);
}


static gmx_inline void
gmx_mm_decrement_4rvec_2ptr_swizzle_pd(double * gmx_restrict ptrA, double * gmx_restrict ptrB,
                                       __m128d x1, __m128d y1, __m128d z1,
                                       __m128d x2, __m128d y2, __m128d z2,
                                       __m128d x3, __m128d y3, __m128d z3,
                                       __m128d x4, __m128d y4, __m128d z4)
{
    __m128d t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12;
    __m128d tA,tB,tC,tD,tE,tF,tG,tH,tI,tJ,tK,tL;

    t1          = _mm_loadu_pd(ptrA);
    t2          = _mm_loadu_pd(ptrA+2);
    t3          = _mm_loadu_pd(ptrA+4);
    t4          = _mm_loadu_pd(ptrA+6);
    t5          = _mm_loadu_pd(ptrA+8);
    t6          = _mm_loadu_pd(ptrA+10);
    t7          = _mm_loadu_pd(ptrB);
    t8          = _mm_loadu_pd(ptrB+2);
    t9          = _mm_loadu_pd(ptrB+4);
    t10         = _mm_loadu_pd(ptrB+6);
    t11         = _mm_loadu_pd(ptrB+8);
    t12         = _mm_loadu_pd(ptrB+10);

    tA          = _mm_unpacklo_pd(x1,y1);
    tB          = _mm_unpackhi_pd(x1,y1);
    tC          = _mm_unpacklo_pd(z1,x2);
    tD          = _mm_unpackhi_pd(z1,x2);
    tE          = _mm_unpacklo_pd(y2,z2);
    tF          = _mm_unpackhi_pd(y2,z2);
    tG          = _mm_unpacklo_pd(x3,y3);
    tH          = _mm_unpackhi_pd(x3,y3);
    tI          = _mm_unpacklo_pd(z3,x4);
    tJ          = _mm_unpackhi_pd(z3,x4);
    tK          = _mm_unpacklo_pd(y4,z4);
    tL          = _mm_unpackhi_pd(y4,z4);

    t1          = _mm_sub_pd(t1,tA);
    t2          = _mm_sub_pd(t2,tC);
    t3          = _mm_sub_pd(t3,tE);
    t4          = _mm_sub_pd(t4,tG);
    t5          = _mm_sub_pd(t5,tI);
    t6          = _mm_sub_pd(t6,tK);

    t7          = _mm_sub_pd(t7,tB);
    t8          = _mm_sub_pd(t8,tD);
    t9          = _mm_sub_pd(t9,tF);
    t10         = _mm_sub_pd(t10,tH);
    t11         = _mm_sub_pd(t11,tJ);
    t12         = _mm_sub_pd(t12,tL);

    _mm_storeu_pd(ptrA,  t1);
    _mm_storeu_pd(ptrA+2,t2);
    _mm_storeu_pd(ptrA+4,t3);
    _mm_storeu_pd(ptrA+6,t4);
    _mm_storeu_pd(ptrA+8,t5);
    _mm_storeu_pd(ptrA+10,t6);
    _mm_storeu_pd(ptrB,  t7);
    _mm_storeu_pd(ptrB+2,t8);
    _mm_storeu_pd(ptrB+4,t9);
    _mm_storeu_pd(ptrB+6,t10);
    _mm_storeu_pd(ptrB+8,t11);
    _mm_storeu_pd(ptrB+10,t12);
}





static gmx_inline void
gmx_mm_update_iforce_1atom_swizzle_pd(__m128d fix1, __m128d fiy1, __m128d fiz1,
                                      double * gmx_restrict fptr,
                                      double * gmx_restrict fshiftptr)
{
    __m128d t1,t2,t3;

    /* transpose data */
    t1 = fix1;
    fix1 = _mm_unpacklo_pd(fix1,fiy1); /* y0 x0 */
    fiy1 = _mm_unpackhi_pd(t1,fiy1);   /* y1 x1 */

    fix1 = _mm_add_pd(fix1,fiy1);
    fiz1 = _mm_add_sd( fiz1, _mm_unpackhi_pd(fiz1,fiz1 ));

    _mm_storeu_pd( fptr, _mm_add_pd( _mm_loadu_pd(fptr), fix1 ));
    _mm_store_sd( fptr+2, _mm_add_sd( _mm_load_sd(fptr+2), fiz1 ));

    _mm_storeu_pd( fshiftptr, _mm_add_pd( _mm_loadu_pd(fshiftptr), fix1 ));
    _mm_store_sd( fshiftptr+2, _mm_add_sd( _mm_load_sd(fshiftptr+2), fiz1 ));
}

static gmx_inline void
gmx_mm_update_iforce_3atom_swizzle_pd(__m128d fix1, __m128d fiy1, __m128d fiz1,
                                      __m128d fix2, __m128d fiy2, __m128d fiz2,
                                      __m128d fix3, __m128d fiy3, __m128d fiz3,
                                      double * gmx_restrict fptr,
                                      double * gmx_restrict fshiftptr)
{
    __m128d t1,t2;

    /* transpose data */
    GMX_MM_TRANSPOSE2_PD(fix1,fiy1);
    GMX_MM_TRANSPOSE2_PD(fiz1,fix2);
    GMX_MM_TRANSPOSE2_PD(fiy2,fiz2);
    t1 = fix3;
    fix3 = _mm_unpacklo_pd(fix3,fiy3); /* y0 x0 */
    fiy3 = _mm_unpackhi_pd(t1,fiy3);   /* y1 x1 */

    fix1 = _mm_add_pd(fix1,fiy1);
    fiz1 = _mm_add_pd(fiz1,fix2);
    fiy2 = _mm_add_pd(fiy2,fiz2);

    fix3 = _mm_add_pd(fix3,fiy3);
    fiz3 = _mm_add_sd( fiz3, _mm_unpackhi_pd(fiz3,fiz3));

    _mm_storeu_pd( fptr, _mm_add_pd( _mm_loadu_pd(fptr), fix1 ));
    _mm_storeu_pd( fptr+2, _mm_add_pd( _mm_loadu_pd(fptr+2), fiz1 ));
    _mm_storeu_pd( fptr+4, _mm_add_pd( _mm_loadu_pd(fptr+4), fiy2 ));
    _mm_storeu_pd( fptr+6, _mm_add_pd( _mm_loadu_pd(fptr+6), fix3 ));
    _mm_store_sd( fptr+8, _mm_add_sd( _mm_load_sd(fptr+8), fiz3 ));

    fix1 = _mm_add_pd(fix1,fix3);
    t1   = _mm_shuffle_pd(fiz1,fiy2,_MM_SHUFFLE2(0,1));
    fix1 = _mm_add_pd(fix1,t1); /* x and y sums */

    t2   = _mm_shuffle_pd(fiy2,fiy2,_MM_SHUFFLE2(1,1));
    fiz1 = _mm_add_sd(fiz1,fiz3);
    fiz1 = _mm_add_sd(fiz1,t2); /* z sum */

    _mm_storeu_pd( fshiftptr, _mm_add_pd( _mm_loadu_pd(fshiftptr), fix1 ));
    _mm_store_sd( fshiftptr+2, _mm_add_sd( _mm_load_sd(fshiftptr+2), fiz1 ));
}


static gmx_inline void
gmx_mm_update_iforce_4atom_swizzle_pd(__m128d fix1, __m128d fiy1, __m128d fiz1,
                                      __m128d fix2, __m128d fiy2, __m128d fiz2,
                                      __m128d fix3, __m128d fiy3, __m128d fiz3,
                                      __m128d fix4, __m128d fiy4, __m128d fiz4,
                                      double * gmx_restrict fptr,
                                      double * gmx_restrict fshiftptr)
{
    __m128d t1,t2;

    /* transpose data */
    GMX_MM_TRANSPOSE2_PD(fix1,fiy1);
    GMX_MM_TRANSPOSE2_PD(fiz1,fix2);
    GMX_MM_TRANSPOSE2_PD(fiy2,fiz2);
    GMX_MM_TRANSPOSE2_PD(fix3,fiy3);
    GMX_MM_TRANSPOSE2_PD(fiz3,fix4);
    GMX_MM_TRANSPOSE2_PD(fiy4,fiz4);

    fix1 = _mm_add_pd(fix1,fiy1);
    fiz1 = _mm_add_pd(fiz1,fix2);
    fiy2 = _mm_add_pd(fiy2,fiz2);
    fix3 = _mm_add_pd(fix3,fiy3);
    fiz3 = _mm_add_pd(fiz3,fix4);
    fiy4 = _mm_add_pd(fiy4,fiz4);
    
    _mm_storeu_pd( fptr, _mm_add_pd( _mm_loadu_pd(fptr),       fix1 ));
    _mm_storeu_pd( fptr+2, _mm_add_pd( _mm_loadu_pd(fptr+2),   fiz1 ));
    _mm_storeu_pd( fptr+4, _mm_add_pd( _mm_loadu_pd(fptr+4),   fiy2 ));
    _mm_storeu_pd( fptr+6, _mm_add_pd( _mm_loadu_pd(fptr+6),   fix3 ));
    _mm_storeu_pd( fptr+8, _mm_add_pd( _mm_loadu_pd(fptr+8),   fiz3 ));
    _mm_storeu_pd( fptr+10, _mm_add_pd( _mm_loadu_pd(fptr+10), fiy4 ));

    t1 = _mm_shuffle_pd(fiz1,fiy2,_MM_SHUFFLE2(0,1));
    fix1 = _mm_add_pd(fix1,t1);
    t2 = _mm_shuffle_pd(fiz3,fiy4,_MM_SHUFFLE2(0,1));
    fix3 = _mm_add_pd(fix3,t2);
    fix1 = _mm_add_pd(fix1,fix3); /* x and y sums */

    fiz1 = _mm_add_sd(fiz1, _mm_unpackhi_pd(fiy2,fiy2));
    fiz3 = _mm_add_sd(fiz3, _mm_unpackhi_pd(fiy4,fiy4));
    fiz1 = _mm_add_sd(fiz1,fiz3); /* z sum */

    _mm_storeu_pd( fshiftptr, _mm_add_pd( _mm_loadu_pd(fshiftptr), fix1 ));
    _mm_store_sd( fshiftptr+2, _mm_add_sd( _mm_load_sd(fshiftptr+2), fiz1 ));
}



static gmx_inline void
gmx_mm_update_1pot_pd(__m128d pot1, double * gmx_restrict ptrA)
{
    pot1 = _mm_add_pd(pot1, _mm_unpackhi_pd(pot1,pot1));
    _mm_store_sd(ptrA,_mm_add_sd(pot1,_mm_load_sd(ptrA)));
}

static gmx_inline void
gmx_mm_update_2pot_pd(__m128d pot1, double * gmx_restrict ptrA,
                      __m128d pot2, double * gmx_restrict ptrB)
{
    GMX_MM_TRANSPOSE2_PD(pot1,pot2);
    pot1 = _mm_add_pd(pot1,pot2);
    pot2 = _mm_unpackhi_pd(pot1,pot1);

    _mm_store_sd(ptrA,_mm_add_sd(pot1,_mm_load_sd(ptrA)));
    _mm_store_sd(ptrB,_mm_add_sd(pot2,_mm_load_sd(ptrB)));
}


#endif /* _kernelutil_x86_sse2_double_h_ */
