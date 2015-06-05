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

   /*2 blocs 8*8 -> 2 blocs 8*4 ->1 bloc 8*8 */
   void contraction_2_to_1(uint8_t *in, uint8_t *out){

      uint8_t *temp=malloc(64*sizeof(uint8_t));

      /* pour chaque bloc, 8*8 ->8*4  (les2 blocs sont a la suite)*/
      for (uint8_t i=0; i<128 ; i+=2){
	 temp[i/2]=0.5*(in[i]+in[i+1]);
      }

      /* Réordonnement: juxtaposition des deux blocs */

      for (uint8_t i=0; i<8; i++){
	 for (uint8_t j =0; j<4;j++) {
	    out[8*i+j]=temp[4*i+j];
	    out[8*i+j+4]=temp[4*i+32+j];
	 }

      }
      free(temp);
   };

  void contraction_4_to_1(uint8_t *in, uint8_t *out){


      /* 4blocs 8*8 = 2*(2 blocs 8*8)  */

      uint8_t *aux1=malloc(128*sizeof(uint8_t));
      uint8_t *aux2=malloc(128*sizeof(uint8_t));


      for (uint8_t i=0; i<128; i++){
	 aux1[i]=in[i]; //2 premiers blocs dans aux1
	 aux2[i]=in[i+128]; //2 derniers blocs dans aux2
	 };


      /* Contraction sur les lignes */
      uint8_t *temp1=malloc(64*sizeof(uint8_t));
      uint8_t *temp2=malloc(64*sizeof(uint8_t));

      contraction_2_to_1(aux1, temp1);
      contraction_2_to_1(aux2, temp2);

      /*Contraction colonnes*/
      uint8_t *out1=malloc(32*sizeof(uint8_t));
      uint8_t *out2=malloc(32*sizeof(uint8_t));

      for (uint8_t i=0; i<8; i+=2){
	 for(uint8_t j=0; j<8; j++){
	    out1[j+8*i/2]=0.5*(temp1[j+8*i]+temp1[j+8*i+8]);
	     out2[j+8*i/2]=0.5*(temp2[j+8*i]+temp2[j+8*i+8]);
	 }
      }


      /*juxtaposition verticale des deux blocs finaux*/
      for (uint8_t i=0; i<32; i++){
	 out[i]=out1[i];
	 out[i+8]=out2[i];

      }
      free(temp1);free(temp2); free(aux1); free(aux2); free(out1); free(out2);
   };


   void downsampler(uint8_t *in,
		    uint8_t nb_blocks_in_h, uint8_t nb_blocks_in_v,
		    uint8_t *out,
		    uint8_t nb_blocks_out_h, uint8_t nb_blocks_out_v){



   }
