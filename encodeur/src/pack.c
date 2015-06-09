#include "pack.h"
#include <stdio.h>
#include <stdlib.h>
#include "huffman.h"

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
	    struct abr *table_DC,
	     int32_t bloc[64] ){


   int32_t dc= bloc[0]-*pred_DC ;

   /*Détermination de la magnitude de dc*/
   uint8_t mag = magnitude(dc);


   /*Ecriture du code de la magnitude dans bitstream */

   uint8_t *code_mag=0;
   uint8_t nb_bits ;
   huffman_value(table_DC,mag, code_mag, &nb_bits);
   write_bitstream(stream, nb_bits, *code_mag);

   /* Ecriture de l'indice de val dans bitstream*/
   uint32_t ind=(uint32_t)val_to_mag(dc,mag );
   uint8_t mag_ind=magnitude(ind);
   write_bitstream(stream,mag_ind,ind);

}


/*Codage RLE des coefficients AC*/
// Attention ne marche qu'avec des blocs  8*8
void RLE_AC(struct bitstream *stream,
	    int32_t bloc[64],
	    struct abr *table_AC
   ){

   uint8_t i=1 ;
   uint32_t nb_zeros=0;
   uint8_t val ;
   uint8_t mag ;
   uint8_t der =1;
   uint8_t *code=0 ;
   uint8_t *nb_bits=0 ;
   while (i<63){

      // si on a un zéro on passe au coeff suivant en incrémentant le compteur
      if (bloc[i]==0){
	 nb_zeros++;
	 i++;


      } else {
	 if (nb_zeros>16){
	    printf("erreur: format incorrect");
	 }

	 /*calcul du symbole (nb zeros + mag) */
	 mag=magnitude(bloc[i]);
	 val= (nb_zeros <<4)||(0x0F & mag);
	 huffman_value(table_AC, val, code, nb_bits);
	 write_bitstream(stream, *nb_bits, (uint32_t )*code) ;

	 /*calcul indice de bloc[i]*/
	 val=val_to_mag(bloc[i], mag);
	 write_bitstream(stream, mag, val);

	 /*maj indices*/
	 der=i;
	 i++;
	 nb_zeros=0;

      }

      if (der !=63){
	 write_bitstream(stream, 8, 0x00);
      }

   }
}
