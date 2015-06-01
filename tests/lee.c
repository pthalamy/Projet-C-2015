#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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

/* CONSTANTS */
#define NUM_ENTRIES 0x000c
#define OFFSET_FIRST_IFD 0x00000008
#define OFFSET_SUIV 0x0000
#define BPS_OFFSET 0x0000009e
#define NO_COMPRESSION 0x10000
#define RGB_IMAGE 0x20000
#define BIG_ENDIAN 0x4d4d
#define TIFF_ID 0x002a
#define SPP_3 0x3
#define X_RES_OFFSET 0xa4
#define Y_RES_OFFSET 0xac
#define RES_UNIT_CM 0x20000
#define SBC_OFFSET 0xb4
#define FIRST_STRIP_OFFSET 0x00b4

#define BYTES_PER_PIXEL 3
#define BYTE_SIZE 8
#define PIXELS_PER_CM 100

/* MACROS */
#define NEXT_STRIP_OFFSET(base,  i, size) ((base) + (i) * (size))

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

void tiff_write_header(struct tiff_file_desc *tfd)
{
   fput16b(tfd->tiff, BIG_ENDIAN);
   fput16b(tfd->tiff, TIFF_ID);
   fput32b(tfd->tiff, OFFSET_FIRST_IFD);
}

void tiff_write_entry(FILE *fp, uint16_t tag, uint16_t type, uint32_t nb_val, uint32_t val)
{
   fput16b (fp, tag);
   fput16b (fp, type);
   fput32b (fp, nb_val);
   fput32b (fp, val);
}

