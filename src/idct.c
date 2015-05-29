#include "idct.h"
#include <stdio.h>
#define PI 3.14159265358979323846
#include <math.h>

uint32_t C(uint32_t xi)
{
   if ( xi == 0 ){
      return 1/sqrt(2);
   } else {
      return 1;
   }
}



void idct_block(int32_t in[64], uint8_t out[64])
{
   float res_32=0;
   float exp=0.;

   for ( uint32_t i=0; i < 64; i++)
   {

      for ( uint32_t j=0; j < 8; j++)
      {
         for ( uint32_t k=0; k < 8; k++)
         {
            exp= exp + C(j)*C(k)*cos((2.*((float)i/8.)+1.)*(float)j*PI/16.)*cos((2.*(float)(i%8)+1.)*(float)k*PI/16.)*in[i];
         }
      }
      res_32=exp/4.;
      if ( res_32 > 255 ){
         out[i]=255;
      }
         else {


            out[i]=(uint8_t)res_32;
      }
      printf("%i \n",out[i]);
      exp=0;
   }

  printf("\n");
}
