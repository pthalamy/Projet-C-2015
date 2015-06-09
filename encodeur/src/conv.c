#include "conv.h"

#include <stdio.h>

enum YCbCr{
   Y,
   Cb,
   Cr
};

uint8_t to_YCbCr(enum YCbCr c, uint32_t R, uint32_t G, uint32_t B)
{

   int32_t res ;

   /* Calcul YCbCr correspondant */
   switch(c){
   case Y :
      res = 0.299*R + 0.587*G + 0.114*B + 1;
      break;
   case Cb:
      res = 128 - 0.168736*R - 0.331264*G + 0.5*B ;
      break;
   case Cr:
      res = 128 + 0.5*R - 0.418688*G - 0.081312*B;
      break;
   }
   /*Saturation et conversion en uint8_t*/
   uint8_t res_8;

   if (res<0){
      res_8 =0;
   } else if (res >255){
      res_8 =255 ;
   } else {
      res_8=(uint8_t) res;
   }
   return res_8;
}

void RGB_to_YCbCr(uint32_t *mcu_RGB, uint32_t nb_blocks_h, uint32_t nb_blocks_v, uint8_t *mcu_YCbCr[3])
{

   uint8_t R;
   uint8_t G;
   uint8_t B ;
   uint32_t nb_pixels=64*nb_blocks_h*nb_blocks_v;

   for(uint32_t i=0 ; i< nb_pixels; i++){

      /*Calcul des composantes R, G et B d'un pixel */
      R = (mcu_RGB[i] >> 16) & 0xff;
      G = (mcu_RGB[i] >> 8) & 0xff;
      B = (mcu_RGB[i]) & 0xff ;

      mcu_YCbCr[0][i]=to_YCbCr(Y, R,G,B);
      mcu_YCbCr[1][i]=to_YCbCr(Cb, R, G,B);
      mcu_YCbCr[2][i]=to_YCbCr(Cr,R,G,B);

   }

}
