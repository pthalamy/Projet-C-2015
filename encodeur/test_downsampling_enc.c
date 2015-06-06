#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


void affiche_tab (uint8_t *tab, uint8_t h, uint8_t v){
   for (uint8_t i=0; i<v; i++){
      for (uint8_t j=0; j<h; j++){
	 printf("  %i  ",tab[i*h+j]);
      }
      printf("\n");
   }
}

void print_block_down(uint8_t *bloc, uint32_t num_bloc)
{
   printf ("bloc numéro %d\n", num_bloc);
   for (uint32_t i = 0; i < 8; i++) {
      for (uint32_t j = 0; j < 8; j++) {
	 printf ("%d ", bloc[8*i + j]);
      }
      printf ("\n ");
   }
   printf ("\n");
}

void print_mcu_down(uint8_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv)
{
   printf ("mcu numéro %d\n", num_mcu);
   for (uint32_t i = 0; i < 64*sfv*sfh; i++) {
      if (!(i % (sfh * 8)))
	 printf ("\n");
      printf ("%d ", mcu[i]);
   }
   printf ("\n");
}

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
};

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
      printf ("cas 1 \n");
      reorganisation_verticale(in, nb_blocks_in_h, nb_blocks_in_v, out);

   } else if ( (nb_blocks_in_h == 2*nb_blocks_out_h) /* sous échantillonnage 4:2:2*/
	       && (nb_blocks_in_v == nb_blocks_out_v) ) {
      printf ("cas 2 \n");
      /* TODO: TEST THIS */
      uint8_t *temp = malloc (nb_blocks_in_h * nb_blocks_in_v * 64 * sizeof(uint8_t));
      reorganisation_verticale (in, nb_blocks_in_h, nb_blocks_in_v, temp);
      for (uint32_t i = 0; i < 2*1; i++) {
      	 print_block_down (temp+(i*64*sizeof(uint8_t)), i);
      }
      contraction_2_to_1(temp, temp + 64*sizeof(uint8_t), out);
      print_block_down(out, 0);

      free (temp);
   } else if ( (nb_blocks_in_h == 2*nb_blocks_out_h) /* sous-échantillonnage 4:2:0 */
	       && (nb_blocks_in_v == 2*nb_blocks_out_v) ) {
      printf ("cas 3 \n");
      /* TODO: Adapter le traitement pour une MCU sans reorganisation */
      /* Réorganisation en une colonne de bloc pour traitement simple */
      uint8_t *temp = malloc (nb_blocks_in_h * nb_blocks_in_v * 64 * sizeof(uint8_t));
      reorganisation_verticale (in, nb_blocks_in_h, nb_blocks_in_v, temp);
      /* for (uint32_t i = 0; i < 2*2; i++) { */
      /* 	 print_block_down (temp+(i*64*sizeof(uint8_t)), i); */
      /* } */
      contraction_4_to_1(temp, out);
      free (temp);
   } else {
      printf("ERREUR : format de sous échantillonnage non supporté");
      exit (EXIT_FAILURE);
   }
}

int main(void)
{
//  initialisation tableau

   /* uint8_t tab[16] ; */
   /* for (uint8_t i =0; i<16; i++) { */
   /*    tab[i]=i; */
   /* } */

   /* uint8_t tab32[32]; */
   /*  for (uint8_t i =0; i<16; i++) { */
   /*    tab32[i]=i; */
   /*    tab32[i+16]=i; */
   /* } */


   /*  uint8_t tab64[64]; */

   /*  for (uint8_t i=0; i<16; i++){ */
   /*     tab64[i]=i; */
   /*     tab64[i+16]=i; */
   /*     tab64[i+32]=i; */
   /*     tab64[i+48]=i; */
   /*  } */

   /* printf("Tab de base \n"); */
   /* affiche_tab(tab, 4,4) ; */

   /* printf("Tab2 \n"); */
   /* affiche_tab(tab32, 4,8); */

   /* uint8_t *out_11=malloc(8*sizeof(uint8_t)); */
   /* contraction_1_to_1(tab, out_11); */
   /* printf("contraction 1->1 \n"); */
   /* affiche_tab(out_11, 4, 4); */

   /* uint8_t*out_21=malloc(16*sizeof(uint8_t)); */
   /* contraction_2_to_1(tab32, out_21); */
   /* printf("contraction 2->1 \n"); */
   /* affiche_tab(out_21, 4,4) ; */

   /* uint8_t *out_41=malloc(16*sizeof(uint8_t)); */
   /* contraction_4_to_1(tab64, out_41); */
   /* printf("contraction 4->1 \n"); */
   /* affiche_tab(out_41,4,4); */


   /* free(out_11); free(out_21); free(out_41); */

   uint8_t mcu[256] = {181, 181, 181, 181, 181, 181, 181, 181, 180, 180, 181, 182, 183, 185, 185, 186,
		       181, 181, 181, 181, 181, 181, 181, 181, 180, 180, 181, 182, 184, 185, 186, 186,
		       181, 181, 181, 181, 181, 181, 181, 181, 181, 181, 182, 183, 184, 185, 186, 187,
		       182, 182, 182, 182, 182, 182, 182, 182, 181, 182, 183, 184, 185, 186, 187, 188,
		       183, 183, 183, 183, 183, 183, 183, 183, 182, 183, 184, 185, 186, 187, 188, 188,
		       184, 184, 184, 184, 184, 184, 184, 184, 183, 183, 184, 185, 187, 188, 189, 189,
		       184, 184, 184, 184, 184, 184, 184, 184, 184, 184, 185, 186, 187, 188, 189, 190,
		       185, 185, 185, 185, 185, 185, 185, 185, 184, 184, 185, 186, 188, 189, 190, 190,
		       184, 184, 184, 185, 185, 186, 186, 186, 186, 186, 187, 187, 187, 188, 188, 188,
		       184, 184, 185, 185, 185, 186, 186, 186, 186, 186, 187, 187, 188, 188, 188, 188,
		       184, 185, 185, 185, 186, 186, 186, 186, 187, 187, 187, 187, 188, 188, 189, 189,
		       185, 185, 185, 186, 186, 186, 187, 187, 187, 187, 187, 188, 188, 189, 189, 189,
		       185, 185, 186, 186, 186, 187, 187, 187, 187, 188, 188, 188, 189, 189, 189, 189,
		       186, 186, 186, 186, 187, 187, 187, 188, 188, 188, 188, 189, 189, 189, 190, 190,
		       186, 186, 186, 187, 187, 187, 188, 188, 188, 188, 189, 189, 189, 190, 190, 190,
		       186, 186, 186, 187, 187, 188, 188, 188, 188, 188, 189, 189, 189, 190, 190, 190};

   uint8_t mcu_420[256] = {162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162,
			   162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162, 162};

   /* print_mcu_down (mcu, 0, 2, 2); */

   uint8_t *blocs = malloc (256 * sizeof (uint8_t));

   /* downsampler (mcu, 2, 2, 2, 2, blocs); */
   /* downsampler (mcu, 2, 1, 1, 1, blocs); */

   for (uint32_t i = 0; i < 1*1; i++) {
      print_block_down (blocs+(i*64*sizeof(uint8_t)), i);
   }

   return 0;
}
