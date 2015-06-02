#include "upsampler.h"
#include <stdio.h>
#include <stdlib.h>

// prérequis: bloc de taille 64
//(8*8)->(16*8)
uint8_t *dilatation_ligne(uint8_t *bloc, uint8_t *out){

   for(uint8_t i=0; i<128; i++){
      out[i]=bloc[i/2];
   }
   return  out ;
}

// prérequis : bloc de taille 64
//( 8*8 )->(16*16)
uint8_t *dilatation_lc(uint8_t *bloc, uint8_t *out){
   uint8_t *temp=malloc(256*sizeof(uint8_t));

   temp=dilatation_ligne(bloc, temp);

   for (uint8_t i=0; i<16; i++){
      for(uint8_t j=0; j<16; j++){
	 out[j+16*i]=temp[i/2*16+j];

      }
   }
   free(temp);
   return out ;
}

// 2 blocs de taille 64(8*8) -> un bloc de taille 128(8*16)
uint8_t* juxtaposition_horizontale(uint8_t *bloc, uint8_t *out){


   for (uint8_t i=0; i<8; i++){
      for (uint8_t j=0 ; j<8 ; j++) {
	 out[16*i+j]=bloc[i*8+j];

	 out[j+8 +i*16]=bloc[i*8+64+j];
      }
   }
   return out ;
};


//4blocs de taille 64(8*8) --> un bloc de taille (16*16)
uint8_t *juxtaposition_hv(uint8_t *bloc, uint8_t *out){

   uint8_t *temp1=malloc(128*sizeof(uint8_t));
   uint8_t *temp1bis=malloc(128*sizeof(uint8_t));
    uint8_t *temp2=malloc(128*sizeof(uint8_t));
   uint8_t *temp2bis=malloc(128*sizeof(uint8_t));

   for(uint8_t i=0; i<128; i++){
      temp1[i]=bloc[i];
      temp1bis[i]=bloc[i];
      temp2[i]=bloc[i+128];
      temp2bis[i]=bloc[i+128];
   }

   temp1=juxtaposition_horizontale(temp1, temp1bis);
   temp2=juxtaposition_horizontale(temp2, temp2bis);

   for (uint8_t i=0; i<128; i++){
      out[i]=temp1[i];
      out[i+128]=temp2[i];
   }
   free(temp1);
   free(temp2);
   free(temp1bis);
   free(temp2bis);
   return out;
}


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
      out=dilatation_ligne(in, out);
   }

// Cas 4:4:0 : il  faut sur échantillonner le bloc
   else if ((2*nb_blocks_in_h==nb_blocks_out_h) & (2*nb_blocks_in_v==nb_blocks_out_v)){

      out=dilatation_lc(in, out);

   }

// Si on veut transformer Y0-Y1 en un seul bloc
   else if ( (nb_blocks_in_h==2) & (nb_blocks_in_v==1)){

      out=juxtaposition_horizontale(in, out);
   }

// Si on veut transformer Y0-Y1-Y2-Y3 en un seul bloc

   else if((nb_blocks_in_h==2) & (nb_blocks_in_v==2)) {

      juxtaposition_hv(in, out);
   }






   else {
      printf("erreur : format d'échantillonnage non reconnu") ;
      exit(1);
   }
}
