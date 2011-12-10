/***********************************************

Author contact information: 
    Michael J. Black
    Department of Computer Science
    Brown University 
    P.O. Box 1910
    Providence, RI 02912 

    http://www.cs.brown.edu/people/black/
    email: black@cs.brown.edu

    401-863-7637 (voice) 
    401-863-7657 (fax) 

---------------------------------------------------------------------
Copyright 1993, 2002 Michael J. Black

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any non-commercial purpose is hereby granted without
fee, provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of the author not be used
in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

For commercial use and licensing information contact the author
at the address above
---------------------------------------------------------------------

Details: Geman/McClure robust error norm, psi function, etc.


************************************************/

#include <stdio.h>
#include <Tools/MathUtils.h>
#include <string.h>

#include "robust.h"

#ifndef ROOT3
#define ROOT3 1.7320508
#endif

/* Geman/McClure robust error norm
*/
float rho(float x, float sigma)
{
  double result, tmp;

  tmp =  (x * x)/sigma;
  result = tmp / (1.0 + tmp);

  return (float) result;
}

float psi(float x, float sigma)
{
  double result, tmp;

  tmp =  (sigma * sigma);
  result = (2.0 * x * tmp)/pow((tmp+(x*x)), 2.0); //old->(float)2.0

  return (float)result;
}


/*
dpsi:
                                   2     2      2
                                  s  (- s  + 3 x )
                              - 2 ----------------
                                       2    2 3
                                     (s  + x )
*/
float dpsi(float x, float sigma)
{
  double result, tmp1, tmp2;

  tmp1 =  (sigma * sigma);
  tmp2 =  (x * x);
  result = (-2.0*tmp1*((3.0*tmp2) - tmp1))/pow((tmp1+tmp2), 3);

  return (float)result;
}

/*
Max second derive of rho occurs at 0
*/
float dpsi_max(float sigma)
{
  double result;

  result = 2.0 / (sigma*sigma);

  return (float)result;
}
  

float outlier_to_sigma(float out)
{
  double sigma;

  sigma = ROOT3 * out;

  return (float)sigma;
}

  
float sigma_to_outlier(float sigma)
{
  double out;

  out = sigma/ROOT3;

  return (float)out;
}

