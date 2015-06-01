#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "tiff.h"

#define NUM_ENTRIES 0x000c

/* TYPES DE DONNEES */
#define SHORT							 0x0003
#define LONG	 						 0x0004
#define RATIONNAL 	 					 0x0005

/* TAGS */
#define IMAGE_WIDTH 	 				 0x0100
#define IMAGE_LENGTH 	 				 0x0101
#define BITS_PER_SAMPLE 	 			 0x0102
#define COMPRESSIO  	 			         0x0103
#define PHOTOMETRIC_INTERPRETATION   0x0106
#define STRIP_OFFSET  	 			         0x0111
#define SAMPLE_PER_PIXEL  	 			 0x0115
#define ROWS_PER_STRIP  	 			 0x0116
#define STRIP_BYTE_COUNTS  	 		 0x0117
#define X_RESOLUTION  	 			         0x011a
#define Y_RESOLUTION  	 			         0x011b
#define RESOLUTION_UNIT  	 			 0x0128
#define SOFTWARE  	 			  	 	 0x0131

/* Structure permettant de stocker les informations nécessaire à
 * l'écriture des données de l'image dans un fichier TIFF. */
struct tiff_file_desc
{
   uint32_t width;
   uint32_t height;
   uint32_t row_per_strip;
   FILE *tiff;
};

void fput16b(FILE *fp,  uint16_t v)
{
   fputc((v >> 8), fp);
   fputc((v & 0x0f), fp);
}

void fput32b(FILE *fp,  uint16_t v)
{
   fputc((v >> 24), fp);
   fputc((v >> 16), fp);
   fputc((v >> 8), fp);
   fputc((v & 0x0f), fp);
}

void tiff_write_entry(FILE *fp, uint16_t tag, uint16_t type, uint32_t nb_val, uint32_t val)
{
   fput16b (fp, tag);
   fput16b (fp, type);
   fput32b (fp, nb_val);
   fput32b (fp, val);
}

/* Initialisation du fichier TIFF résultat, avec les paramètres suivants:
   - width: la largeur de l'image ;
   - height: la hauteur de l'image ;
   - row_per_strip: le nombre de lignes de pixels par bande.
*/
struct tiff_file_desc *init_tiff_file (const char *file_name,
				       uint32_t width,
				       uint32_t height,
				       uint32_t row_per_strip)
{
   struct tiff_file_desc *tfd = malloc (sizeof(struct tiff_file_desc));
   tfd->tiff = fopen(file_name,"w");
   tfd->width = width;
   tfd->height = height;

   /* Ecriture du header */
   tfd->row_per_strip=row_per_strip;
   uint8_t buffer[8]={0x4D, 0x4D, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08};
   fwrite(buffer, 1, 8, tfd->tiff);

   /* Ecriture de l'IFD */
   /* Nombre d'entrées */
   fput16b (tfd->tiff, NUM_ENTRIES);
   /* Ecriture des entries */
   if ((tfd->width >> 16) == 0 )
      tiff_write_entry(tfd->tiff, IMAGE_WIDTH, SHORT, 1, tfd->width);
   else
      tiff_write_entry(tfd->tiff, IMAGE_WIDTH, LONG, 1, tfd->width);

   if ((tfd->height >> 16) == 0 )
      tiff_write_entry(tfd->tiff, IMAGE_LENGTH, SHORT, 1, tfd->height);
   else
      tiff_write_entry(tfd->tiff, IMAGE_LENGTH, LONG, 1, tfd->height);

   return tfd;
}

/* Ferme le fichier associé à la structure tiff_file_desc passée en
 * paramètre et désalloue la mémoire occupée par cette structure. */
void close_tiff_file(struct tiff_file_desc *tfd)
{
   fclose(tfd->tiff);
   free(tfd);
   tfd=NULL;
}

/* Ecrit le contenu de la MCU passée en paramètre dans le fichier TIFF
 * représenté par la structure tiff_file_desc tfd. nb_blocks_h et
 * nb_blocks_v représentent les nombres de blocs 8x8 composant la MCU
 * en horizontal et en vertical. */
int32_t write_tiff_file (struct tiff_file_desc *tfd,
			 uint32_t *mcu_rgb,
			 uint8_t nb_blocks_h,
			 uint8_t nb_blocks_v)
{

}

int main(void)
{
   struct tiff_file_desc *tfd = init_tiff_file("gros_lee.tiff",10,10,10);

   close_tiff_file(tfd);

   return 0;
}
