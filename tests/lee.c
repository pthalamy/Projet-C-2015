#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define NUM_ENTRIES 0x000c
#define OFFSET_SUIV 0x0000

/* TYPES DE DONNEES */
#define SHORT							 0x0003
#define LONG	 						 0x0004
#define RATIONNAL 	 					 0x0005

/* TAGS */
#define IMAGE_WIDTH 	 				 0x0100
#define IMAGE_LENGTH 	 				 0x0101
#define BITS_PER_SAMPLE 	 			 0x0102
#define COMPRESSION  	 			         0x0103
#define PHOTOMETRIC_INTERPRETATION                       0x0106
#define STRIP_OFFSET  	 			         0x0111
#define SAMPLE_PER_PIXEL  	 			 0x0115
#define ROWS_PER_STRIP  	 			 0x0116
#define STRIP_BYTE_COUNTS  	 		         0x0117
#define X_RESOLUTION  	 			         0x011a
#define Y_RESOLUTION  	 			         0x011b
#define RESOLUTION_UNIT  	 			 0x0128

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
   fputc((v & 0xff), fp);
}

void fput32b(FILE *fp,  uint32_t v)
{
   fputc((v >> 24), fp);
   fputc((v >> 16), fp);
   fputc((v >> 8), fp);
   fputc((v & 0xff), fp);
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

   /*nombre de colonnes de l'image*/
   tfd->width = width;
   /*nombre de lignes de l'image*/
   tfd->height = height;
   put("tfd->height\n",%d);
   /*hauteur (en pixels) es lignes TIFF*/
   tfd->row_per_strip=row_per_strip;
   /* Calcul du nombre de strip */
   uint32_t nb_strips = tfd->height / tfd->row_per_strip;
   /*calcul de la taille (en octets) des lignes*/
   uint32_t strip_byte_count=3*row_per_strip*height;

   /* Ecriture du header */
   uint8_t buffer[8]={0x4D, 0x4D, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08};
   fwrite(buffer, 1, 8, tfd->tiff);

   /* Ecriture de l'IFD */
   /* Nombre d'entrées */
   fput16b(tfd->tiff, NUM_ENTRIES);
   /* Ecriture des entrees */
   /*IMAGE_WIDTH*/
   tiff_write_entry(tfd->tiff, IMAGE_WIDTH, LONG, 1, tfd->width);

   /*IMAGE_LENGHT*/
   tiff_write_entry(tfd->tiff, IMAGE_LENGTH, LONG, 1, tfd->height);

   /*BITS_PER_SAMPLE*/
   tiff_write_entry(tfd->tiff, BITS_PER_SAMPLE, SHORT, 3, 0x9e);

   /*COMPRESSION*/
   tiff_write_entry(tfd->tiff, COMPRESSION, SHORT, 1, 1 << 16);

   /*PHOTOMETRIC_INTERPRETATION*/
   tiff_write_entry(tfd->tiff, PHOTOMETRIC_INTERPRETATION, SHORT, 1, 0x20000);

   /*STRIP_OFFSET*/
   if (nb_strips < 3)
      tiff_write_entry(tfd->tiff, STRIP_OFFSET, SHORT, 2, 0xb40174);
   else 
      tiff_write_entry(tfd->tiff, STRIP_OFFSET, LONG, 1, 0xb4);

   /*SAMPLE_PER_PIXEL*/
   tiff_write_entry(tfd->tiff, SAMPLE_PER_PIXEL, LONG, 1, 0x3);

   /*ROWS_PER_STRIP*/
   tiff_write_entry(tfd->tiff, ROWS_PER_STRIP, LONG, 1, tfd->row_per_strip);

   /*STRIP_BYTE_COUNTS*/
   tiff_write_entry(tfd->tiff, STRIP_BYTE_COUNTS, SHORT, 2, 0xc00c0);

   /*X_RESOLUTION*/
   tiff_write_entry(tfd->tiff, X_RESOLUTION, RATIONNAL, 1, 0xa4);

   /*Y_RESOLUTION*/
   tiff_write_entry(tfd->tiff, Y_RESOLUTION, RATIONNAL, 1, 0xac);

   /*RESOLUTION_UNIT*/
   tiff_write_entry(tfd->tiff, RESOLUTION_UNIT, SHORT, 1, 0x20000);

   /* OFFSET SUIVANT */
   fput32b(tfd->tiff, OFFSET_SUIV);

   /*BitsPerSamples*/
   fput16b(tfd->tiff, 0x8);
   fput16b(tfd->tiff, 0x8);
   fput16b(tfd->tiff, 0x8);

   /*XResolution*/
   fput32b(tfd->tiff, 0x64);
   fput32b(tfd->tiff, 0x1);

   /*YResolution*/
   fput32b(tfd->tiff, 0x64);
   fput32b(tfd->tiff, 0x1);
   
   if (nb_strips > 2) 
      for(uint32_t i=0; i < tfd->height; i++){
         fput16b(tfd->tiff, (0xb4+2*tfd->height)+i*strip_byte_count);
      }

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
   return 0;
}

int main(void)
{
   uint32_t RGB[8*32];
   for (uint32_t i=0; i < 8*32; i++){
      RGB[i]=0xff0000;
   }
   struct tiff_file_desc *tfd = init_tiff_file("gros_lee.tiff",8,32,8);
   for (uint32_t i = 0; i < 8*32; i++) {
      fputc ((RGB[i] >> 16) & 0xff, tfd->tiff);
      printf("Ox%x\n",(RGB[i] >> 16) & 0xff);
      fputc ((RGB[i] >> 8) & 0xff, tfd->tiff);
      printf("Ox%x\n",(RGB[i] >> 8) & 0xff);
      fputc (RGB[i] & 0xff, tfd->tiff);
      printf("0x%x\n",RGB[i] & 0xff);
}       
   close_tiff_file(tfd);

   return 0;
}