void tiff_write_ifd(struct tiff_file_desc *tfd)
{
   /* Calcul du nombre de strip */
   uint32_t nb_strips = tfd->height / tfd->row_per_strip;
   printf ("nb_strips = %d\n", nb_strips);

   /*calcul de la taille (en octets) des lignes*/
   uint32_t strip_byte_count = BYTES_PER_PIXEL * tfd->row_per_strip * tfd->width;
   printf ("strip_byte_count = %d\n", strip_byte_count);

   /* /\* Longueur de la dernière strip *\/ */
   /* uint32_t last_strip_length = (tfd->width * tfd->height * BYTES_PER_PIXEL) */
   /*    - (strip_byte_count*(tfd->height / tfd->row_per_strip)); */
   /* /\* if (!last_strip_length)  *\/last_strip_length = strip_byte_count; */
   /* printf ("last_strip_length = %d\n", last_strip_length); */

   /* Nombre d'entrées */
   fput16b(tfd->tiff, NUM_ENTRIES);

   /* Ecriture des entrees */
   /* Entrées constantes */
   tiff_write_entry(tfd->tiff, BITS_PER_SAMPLE, SHORT, 3, BPS_OFFSET);
   tiff_write_entry(tfd->tiff, COMPRESSION, SHORT, 1, NO_COMPRESSION);
   tiff_write_entry(tfd->tiff, PHOTOMETRIC_INTERPRETATION, SHORT, 1, RGB_IMAGE);
   tiff_write_entry(tfd->tiff, SAMPLE_PER_PIXEL, LONG, 1, SPP_3);
   tiff_write_entry(tfd->tiff, X_RESOLUTION, RATIONNAL, 1, X_RES_OFFSET);
   tiff_write_entry(tfd->tiff, Y_RESOLUTION, RATIONNAL, 1, Y_RES_OFFSET);
   tiff_write_entry(tfd->tiff, RESOLUTION_UNIT, SHORT, 1, RES_UNIT_CM);

   /* Traitement des entrées non constantes */
   tiff_write_entry(tfd->tiff, IMAGE_WIDTH, LONG, 1, tfd->width);
   tiff_write_entry(tfd->tiff, IMAGE_LENGTH, LONG, 1, tfd->height);
   tiff_write_entry(tfd->tiff, ROWS_PER_STRIP, LONG, 1, tfd->row_per_strip);

   if (nb_strips == 1) {
      /* tiff_write_entry(tfd->tiff, STRIP_BYTE_COUNTS, SHORT, 1, last_strip_length << 16); */
      tiff_write_entry(tfd->tiff, STRIP_BYTE_COUNTS, SHORT, 1, (strip_byte_count << 16));
      tiff_write_entry(tfd->tiff, STRIP_OFFSET, SHORT, 1, FIRST_STRIP_OFFSET << 16);
   } else if (nb_strips == 2) {
      /* tiff_write_entry(tfd->tiff, STRIP_BYTE_COUNTS, SHORT, 2, (strip_byte_count << 16) | last_strip_length); */
      tiff_write_entry(tfd->tiff, STRIP_BYTE_COUNTS, SHORT, 2, (strip_byte_count << 16) | strip_byte_count);
      tiff_write_entry(tfd->tiff, STRIP_OFFSET, SHORT, 2,
		       (FIRST_STRIP_OFFSET << 16) | (FIRST_STRIP_OFFSET + strip_byte_count));
   } else {
      tiff_write_entry(tfd->tiff, STRIP_BYTE_COUNTS, LONG, nb_strips, SBC_OFFSET);
      tiff_write_entry(tfd->tiff, STRIP_OFFSET, LONG, nb_strips, SBC_OFFSET + (nb_strips * 4));
   }

   /* Un seul IFD, OFFSET_SUIV sur 0x0 */
   fput32b(tfd->tiff, OFFSET_SUIV);

   /*BitsPerSamples*/
   fput16b(tfd->tiff, BYTE_SIZE);
   fput16b(tfd->tiff, BYTE_SIZE);
   fput16b(tfd->tiff, BYTE_SIZE);

   /*XResolution*/
   fput32b(tfd->tiff, PIXELS_PER_CM);
   fput32b(tfd->tiff, 1);

   /*YResolution*/
   fput32b(tfd->tiff, PIXELS_PER_CM);
   fput32b(tfd->tiff, 1);

   if (nb_strips > 2) {
      printf ("%d strips: \n", nb_strips);
      printf ("longueurs: ");
      /* Longueur des strips */
      for (uint32_t i = 0; i < nb_strips ; i++) {
	 printf ("%d ", strip_byte_count);
	 fput32b (tfd->tiff, strip_byte_count);
      }
      /* fput32b (tfd->tiff, last_strip_length);      /\* Derniere strip de longueur plus courte *\/ */
      /* printf ("%d ", last_strip_length); */

      /* Offsets des strips */
      uint32_t first_strip = SBC_OFFSET + (nb_strips * 8);
      printf ("\noffsets: ");
      for (uint32_t i = 0; i < nb_strips; i++) {
	 printf ("0x%x ", NEXT_STRIP_OFFSET(first_strip, i, strip_byte_count));
	 fput32b (tfd->tiff, NEXT_STRIP_OFFSET(first_strip, i, strip_byte_count));
      }
   }
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
   printf ("tfd->width = %d\n",tfd->width);
   /*nombre de lignes de l'image*/
   tfd->height = height;
   printf ("tfd->height = %d\n",tfd->height);
   /*hauteur (en pixels) es lignes TIFF*/
   tfd->row_per_strip=row_per_strip;
   printf ("tfd->row_per_strip = %d\n",tfd->row_per_strip);

   /************************* Ecriture du header *************************/
   tiff_write_header (tfd);

   /************************ Ecriture de l'IFD ************************/

   tiff_write_ifd (tfd);

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
   uint32_t RGB[64];
   for (uint32_t i=0; i < 64; i++){
      RGB[i]=0xff0000;
   }

   struct tiff_file_desc *tfd = init_tiff_file("gros_lee.tiff", 43, 17, 8);

   for (uint32_t j = 0; j < 5; j++) {
      for (uint32_t i = 0; i < 64; i++) {
	 fputc ((RGB[i] >> 16) & 0xff, tfd->tiff);
	 fputc ((RGB[i] >> 8) & 0xff, tfd->tiff);
	 fputc (RGB[i] & 0xff, tfd->tiff);
      }
   }

   for (uint32_t i = 0; i < 24; i++) {
      fputc ((RGB[i] >> 16) & 0xff, tfd->tiff);
      fputc ((RGB[i] >> 8) & 0xff, tfd->tiff);
      fputc (RGB[i] & 0xff, tfd->tiff);
   }

   for (uint32_t i = 0; i < 5*64+24; i++) {
      fputc ((0x00ff0000 >> 16) & 0xff, tfd->tiff);
      fputc ((0x00ff0000 >> 8) & 0xff, tfd->tiff);
      fputc (0x00ff0000 & 0xff, tfd->tiff);
   }

   printf ("\n");
   close_tiff_file(tfd);

   return 0;
}
