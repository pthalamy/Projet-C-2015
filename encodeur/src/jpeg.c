
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
   FILE *jpeg;
   uint32_t imageWidth;
   uint32_t imageHeight;
   uint8_t sfh;
   uint8_t sfv;
};

struct jpeg_file_desc *init_jpeg_file (struct bitstream *stream, const char *file_name,
				       uint32_t width,
				       uint32_t height,
				       uint8_t sfh,
				       uint8_t sfv)
{
   struct jpeg_file_desc *jfd = smalloc (sizeof(struct jpeg_file_desc));
   jfd->jpeg = fopen(file_name,"wb");
   if (!jfd->jpeg)
      return NULL;

   jfd->imageWidth = width;
   jfd->imageHeight = height;
   jfd->sfh = sfh;
   jfd->sfv = sfv;

   /* Écriture du marqueur SOI */
   write_bitstream (stream, 16, SOI);

   /* APP0 */
   printf ("-> APP0: \n");
   write_bitstream (stream, 16, APP0);

   write_bitstream (stream, 16, 16); /* Longueur APP0 */

   /* ecriture  de JFIF */
   write_bitstream (stream, 8, 'J');
   write_bitstream (stream, 8, 'F');
   write_bitstream (stream, 8, 'I');
   write_bitstream (stream, 8, 'F');
   write_bitstream (stream, 8, '\0');

   /* Écriture des données additionnelles */
   write_bitstream(stream, 16, 257); /* JFIF Version */
   write_bitstream(stream, 8, 0); /* Density units */
   write_bitstream(stream, 16, 1); /* X Density */
   write_bitstream(stream, 16, 1); /* Y Density */
   write_bitstream(stream, 8, 0);  /* Thumbnail width */
   write_bitstream(stream, 8, 0);  /* Thumbnail height */

   return jfd;
}

void close_jpeg_file(struct bitstream *stream, struct jpeg_file_desc *jfd)
{
   if (jfd) {
      write_bitstream(stream, 16, EOI);
      fclose (jfd->jpeg);
      free (jfd);
      jfd = NULL;
   }
}
