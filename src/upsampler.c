#include "upsampler.h"
#include <stdio.h>
#include <stdlib.h>

// prérequis: bloc de taille 64
uint8_t *dilatation_ligne(uint8_t *bloc){
   uint8_t *new_bloc=malloc(128*sizeof(uint8_t));
   for(uint8_t i=0; i<128; i++){
      new_bloc[i]=bloc[i/2];
   }
   return  new_bloc ;
}

// prérequis : bloc de taille 128 ( 8*16 )
uint8_t *dilatation_colonne(uint8_t *bloc){
   uint8_t * new_bloc=malloc(256*sizeof(uint8_t));

   for (uint8_t i=0; i<16; i++){
      for(uint8_t j=0; j<16; j++){
      new_bloc[j+16*i]=bloc[i/2*16+j];

      }
   }
   return new_bloc ;
}

// 2 blocs de taille 64(8*8) -> un bloc de taille 128(8*16)
uint8_t* juxtaposition_horizontale(uint8_t *bloc){

   uint8_t *new_bloc=malloc(128*sizeof(uint8_t));

    for (uint8_t i=0; i<8; i++){
      for (uint8_t j=0 ; j<8 ; j++) {
	 new_bloc[16*i+j]=bloc[i*8+j];

	 new_bloc[j+8 +i*16]=bloc[i*8+64+j];
      }
   }
   return new_bloc ;
};





void upsampler(uint8_t *in,
	       uint8_t nb_blocks_in_h, uint8_t nb_blocks_in_v,
	       uint8_t *out,
	       uint8_t nb_blocks_out_h, uint8_t nb_blocks_out_v)
{


// Cas 4:4:4
   if ( (nb_blocks_in_h==nb_blocks_out_h) & (nb_blocks_in_v ==nb_blocks_out_v)) {
      out=in ;
   }

// Cas 4:2:2 :il faut etendre sur échantillonner le bloc
   else if ((2*nb_blocks_in_h==nb_blocks_out_h) & (nb_blocks_in_v==nb_blocks_out_v)){
      out=dilatation_ligne(in);
   }

// Cas 4:4:0 : il  faut sur échantillonner le bloc
   else if ((2*nb_blocks_in_h==nb_blocks_out_h) & (2*nb_blocks_in_v==nb_blocks_out_v)){
      out=dilatation_ligne(in);
      out=dilatation_colonne(out);

   }

// Si on veut transformer Y0-Y1 en un seul bloc
   else if ( (nb_blocks_in_h==2) & (nb_blocks_in_v==1)){

      out=juxtaposition_horizontale(in);
   }

// Si on veut transformer Y0-Y1-Y2-Y3 en un seul bloc

   // else if((nb_blocks_in_h==2) & (nb_blocks_in_v==2)) {



   //}


   else {
      printf("erreur : format d'échantillonnage non reconnu") ;
      exit(1);
   }
}
