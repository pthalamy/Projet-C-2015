#include "unpack.h"
#include <stdio.h>

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
   int8_t dc= next_huffman_value(table_DC,stream)-*pred_DC;
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

      // on récupère le symbole précédant le coeff
      //symbole=next_huffman_value(table_AC, stream);

      // on récupère le nombre de zéros et on remplit le bloc
      nb_zeros=(symbole>>4)&0x0F;
      for (uint8_t j=0; j<nb_zeros; j++){
	 bloc[i+j]=0;
      }
      i=i+nb_zeros ;
      printf("nb zeros : %i ", nb_zeros);
      printf("izéro : %i ",i);

      // on récupère la magnitude du symbole
      magnitude=(symbole & 0x0F);

      // on lit le coefficient
      uint32_t next=next_huffman_value(table_AC, stream);
      bloc[i]=next;

      i++;

      printf("fb i= %i \n",i);

   }

   printf("fin bloc \n");
   return ;
}
