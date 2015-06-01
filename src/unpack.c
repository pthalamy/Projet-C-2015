#include "unpack.h"
#include <stdio.h>

#define BITMASK(n) (0xFFFFFFFF)>>(32-(n))

// permet de retrouver une valeur à partir d'une magnitude
int32_t decode_magnitude(uint8_t magnitude, struct bitstream *stream,
			 uint32_t dest, bool byte_stuffing){

   // on lit le nombre de bits correspondant à la magnitude
   uint8_t mag = read_bitstream(stream, magnitude, &dest, byte_stuffing);
   dest &= BITMASK(magnitude);

   // on décode la valeur dans la classe de magnitude
   uint8_t premier_bit = dest >> (magnitude-1);
   printf("pbit  %i ", premier_bit);
   if (premier_bit ==1){
      printf("dest %i ", dest);
      return dest ;
   }
   else {
      printf ("dest %i ", dest);
      return ( ~dest);
   }
}



void unpack_block(struct bitstream *stream,
		  struct huff_table *table_DC, int32_t *pred_DC,
		  struct huff_table *table_AC,
		  int32_t bloc[64])
{
  // Decodage de DC
   if (stream==NULL)
      return;
   if (table_DC==NULL)
      return;
   if (table_AC==NULL)
      return;

// récupération de la magnitude
   printf("decode dc ");
   uint8_t magnitude = next_huffman_value(table_DC,stream);
   uint32_t dest;
   bool byte_stuffing=true;

   int8_t dc= decode_magnitude(magnitude,stream, dest, byte_stuffing)-*pred_DC;
   bloc[0]=dc;

   printf("dc décodé \n");

// Decodage de AC (x63)
   uint8_t i=1 ;
   uint8_t nb_zeros;
   uint8_t symbole ;

   printf("boucle AC \n");

   while (i<64) {
      printf("i= %i ", i);

      // on récupère le symbole sur un octet (nombre de 0+magnitude)
      uint32_t symbole=next_huffman_value(table_AC, stream);

      // si on lit un caractère de fin de fichier
      // on remplit la fin du bloc avec des 0
      if (symbole==0x00){
	 for (uint8_t j=i ; j<64; j++){
	    bloc[j]=0;
	       }

      }



      // on récupère le nombre de zéros et on remplit le bloc
      nb_zeros=(symbole>>4)&0x0F;
      for (uint8_t j=0; j<nb_zeros; j++){
	 bloc[i+j]=0;
      }
      i=i+nb_zeros ;
      //printf("nb zeros : %i ", nb_zeros);
      //printf("izéro : %i ",i);

      // on récupère la magnitude du symbole
      magnitude=(symbole & 0x0F);

      // on lit le coefficient
      bloc[i]=decode_magnitude(magnitude,stream,dest, byte_stuffing);
      printf("bloc %i, magnitude %i \n", bloc[i], magnitude);
      i++;

      //printf("fb i= %i \n",i);

   }

   printf("fin bloc \n");
   return ;
}
