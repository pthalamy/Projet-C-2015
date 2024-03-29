
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

#define NUM_COMPOSANTES 3

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

   jfd->sfh[0] = sfh;
   jfd->sfh[1] = 1;
   jfd->sfh[2] = 1;

   jfd->sfv[0] = sfv;
   jfd->sfv[1] = 1;
   jfd->sfv[2] = 1;

   jfd->N = NUM_COMPOSANTES;

   jfd->iq[0] = 0;
   jfd->iq[1] = 1;
   jfd->iq[2] = 1;

   jfd->ic[0] = 1;
   jfd->ic[1] = 2;
   jfd->ic[2] = 3;

   jfd->ih_dc[0] = 0;
   jfd->ih_dc[1] = 1;
   jfd->ih_dc[2] = 1;

   jfd->ih_ac[0] = 2;
   jfd->ih_ac[1] = 3;
   jfd->ih_ac[2] = 3;

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

   for (uint32_t i = 0; i < 2; i++) {
      write_bitstream (jfd->stream, 4, 0); /* Precision 0 */
      write_bitstream (jfd->stream, 4, i); /* Indice i */
      for (uint8_t j = 0; j < 64; j++) {
	 /* printf ("%#x ", table_quantif[i][j]); */
	 write_bitstream (jfd->stream, 8, table_quantif[i][j]);
      }
      /* printf ("\n"); */
   }

   /* exit (1); */
}

void export_SOF0(struct jpeg_file_desc *jfd)
{
   printf ("-> SOF0: \n");
   write_bitstream (jfd->stream, 16, SOF0);

   write_bitstream (jfd->stream, 16, 17); /* Longueur */
   write_bitstream (jfd->stream, 8, 8);  /* Precision */

   write_bitstream (jfd->stream, 16, jfd->imageHeight);
   write_bitstream (jfd->stream, 16, jfd->imageWidth);

   write_bitstream (jfd->stream, 8, jfd->N);

   for(uint8_t i = 0; i < jfd->N; i++) {
      write_bitstream (jfd->stream, 8, jfd->ic[i]);
      write_bitstream (jfd->stream, 4, jfd->sfh[i]);
      write_bitstream (jfd->stream, 4, jfd->sfv[i]);
      write_bitstream (jfd->stream, 8, jfd->iq[i]);
   }
}


void export_DHT(struct jpeg_file_desc *jfd,
		const uint8_t codeLengths[16],
		const uint8_t *symboles,
		const uint8_t nbSym, uint8_t type, uint8_t indice)
{
   printf ("-> DHT: \n");
   write_bitstream (jfd->stream, 16, DHT);

   uint16_t longueur = 2 + 1 + 16 + nbSym;
   printf ("nbSym : %d\n", nbSym);
   printf ("longueur_DHT : %d\n", longueur);
   write_bitstream (jfd->stream, 16, longueur);

   write_bitstream (jfd->stream, 3, 0); /* 3 zeros */
   write_bitstream (jfd->stream, 1, type); /* DC = 0; AC = 1 */
   write_bitstream (jfd->stream, 4, indice); /* ic */

   for (uint8_t i = 0; i < 16; i++)
      write_bitstream (jfd->stream, 8, codeLengths[i]);

   for (uint8_t i = 0; i < nbSym; i++)
      write_bitstream (jfd->stream, 8, symboles[i]);
}

void export_SOS_Header(struct jpeg_file_desc *jfd)
{
   printf ("-> SOS: \n");
   write_bitstream (jfd->stream, 16, SOS);

   /* 2 * N = 3 + 6 */
   write_bitstream (jfd->stream, 16, 12); /* longueur */

   write_bitstream (jfd->stream, 8, jfd->N);

   for (uint8_t i = 0; i < jfd->N; i++){
      /* printf ("indice: %d\n", i); */
      write_bitstream (jfd->stream, 8, jfd->ic[i]);
      /* printf ("ic: %d\n", jfd->ic[i]); */
      write_bitstream (jfd->stream, 4, jfd->ih_dc[i]);
      /* printf ("dc: %d\n", jfd->ih_dc[i]); */
      write_bitstream (jfd->stream, 4, jfd->ih_ac[i]);
      /* printf ("ac: %d\n", jfd->ih_ac[i]); */
   }

   write_bitstream (jfd->stream, 24, 0); /* unused */
}

void close_jpeg_file(struct jpeg_file_desc *jfd)
{
   if (jfd) {
      printf ("-> EOI: \n");
      write_bitstream(jfd->stream, 16, EOI);
      free_bitstream(jfd->stream);
      free (jfd);
      jfd = NULL;
   }
}
