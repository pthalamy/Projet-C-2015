#include "idct.h"
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846
#define N 8.
#define COS(x, lambda) (cos(((2. * x + 1.) * lambda * acos(-1) / 16)))

double C(int32_t xi)
{
   if ( xi == 0 ){
      return 1./sqrt(2.);
   } else {
      return 1.;
   }
}

void idct_block(int32_t in[64], uint8_t out[64])
{
   for (uint32_t i = 0; i < 8; i++) {
      for (uint32_t j = 0; j < 8; j++) {
	 double sum = 0;
	 for (uint32_t k = 0; k < 8; k++) {
	    for (uint32_t l = 0; l < 8; l++) {
	       sum += C(k) * C(l) * COS(i, k) * COS(j, l) * in[8*k + l];
	    }
	 }

	 sum *= 0.25;
	 sum += 128.;

	 if (sum < 0) sum = 0;
	 if (sum > 255) sum = 255;

	 out[8 * i + j] = sum;
      }
   }
}
