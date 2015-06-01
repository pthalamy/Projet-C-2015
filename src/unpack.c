#include "unpack.h"
#include <stdio.h>
#include <stdlib.h>

#define BITMASK(n) (0xFFFFFFFF)>>(32-(n))
#define EOB 0x00
#define ZRL 0xF0

int16_t mag_neg_val (uint8_t mag, uint32_t val)
{
   return (val - (BITMASK(mag)));
}


// permet de retrouver une valeur à partir d'une magnitude
int32_t decode_magnitude(struct bitstream *stream, uint8_t magnitude)
{
   uint32_t dest = 0;

   // on lit le nombre de bits correspondant à la magnitude
   uint8_t mag = read_bitstream(stream, magnitude, &dest, true);
   (void)mag;
   dest &= BITMASK(magnitude);

   // on décode la valeur dans la classe de magnitude
   uint8_t premier_bit = dest >> (magnitude-1);
   if (premier_bit == 1){
      return dest ;
   }  else {
      printf ("-> ac' : m = %d  |  0x%x -> %d\n", magnitude, dest, dest - (BITMASK(magnitude)));
      return (dest - (BITMASK(magnitude)));
   }
}



void unpack_block(struct bitstream *stream,
		  struct huff_table *table_DC, int32_t *pred_DC,
		  struct huff_table *table_AC,
		  int32_t bloc[64])
{
   // Decodage de DC
   if (stream == NULL)
      return;
   if (table_DC == NULL)
      return;
   if (table_AC == NULL)
      return;

// récupération de la magnitude
   uint8_t magnitude = next_huffman_value(table_DC,stream);

   int32_t dc = decode_magnitude(stream, magnitude) - *pred_DC;
   printf ("-> dc : m = %d  |  %d\n", magnitude, dc);
   bloc[0]=dc;

// Decodage de AC (x63)
   uint8_t i = 1 ;
   uint8_t nb_zeros;

   while (i < 64) {
      // on récupère le symbole sur un octet (nombre de 0+magnitude)
      uint32_t symbole = next_huffman_value(table_AC, stream);

      // si on lit un caractère de fin de bloc
      // on remplit la fin du bloc avec des 0
      if (symbole == EOB) {
	 /* printf ("EOB !\n"); */
	 for (uint8_t j = i ; j < 64; j++) {
	    bloc[j] = 0;
	 }

	 return;
      } else if (symbole == ZRL) {
	 /* printf ("ZRL !\n"); */
	 for (uint8_t j = 0; j < 16; j++) {
	    bloc[i+j] = 0;
	 }

	 i += 16;
      } else {
	 // on récupère le nombre de zéros et on remplit le bloc
	 nb_zeros = (symbole >> 4) & 0x0F;
	 for (uint8_t j = 0; j < nb_zeros;  j++) {
	    bloc[i+j] = 0;
	 }
	 i += nb_zeros ;

	 // on récupère la magnitude du symbole
	 magnitude = (symbole & 0x0F);

	 // on lit le coefficient
	 bloc[i] = decode_magnitude(stream,magnitude);
	 printf ("-> ac%d : m = %d  |  %d\n", i, magnitude, bloc[i]);
	 if (!bloc[i])
	    exit (1);
	 i++;
      }
   }

   return ;
}
