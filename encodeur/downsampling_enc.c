#include "downsampling_enc.h"

#include <stdio.h>
#include <stdlib.h>

/* ATTENTION POUR LES CONTRACTIONS : faire moyenne sur les RGB ET PAS Y Cb Cr (sous échantillonner avant la conversion y Cb Cr)*/

/*bloc 256 -> 4 blocs 8*8   */
void reorganisation_verticale(uint8_t *in,
			uint8_t nb_blocks_h, uint8_t nb_blocks_v,
			uint8_t *out)
{
   uint8_t bloc_id = 0;

   for (uint8_t y = 0 ; y < nb_blocks_v; y++) {
      for (uint8_t x = 0 ; x < nb_blocks_h; x++) {

	 for (uint8_t i = 0 ; i < 8; i++) {
	    for (uint8_t j = 0 ; j < 8; j++) {
	       uint8_t out_id = 64*bloc_id + 8*i + j;
	       uint8_t in_id = y*128 + 8*x + 16*i + j;
	       out[out_id] = in[in_id];
	       /* printf ("out: %d\t|\tin: %d\n", out_id, in_id); */
	    }
	 }

	 bloc_id++;
      }
   }
}

/* 2 bloc de taille 8*8 -> 1 bloc 8*8 */
void contraction_2_to_1 (uint8_t *in1, uint8_t *in2, uint8_t *out)
{
   for (uint8_t i = 0; i <  64; i++){
      out[i] = 0.5*(in1[i] + in2[i]);
      /* printf ("%d = 0.5 * (%d + %d)\n", out[i], in1[i], in2[i]); */
   }
}

/* 4 blocs 4*4 -> 4 blocs 2*2 -> 1 bloc 4*4 */
void contraction_4_to_1(uint8_t *in, uint8_t *out){
   /* Passage en 8*16 */
   uint8_t *temp1 = malloc(64*sizeof(uint8_t));
   uint8_t *temp2 = malloc(64*sizeof(uint8_t));

   contraction_2_to_1 (in, in + (64 * sizeof(uint8_t)), temp1);
   contraction_2_to_1 (in + (128 * sizeof(uint8_t)), in + (192 * sizeof(uint8_t)), temp2);

   contraction_2_to_1 (temp1, temp2, out);

   free (temp1);
   free (temp2);
}

void downsampler(uint8_t *in,
		 uint8_t nb_blocks_in_h, uint8_t nb_blocks_in_v,
		 uint8_t nb_blocks_out_h, uint8_t nb_blocks_out_v,
		 uint8_t *out)
{
   if ( (nb_blocks_in_h == nb_blocks_out_h)   /* pas de sous échantillonnage */
	&& (nb_blocks_in_v == nb_blocks_out_v) ) {
      reorganisation_verticale(in, nb_blocks_in_h, nb_blocks_in_v, out);

   } else if ( (nb_blocks_in_h == 2*nb_blocks_out_h) /* sous échantillonnage 4:2:2*/
	       && (nb_blocks_in_v == nb_blocks_out_v) ) {
      /* TODO: TEST THIS */
      uint8_t *temp = malloc (nb_blocks_in_h * nb_blocks_in_v * 64 * sizeof(uint8_t));
      reorganisation_verticale (in, nb_blocks_in_h, nb_blocks_in_v, temp);
      contraction_2_to_1(temp, temp + 64*sizeof(uint8_t), out);

      free (temp);
   } else if ( (nb_blocks_in_h == 2*nb_blocks_out_h) /* sous-échantillonnage 4:2:0 */
	       && (nb_blocks_in_v == 2*nb_blocks_out_v) ) {
      /* TODO: Adapter le traitement pour une MCU sans reorganisation */
      /* Réorganisation en une colonne de bloc pour traitement simple */
      uint8_t *temp = malloc (nb_blocks_in_h * nb_blocks_in_v * 64 * sizeof(uint8_t));
      reorganisation_verticale (in, nb_blocks_in_h, nb_blocks_in_v, temp);
      contraction_4_to_1(temp, out);

      free (temp);
   } else {
      printf("ERREUR : format de sous échantillonnage non supporté");
      exit (EXIT_FAILURE);
   }
}
