/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2012, by the GROMACS development team, led by
 * David van der Spoel, Berk Hess, Erik Lindahl, and including many
 * others, as listed in the AUTHORS file in the top-level source
 * directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
#include <ctype.h>
#include <math.h>

#include <types/simple.h>
#include "gmx_blas.h"

void
F77_FUNC(strsm,STRSM)(const char * side,
                      const char * uplo,
                      const char * transa,
                      const char * diag,
                      int *  m__,
                      int *  n__,
                      float *alpha__,
                      float *a,
                      int *  lda__,
                      float *b,
                      int *  ldb__)
{
  const char xside  = toupper(*side);
  const char xuplo  = toupper(*uplo);
  const char xtrans = toupper(*transa);
  const char xdiag  = toupper(*diag);
  int i,j,k;
  float temp;

  int m = *m__;
  int n = *n__;
  int lda = *lda__;
  int ldb = *ldb__;
  float alpha = *alpha__;
  
  if(n<=0)
    return;

  
  if(fabs(alpha)<GMX_FLOAT_MIN) { 
    for(j=0;j<n;j++)
      for(i=0;i<m;i++)
	b[j*(ldb)+i] = 0.0;
    return;
  }

  if(xside=='L') {
    /* left side */
    if(xtrans=='N') {
      /* No transpose */
      if(xuplo=='U') {
	/* upper */
	for(j=0;j<n;j++) {
	  if(fabs(alpha-1.0)>GMX_FLOAT_EPS) {
	    for(i=0;i<m;i++)
	      b[j*(ldb)+i] *= alpha;
	  }
	  for(k=m-1;k>=0;k--) {
	    if( fabs(b[j*(ldb)+k])>GMX_FLOAT_MIN) {
	      if(xdiag=='N')
		b[j*(ldb)+k] /= a[k*(lda)+k];
	      for(i=0;i<k;i++)
		b[j*(ldb)+i] -= b[j*(ldb)+k]*a[k*(lda)+i];
	    }
	  }
	}
      } else {
	/* lower */
	for(j=0;j<n;j++) {
	  if(fabs(alpha-1.0)>GMX_FLOAT_EPS)
	    for(i=0;i<m;i++)
	      b[j*(ldb)+i] *= alpha;
	  for(k=0;k<m;k++) {
	    if( fabs(b[j*(ldb)+k])>GMX_FLOAT_MIN) {
	      if(xdiag=='N')
		b[j*(ldb)+k] /= a[k*(lda)+k];
	      for(i=k+1;i<m;i++)
		b[j*(ldb)+i] -= b[j*(ldb)+k]*a[k*(lda)+i];
	    }
	  }
	}
      }
    } else {
      /* Transpose */
      if(xuplo=='U') {
	/* upper */
	for(j=0;j<n;j++) {
	  for(i=0;i<m;i++) {
	    temp = alpha * b[j*(ldb)+i];
	    for(k=0;k<i;k++)
	      temp -= a[i*(lda)+k] * b[j*(ldb)+k];
	    if(xdiag=='N')
		temp /= a[i*(lda)+i];
	    b[j*(ldb)+i] = temp;
	  }
	}
      } else {
	/* lower */
	for(j=0;j<n;j++) {
	  for(i=m-1;i>=0;i--) {
	    temp = alpha * b[j*(ldb)+i];
	    for(k=i+1;k<m;k++)
	      temp -= a[i*(lda)+k] * b[j*(ldb)+k];
	    if(xdiag=='N')
		temp /= a[i*(lda)+i];
	    b[j*(ldb)+i] = temp;
	  }
	}
      }
    }
  } else {
    /* right side */
    if(xtrans=='N') {
      /* No transpose */
      if(xuplo=='U') {
	/* upper */
	for(j=0;j<n;j++) {
	  if(fabs(alpha-1.0)>GMX_FLOAT_EPS)
	    for(i=0;i<m;i++)
	      b[j*(ldb)+i] *= alpha;
	  for(k=0;k<j;k++) {
	    if( fabs(a[j*(lda)+k])>GMX_FLOAT_MIN) {
	      for(i=0;i<m;i++)
		b[j*(ldb)+i] -= a[j*(lda)+k]*b[k*(ldb)+i];
	    }
	  }
	  if(xdiag=='N') {
	    temp = 1.0/a[j*(lda)+j];
	    for(i=0;i<m;i++)
	      b[j*(ldb)+i] *= temp;
	  }
	}
      } else {
	/* lower */
	for(j=n-1;j>=0;j--) {
	  if(fabs(alpha-1.0)>GMX_FLOAT_EPS)
	    for(i=0;i<m;i++)
	      b[j*(ldb)+i] *= alpha;
	  for(k=j+1;k<n;k++) {
	    if( fabs(a[j*(lda)+k])>GMX_FLOAT_MIN ) {
	      for(i=0;i<m;i++)
		b[j*(ldb)+i] -= a[j*(lda)+k]*b[k*(ldb)+i];
	    }
	  }
	  if(xdiag=='N') {
	    temp = 1.0/a[j*(lda)+j];
	    for(i=0;i<m;i++)
	      b[j*(ldb)+i] *= temp;
	  }
	}
      }
    } else {
      /* Transpose */
      if(xuplo=='U') {
	/* upper */
	for(k=n-1;k>=0;k--) {
	  if(xdiag=='N') {
	    temp = 1.0/a[k*(lda)+k];
	    for(i=0;i<m;i++)
	      b[k*(ldb)+i] *= temp;
	  }
	  for(j=0;j<k;j++) {
	    if( fabs(a[k*(lda)+j])>GMX_FLOAT_MIN) {
	      temp = a[k*(lda)+j];
	      for(i=0;i<m;i++)
		b[j*(ldb)+i] -= temp * b[k*(ldb)+i];
	    }
	  }
	  if(fabs(alpha-1.0)>GMX_FLOAT_EPS)
	    for(i=0;i<m;i++)
	      b[k*(ldb)+i] *= alpha;
	}
      } else {
	/* lower */
	for(k=0;k<n;k++) {
	  if(xdiag=='N') {
	    temp = 1.0/a[k*(lda)+k];
	    for(i=0;i<m;i++)
	      b[k*(ldb)+i] *= temp;
	  }
	  for(j=k+1;j<n;j++) {
	    if( fabs(a[k*(lda)+j])>GMX_FLOAT_MIN) {
	      temp = a[k*(lda)+j];
	      for(i=0;i<m;i++)
		b[j*(ldb)+i] -= temp * b[k*(ldb)+i];
	    }
	  }
	  if(fabs(alpha-1.0)>GMX_FLOAT_EPS)
	    for(i=0;i<m;i++)
	      b[k*(ldb)+i] *= alpha;
	}
      }      
    }
  }    
}
