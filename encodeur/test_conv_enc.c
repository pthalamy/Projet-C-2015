/* TEST A FAIRE !!*/
#include <stdio.h>
#include <stdint.h>

enum YCbCr{
   Y,
   Cb,
   Cr
};


uint32_t to_YCbCr(enum YCbCr c, uint32_t R, uint32_t G, uint32_t B){

   int32_t res ;

   /* Calcul YCbCr correspondant */
   switch(c){
   case Y :
      res = 0.299*R+0.587*G+0.114*B ;
      break;
   case Cb:
      res = -0.1687*R-0.3313*G+0.5*B+128 ;
      break;
   case Cr:
      res=0.5*R-0.4187*G+0.0813*B+128 ;
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

void RGB_to_YCbCr(uint8_t *mcu_YCbCr[3], uint32_t *mcu_RGB, uint32_t nb_blocks_h, uint32_t nb_blocks_v){

   uint8_t R;
   uint8_t G;
   uint8_t B ;
   uint32_t nb_pixels=4*nb_blocks_h*nb_blocks_v;

   for(uint32_t i=0 ; i< nb_pixels; i++){

      /*Calcul des composantes R, G et B d'un pixel */
      R=(mcu_RGB[i]>>16)&0x000F;
      G=(mcu_RGB[i]>>8)& 0x000F;
      B=(mcu_RGB[i])&0x000F ;

      mcu_YCbCr[0][i]=to_YCbCr(Y, R,G,B);
      mcu_YCbCr[1][i]=to_YCbCr(Cb, R, G,B);
      mcu_YCbCr[2][i]=to_YCbCr(Cr,R,G,B);

   }

}




main(void){

   uint32_t tab[4]{0xb8a092,0xb19b8d, 0xb29b8d, 0xb49e90};
   RGB_to_YCbCr(mcu, &tab, 1,1);

}
