#include "bitstream_enc.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t magnitude(int32_t val){

   uint8_t mag=1 ;
   uint8_t i=abs(val) ;
   while (i>1){
      i=i/2;
      mag++;
   }
   return mag ;
}

/*retourne l'indice de val dans la classe de magnitude mag*/
uint8_t val_to_mag(int32_t val,uint8_t mag){
   if (val>=0) {
      return val ;
   } else {
      return (uint8_t)(0xFFFFFFFF >> (32-mag))&(~ abs(val));
	 };

}

int main(void){

   int32_t n = 35 ;
   printf ("n=%i,  mag=%i, ind=%i \n", n, magnitude(n), val_to_mag(n,magnitude(n)));

   n=14 ;
    printf ("n=%i,  mag=%i, ind=%i \n", n, magnitude(n), val_to_mag(n,magnitude(n)));

   n=-12 ;
    printf ("n=%i,  mag=%i, ind=%i \n", n, magnitude(n), val_to_mag(n,magnitude(n)));

   n=-9 ;
    printf ("n=%i,  mag=%i, ind=%i \n", n, magnitude(n), val_to_mag(n,magnitude(n)));

   return 0;
}
