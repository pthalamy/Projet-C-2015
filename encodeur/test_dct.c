#include "dct.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#define N 8.
#define COS(x, lambda) (cos(((2. * x + 1.) * lambda * acos(-1) / 16)))

/* question : dans quel type de données on stocke le in et le out*/


double C(int32_t xi) {
   if (xi==0){
      return 1./sqrt(2.) ;
   }
   else {
      return 1.;
   }
}


void dct(uint8_t in[64], int32_t out[64]){

   for (uint32_t u=0; u<8; u++){
      for (uint32_t v=0; v<8; v++){
	 double S=0 ;

	 for (uint32_t i=0; i<8; i++){
	    for (uint32_t j=0; j<8; j++){
	      S+= C(i)*C(j)*COS(i,u)*COS(j,v)*in[8*i+j];
	    }
	 }

	 S*=0.25;
	 //S=S+128.;



	 out[8*u+v]=S;
      }
   }
}


void affiche_tab (int32_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }


}

int main(void){

   uint8_t in[64];
   in[0]=0xa6;
   in[1]=0xa1;
   in[2]=0x9b;
   in[3]=0x9a;
   in[4]=0x9b;
   in[5]=0x9c;
   in[6]=0x97;
   in[7]=0x92;
   in[8]=0x9f;
   in[9]=0xa3;
   in[10]=0x9d;
   in[11]=0x8e;
   in[12]=0x89;
   in[13]=0x8f;
   in[14]=0x95;
   in[15]=0x94;
   in[16]=0xa5;
   in[17]=0x97;
   in[18]=0x96;
   in[19]=0xa1;
   in[20]=0x9e;
   in[21]=0x90;
   in[22]=0x90;
   in[23]=0x9e;
   in[24]=0xa7;
   in[25]=0x9b;
   in[26]=0x91;
   in[27]=0x91;
   in[28]=0x92;
   in[29]=0x91;
   in[30]=0x91;
   in[31]=0x94;
   in[32]=0xca;
   in[33]=0xda;
   in[34]=0xc8;
   in[35]=0x98;
   in[36]=0x85;
   in[37]=0x98;
   in[38]=0xa2;
   in[39]=0x96;
   in[40]=0xf0;
   in[41]=0xf7;
   in[42]=0xfb;
   in[43]=0xe8;
   in[44]=0xbd;
   in[45]=0x96;
   in[46]=0x90;
   in[47]=0x9d;
   in[48]=0xe9;
   in[49]=0xe0;
   in[50]=0xf1;
   in[51]=0xff;
   in[52]=0xef;
   in[53]=0xad;
   in[54]=0x8a;
   in[55]=0x90;
   in[56]=0xe7;
   in[57]=0xf2;
   in[58]=0xf1;
   in[59]=0xeb;
   in[60]=0xf7;
   in[61]=0xfb;
   in[62]=0xd0;
   in[63]=0x97;



   int32_t out[64] ;
   dct(in, out);
   affiche_tab(out, 8,8);

return 0 ;

}
