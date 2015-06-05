#include "downsampling_enc.h"
#include <stdio.h>
#include <stdlib.h>

/* ATTENTION POUR LES CONTRACTIONS : faire moyenne sur les RGB ET PAS Y Cb Cr (sous échantillonner avant la conversion y Cb Cr)*/


/*bloc 8*8 -> bloc 8*8  */
void contraction_1_to_1(uint8_t *in, uint8_t *out){
      for (uint8_t i=0 ; i<64; i++){
	 out[i]=in[i];
      }
   }

   /*2 blocs 8*8 -> 2 blocs 8*4 */
   void contraction_2_to_1(uint8_t *in, uint8_t *out){

      /* pour chaque bloc, 8*8 ->8*4 */
      for (uint8_t i=0; i<128 ; i+=2){
	 out[i/2]=0.5*(in[i]+in[i+1]);
      }

   };


/* 4 blocs 8*8 -> 4 blocs 2*2 */
   void contraction_4_to_1(uint8_t *in, uint8_t *out){

      uint8_t *temp=malloc(32*sizeof(uint8_t));

      contraction_2_to_1(in ,temp);

      for (uint8_t i=0; i<4; i++){
	 for(uint8_t j=0; j<8; j+=2){
	    out[j+16*i]=0.5*(temp[j+16*i]+temp[j+16*i+16]);
	 }
      }
      free(temp);
   };


   void downsampler(uint8_t *in,
		    uint8_t nb_blocks_in_h, uint8_t nb_blocks_in_v,
		    uint8_t *out,
		    uint8_t nb_blocks_out_h, uint8_t nb_blocks_out_v){



   }
