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
	 // S+=128. ?? fait dans l'idct pourquoi ? le faire ici ?

	 if (S<0) S=0 ;
	 if (S>255) S=255;

	 out[8*u+v]=S;


      }
   }
}
