#include "bitstream.h"

#include <stdio.h>
#include <stdlib.h>

void check_alloc_bitstream(void* ptr)
{
   if (!ptr) {
      fprintf (stderr, "alloc error: OUT OF MEMORY\n");
   }
}

struct bitstream {
   FILE* fp;			/* FILE Pointer sur fichier à lire */
   uint8_t buf;			/* Reste de la dernier lecture */
   uint8_t buf_len;		/* Nombre de bits dans buf */
   uint8_t last_byte;		/* Valeur du dernier octet lu */
};

struct bitstream *create_bitstream(const char *filename)
{
   /* Allocation du bitstream */
   struct bitstream *stream = malloc (sizeof(struct bitstream));
   check_alloc_bitstream (stream);
   if (!stream) {
      fprintf(stderr, "malloc: OUT OF MEMORY\n");
      return NULL;
   }

   /* Ouverture du flux */
   stream->fp = fopen (filename, "rb");
   if (!stream->fp) {
      fprintf(stderr, "fopen: filename invalide\n");
      free (stream);
      return NULL;
   }

   stream->buf = 0;
   stream->buf_len = 0;
   stream->last_byte = 0;

   return stream;
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

#define BITMASK(n) (0xFFFFFFFF >> (32 - (n)))

uint8_t read_bitstream(struct bitstream *stream,
		       uint8_t nb_bits, uint32_t *dest,
		       bool byte_stuffing)
{
   *dest = 0;
   uint8_t bits_lu = 0;

   while (nb_bits && !end_of_bitstream(stream)) {
      /* Utilisation des bits restants mis dans buf */
      /* Le buffer contient plus ou autant de bits que demandé */
      if (nb_bits <= stream->buf_len) {
	 *dest += (stream->buf >> (stream->buf_len - nb_bits)) & BITMASK(nb_bits);
	 stream->buf &= BITMASK (stream->buf_len - nb_bits);
	 stream->buf_len -= nb_bits;
	 bits_lu += nb_bits;
	 nb_bits = 0;
      } else {
	 /* On charge d'abord les bits du buffer  */
	 if (stream->buf_len) {
	    *dest += (stream->buf << (nb_bits - stream->buf_len));
	    bits_lu += stream->buf_len;
	    nb_bits -= stream->buf_len;
	    stream->buf = 0;
	    stream->buf_len = 0;
	 }

	 /* Puis on recharge le buffer avec le stream */
	 uint32_t bstuff = 0;
	 bstuff = fgetc (stream->fp);
	 if (stream->last_byte == 0xff && (bstuff == 0x0) && byte_stuffing) {
	       bstuff = fgetc (stream->fp);
	 }

	 stream->last_byte = bstuff;
	 stream->buf = bstuff & BITMASK (8);
	 stream->buf_len = 8;
      }
   }

   return bits_lu;
}

void skip_bitstream_until(struct bitstream *stream,
			  uint8_t byte)
{
   char lu;

   /* Vidage du buffer */
   stream->buf = 0;
   stream->buf_len = 0;

   /* Recherche de l'octet byte */
   while ((lu = fgetc (stream->fp)) != EOF) {
      if (lu == byte) {
	 /* On se positionne à l'octet précédent */
	 fseek (stream->fp, -1, SEEK_CUR);
	 return;
      }
   }
   /* SHOULD NOT BE NECESSARY */
   fseek (stream->fp, -1, SEEK_CUR);
}

void free_bitstream(struct bitstream *stream)
{
   if (stream) {
      fclose (stream->fp);
      free (stream);
      stream = NULL;
   }
}
