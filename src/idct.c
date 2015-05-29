#include "idct.h"
#include <stdio.h>
#define PI 3.14159265358979323846
#include <math.h>

uint32_t C(int32_t xi) 
{
   if ( xi == 0 ){
      return 1/sqrt(2);
   } else {
      return 1;
   } 
}



void idct_block(int32_t in[64], uint8_t out[64])
{
   int32_t res_32=0;
   int32_t exp=0;

   for ( uint32_t i=0; i < 64; i++) 
   {
      exp=0;
      for ( uint32_t j=0; j < 8; j++)
      {
         for ( uint32_t k=0; k < 8; k++) 
         {
            exp= exp + C(j)*C(k)*cos((2*(i/8)+1)*j*PI/16)*cos((2*(i%8)+1)*k*PI/16)*in[8*j+k];
         }
      }
      res_32=(1/4)*exp;
      if ( res_32 > 255 ){
         out[i]=255;
      }
         else {
            out[i]=res_32;
      }
      printf("%d \t",out[i]);
   }
  printf("\n"); 
}
