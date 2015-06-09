#include "idct.h"

#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

#ifdef LOEFFLER

void butterfly(float *i0, float *i1)
{
   float tmp1 = *i0 + *i1;
   float tmp2 = *i0 - *i1;
   *i0 = tmp1;
   *i1 = tmp2;
}

void rotator(float *i0, float *i1, float k, float n)
{
   float tmp1 = *i0 * k * cos((n*PI)/16) - *i1 * k * sin((n*PI)/16);
   float tmp2 = *i0 * k * sin((n*PI)/16) + *i1 * k * cos((n*PI)/16);
   *i0 = tmp1;
   *i1 = tmp2;
}

void loeffler (float  in[8], float out[8])
{
   out[0] = in[0];
   out[7] = in[1];
   out[2] = in[2];
   out[5] = in[3];
   out[1] = in[4];
   out[6] = in[5];
   out[3] = in[6];
   out[4] = in[7];
   out[7] = out[7] / sqrt(2);
   out[4] = out[4] / sqrt(2);
   /*etape 1*/
   butterfly ( &out[0], &out[1] );
   rotator ( &out[2], &out[3], sqrt(2), 6 );
   /*   mult ( &out[7], sqrt(1/2) );
        mult ( &out[4], sqrt(1/2) );*/
   butterfly ( &out[7], &out[4] );
   /*etape 2*/
   butterfly ( &out[0], &out[3] );
   butterfly ( &out[4], &out[6] );
   butterfly ( &out[7], &out[5] );
   butterfly ( &out[1], &out[2] );
   /*etape 3*/
   rotator ( &out[4], &out[7], sqrt(2), 3 );
   rotator ( &out[5], &out[6], sqrt(2), 1 );
   /*etape 4*/
   butterfly ( &out[0], &out[7] );
   butterfly ( &out[1], &out[6] );
   butterfly ( &out[2], &out[5] );
   butterfly ( &out[3], &out[4] );
}

/* LOEFFLER IDCT */
void idct_block(int32_t in[64], uint8_t out[64])
{
   printf ("idct loeffler\n");
   float matrix[64];
   float tmp_vector[8];
   float tmp[8];
   for ( uint32_t i = 0; i < 8; i++ ) {
      for ( uint32_t j = 0; j < 8; j++ ) {
         tmp[j] = in[8*i+j];
      }
      loeffler( tmp, tmp_vector);
      for ( uint32_t j = 0; j < 8; j++ ){
         matrix[8*i+j] = tmp_vector[j];
      }
   }
   for ( uint32_t i = 0; i < 8; i++ ) {
      for ( uint32_t j = 0; j < 8; j++ ) {
         tmp[j] = matrix[8*j+i];
      }
      loeffler( tmp, tmp_vector );
      for ( uint32_t j = 0; j < 8; j++ ){
         /*Saturation*/
         if ( floor( tmp_vector[j]/8.0 ) + 128.0 > 255.0 )
            out[i+8*j] = 255;
         else if ( floor( tmp_vector[j]/8.0 ) + 128.0 < 0.0 )
            out[i+8*j] = 0;
         else
            out[i+8*j] = (uint8_t) ( floor( tmp_vector[j]/8.0 ) + 128.0 );
      }
   }
}

#else

#define COS(x, lambda) (cos(((2. * x + 1.) * lambda * acos(-1) / 16)))

double C(int32_t xi)
{
   if ( xi == 0 ){
      return 1./sqrt(2.);
   } else {
      return 1.;
   }
}

/* BASIC IDCT */
void idct_block(int32_t in[64], uint8_t out[64])
{
   printf ("idct basique\n");

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

#endif
