
#include "jpeg.h"

#include "utils.h"

#include <stdint.h>
#include <stdio.h>

#define APP0 0xffe0
#define COM 0xfffe
#define DQT 0xffdb
#define SOF0 0xffc0
#define DHT 0xffc4
#define SOS 0xffda
#define EOI 0xffd9
#define SOI 0xffd8

struct jpeg_file_desc {
   struct bitstream *stream;
   uint32_t imageWidth;
   uint32_t imageHeight;
   uint8_t sfh;
   uint8_t sfv;
};

struct jpeg_file_desc *init_jpeg_file (const char* output_name,
				       uint32_t width,
				       uint32_t height,
				       uint8_t sfh,
				       uint8_t sfv)
{
   struct jpeg_file_desc *jfd = smalloc (sizeof(struct jpeg_file_desc));

   /* Initialisation du bitstream */
   jfd->stream = create_bitstream(output_name);
   if (!jfd->stream) {
      fprintf(stderr, "Impossible de créer le bitstream.\n");
      exit (EXIT_FAILURE);
   }

   jfd->imageWidth = width;
   jfd->imageHeight = height;
   jfd->sfh = sfh;
   jfd->sfv = sfv;

   /* Écriture du marqueur SOI */
   write_bitstream (jfd->stream, 16, SOI);

   /* APP0 */
   printf ("-> APP0: \n");
   write_bitstream (jfd->stream, 16, APP0);

   write_bitstream (jfd->stream, 16, 16); /* Longueur APP0 */

   /* ecriture  de JFIF */
   write_bitstream (jfd->stream, 8, 'J');
   write_bitstream (jfd->stream, 8, 'F');
   write_bitstream (jfd->stream, 8, 'I');
   write_bitstream (jfd->stream, 8, 'F');
   write_bitstream (jfd->stream, 8, '\0');

   /* Écriture des données additionnelles */
   write_bitstream(jfd->stream, 16, 257); /* JFIF Version */
   write_bitstream(jfd->stream, 8, 0); /* Density units */
   write_bitstream(jfd->stream, 16, 1); /* X Density */
   write_bitstream(jfd->stream, 16, 1); /* Y Density */
   write_bitstream(jfd->stream, 8, 0);  /* Thumbnail width */
   write_bitstream(jfd->stream, 8, 0);  /* Thumbnail height */

   return jfd;
}

void export_DQT(struct jpeg_file_desc *jfd,
		const uint8_t table_quantif[2][64])
{
   printf ("-> DQT: \n");
   write_bitstream (jfd->stream, 16, DQT);

   /* 2 tables (128) + 2 * (1) + champ longueur (2) = 132 */
   write_bitstream (jfd->stream, 16, 132);

   write_bitstream (jfd->stream, 4, 0); /* Precision 0 */
   write_bitstream (jfd->stream, 4, 0); /* Indice 0 */
   write_bitstream (jfd->stream, 4, 0); /* Precision 0 */
   write_bitstream (jfd->stream, 4, 1); /* Indice 1 */

   /* 2 Tables de quantif */
   for (uint32_t i = 0; i < 2; i++) {
      for (uint8_t j = 0; j < 64; j++) {
	 write_bitstream (jfd->stream, 8, table_quantif[i][j]);
      }
   }

}

void close_jpeg_file(struct jpeg_file_desc *jfd)
{
   if (jfd) {
      write_bitstream(jfd->stream, 16, EOI);
      free_bitstream(jfd->stream);
      free (jfd);
      jfd = NULL;
   }
}
