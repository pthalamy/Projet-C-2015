#include "pack.h"
#include <stdio.h>
#include <stdlib.h>

/* MAJ de bitstream.h => Pas besoin de donner un pointeur sur l'entier à écrire
   en argument de write_bitstream */

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

/* Codage différentiel de DC */
void diff_DC(struct bitstream *stream,
	     int32_t  *pred_DC,
	     //struct huff_table *table_DC,
	     int32_t bloc[64] ){


   int32_t dc= bloc[0]-*pred_DC ;

   /*Détermination de la magnitude de dc*/
   uint8_t mag = magnitude(dc);


   /*Ecriture du code de la magnitude dans bitstream */

   //int32_t code_mag = huffman(mag, table_DC)
   //int32_t nb_bits=magnitude(code_mag) ;
   //write_bitstream(stream, nb_bits, code_mag);

   /* Ecriture de l'indice de val dans bitstream*/
   uint32_t ind=(uint32_t)val_to_mag(dc,mag );
   uint8_t mag_ind=magnitude(ind);
   write_bitstream(stream,mag_ind,ind);

}


/*Codage RLE des coefficients AC*/
