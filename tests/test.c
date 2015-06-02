#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define PI 3.14159265358979323846
#define N 8.
#define COS(x, lambda) (cos(((2.*x + 1.)/2.*N)*lambda*PI))

double C(int32_t xi)
{
   if (!xi){
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

	 printf ("%f ", sum);

	 sum *= 1/4;
	 sum += 128.;

	 if (sum < 0) sum = 0;
	 if (sum > 255) sum = 255;

	 out[8 * i + j] = sum;
      }
   }
   printf ("\n");
}

int main (void)
{

   /* http://en.wikipedia.org/wiki/JPEG#Discrete_cosine_transform */
   int32_t g[] = {
      -416, -33, -60, 32, 48, -40, 0, 0,
      0, -24, -56, 19, 26, 0, 0, 0,
      -42, 13, 80, -24, -40, 0, 0, 0,
      -42, 17, 44, -29, 0, 0, 0, 0,
      18, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0
   };

   for (uint8_t i = 0; i < 8; i++) {
      for (uint8_t j = 0; j < 8; j++)
	 printf ("%d ", g[i*8 + j]);

      printf ("\n");
   }
   printf ("\n");

   uint8_t G[64];

   idct_block (g, G);

   printf ("\n");
   for (uint8_t i = 0; i < 8; i++) {
      for (uint8_t j = 0; j < 8; j++)
	 printf ("%d ", G[i*8 + j]);

      printf ("\n");
   }

   return 0;
}
