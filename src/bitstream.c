#include "bitstream.h"

#include <stdio.h>
#include <stdlib.h>

struct bitstream {
   FILE* fp;			/* FILE Pointer sur fichier à lire */
   char buf;			/* Reste de la dernier lecture */
   uint8_t buf_len;		/* Nombre de bits dans buf */
};

const uint8_t putmask[8] = {
   0x00, /* 0b0000 0000 */
   0x01, /* 0b0000 0001 */
   0x03, /* 0b0000 0011 */
   0x07, /* 0b0000 0111 */
   0x0f, /* 0b0000 1111 */
   0x1f, /* 0b0001 1111 */
   0x3f, /* 0b0011 1111 */
   0x7f  /* 0b0111 1111 */
};

struct bitstream *create_bitstream(const char *filename)
{
   /* Allocation du bitstream */
   struct bitstream *bs = malloc (sizeof(struct bitstream));
   if (!bs) {
      fprintf(stderr, "malloc: OUT OF MEMORY\n");
      return NULL;
   }

   /* Ouverture du flux */
   bs->fp = fopen (filename, "rb");
   if (!bs->fp) {
      fprintf(stderr, "fopen: filename invalide\n");
      free (bs);
      return NULL;
   }

   bs->buf = 0;
   bs->buf_len = 0;

   return bs;
}

bool end_of_bitstream(struct bitstream *stream)
{
   /* Si on est à la fin du fichier et que le buffer est vide */
   /* => TRUE */
   if ((fgetc (stream->fp) == EOF) && (stream->buf_len == 0)) {
      return true;
   } else {
      /* Sinon => FALSE */
      /* On retourne à la position avant lecture */
      fseek (stream->fp, -1, SEEK_CUR);
      return false;
   }
}

uint8_t read_bitstream(struct bitstream *stream,
		       uint8_t nb_bits, uint32_t *dest,
		       bool byte_stuffing)
{
   /* Utilisation des bits restants mis dans buf */
   /* if (nb_bits < bs->buf_len) */
   /*    dest =  */
   return 0;
}

bool skip_bitstream_until(struct bitstream *stream,
			  uint8_t byte)
{
   uint8_t lu;

   /* Vidage du buffer */
   stream->buf = 0;
   stream->buf_len = 0;

   /* Recherche de l'octet byte */
   while ((lu = fgetc (stream->fp)) != EOF) {
      if (lu == byte)
      /* On se positionne à l'octet précédent */
      fseek (stream->fp, -1, SEEK_CUR);
      return true;
   }

   return false;
}

void free_bitstream(struct bitstream *stream)
{
   if (stream) {
      fclose (stream->fp);
      free (stream);
      stream = NULL;
   }
}
