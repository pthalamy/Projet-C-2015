#include "upsampler.h"
#include <stdio.h>
#include <stdlib.h>

//ATTENTION : a tester si possible les MCU 8*16. Attention au valgrind, problème.



// prérequis: bloc de taille 64
//(8*8)->(16*8)
void dilatation_ligne(uint8_t *bloc, uint8_t *out){

   for(uint8_t i=0; i<128; i++){
      out[i]=bloc[i/2];
   }
}

// prérequis : bloc de taille 64
//( 8*8 )->(16*16)
void dilatation_lc(uint8_t *bloc, uint8_t *out){
   uint8_t *temp=malloc(256*sizeof(uint8_t));

   dilatation_ligne(bloc, temp);

   for (uint8_t i=0; i<16; i++){
      for(uint8_t j=0; j<16; j++){
	 out[j+16*i]=temp[i/2*16+j];

      }
   }

   free(temp);
}

// 2 blocs de taille 64(8*8) -> un bloc de taille 128(8*16)
void juxtaposition_horizontale(uint8_t *bloc, uint8_t *out){


   for (uint8_t i=0; i<8; i++){
      for (uint8_t j=0 ; j<8 ; j++) {
	 out[16*i+j]=bloc[i*8+j];

	 out[j+8 +i*16]=bloc[i*8+64+j];
      }
   }
};


//4blocs de taille 64(8*8) --> un bloc de taille (16*16)
void juxtaposition_hv(uint8_t *bloc, uint8_t *out){

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

   juxtaposition_horizontale(temp1, temp1bis);
   juxtaposition_horizontale(temp2, temp2bis);

   for (uint8_t i=0; i<128; i++){
      out[i]=temp1bis[i];
      out[i+128]=temp2bis[i];
   }
   free(temp1);
   free(temp2);
   free(temp1bis);
   free(temp2bis);
}

void inout(uint8_t *in, uint8_t*out){
   for (uint8_t i=0; i<64; i++){
      out[i]=in[i];

   }
}

#include <stdio.h>
#include <stdlib.h>

void check_alloc_unpack(void* ptr)
{
   if (!ptr) {
      fprintf (stderr, "alloc error: OUT OF MEMORY\n");
   }
}

// prérequis: bloc de taille 64
//(8*8)->(16*8)
void dilatation_ligne(uint8_t *bloc, uint8_t *out){
   for(uint8_t i=0; i<128; i++){
      out[i]=bloc[i/2];
   }
}

// prérequis : bloc de taille 64
//( 8*8 )->(16*16)
void dilatation_lc(uint8_t *bloc, uint8_t *out){
   uint8_t *temp=malloc(256*sizeof(uint8_t));
   check_alloc_unpack (temp);

   dilatation_ligne(bloc, temp);

   for (uint8_t i=0; i<16; i++){
      for(uint8_t j=0; j<16; j++){
	 out[j+16*i]=temp[i/2*16+j];
      }
   }

   free(temp);
}

// 2 blocs de taille 64(8*8) -> un bloc de taille 128(8*16)
void juxtaposition_horizontale(uint8_t *bloc, uint8_t *out){


   for (uint8_t i=0; i<8; i++){
      for (uint8_t j=0 ; j<8 ; j++) {
	 out[16*i+j]=bloc[i*8+j];
	 out[j+8 +i*16]=bloc[i*8+64+j];
      }
   }
};

//4blocs de taille 64(8*8) --> un bloc de taille (16*16)
void juxtaposition_hv(uint8_t *bloc, uint8_t *out){

   uint8_t *temp1=malloc(128*sizeof(uint8_t));
   check_alloc_unpack (temp1);
   uint8_t *temp1bis=malloc(128*sizeof(uint8_t));
   check_alloc_unpack (temp1bis);
   uint8_t *temp2=malloc(128*sizeof(uint8_t));
   check_alloc_unpack (temp2);
   uint8_t *temp2bis=malloc(128*sizeof(uint8_t));
   check_alloc_unpack (temp2bis);

   for(uint8_t i=0; i<128; i++){
      temp1[i]=bloc[i];
      temp1bis[i]=bloc[i];
      temp2[i]=bloc[i+128];
      temp2bis[i]=bloc[i+128];
   }

   juxtaposition_horizontale(temp1, temp1bis);
   juxtaposition_horizontale(temp2, temp2bis);

   for (uint8_t i=0; i<128; i++){
      out[i]=temp1bis[i];
      out[i+128]=temp2bis[i];
   }
   free(temp1);
   free(temp2);
   free(temp1bis);
   free(temp2bis);
}

void inout(uint8_t *in, uint8_t*out){
   for (uint8_t i=0; i<64; i++){
      out[i]=in[i];
   }
}

void upsampler(uint8_t *in,
	       uint8_t nb_blocks_in_h, uint8_t nb_blocks_in_v,
	       uint8_t *out,
	       uint8_t nb_blocks_out_h, uint8_t nb_blocks_out_v)
{
// Cas 4:4:4
   if ( (nb_blocks_out_h==1) && (nb_blocks_out_v ==1)) {
       inout(in, out) ;
   }

// Cas 4:2:2 :il faut sur échantillonner le bloc
   else if ((2*nb_blocks_in_h==nb_blocks_out_h) && (nb_blocks_in_v==nb_blocks_out_v)){
      dilatation_ligne(in, out);

   }

// Cas 4:2:0 : il  faut sur échantillonner le bloc
   else if ((2*nb_blocks_in_h==nb_blocks_out_h) && (2*nb_blocks_in_v==nb_blocks_out_v)){
      dilatation_lc(in, out);

   }

   else if((nb_blocks_in_h==nb_blocks_out_h) && (nb_blocks_in_v==nb_blocks_out_v)) {
      if (nb_blocks_out_h == 2 *nb_blocks_out_v)  /* 2 blocs 8x8 -> 1 bloc 8x16 */
	 juxtaposition_horizontale(in, out);
      else // Si on veut transformer Y0-Y1-Y2-Y3 en un seul bloc (4 blocs 8*8 --> 1 bloc 16*16)
	 juxtaposition_hv(in, out);

   }

   else {
      printf("erreur : format d'échantillonnage non reconnu \n") ;
      exit(EXIT_FAILURE);
   }
}
