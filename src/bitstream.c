#include "bitstream.h"

#include <stdio.h>
#include <stdlib.h>

struct bitstream {
   FILE* fp;			/* FILE Pointer sur fichier à lire */
   char buf;			/* Reste de la dernier lecture */
   uint8_t buf_len;		/* Nombre de bits dans buf */
   char last_byte;
};

struct bitstream *create_bitstream(const char *filename)
{
   /* Allocation du bitstream */
   struct bitstream *stream = malloc (sizeof(struct bitstream));
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

#define BITMASK(n) (0xFFFFFFFF >> (32 - n))

uint8_t read_bitstream(struct bitstream *stream,
		       uint8_t nb_bits, uint32_t *dest,
		       bool byte_stuffing)
{
   *dest = 0;
   uint8_t bits_lu = 0;

   printf ("get %d bits\n", nb_bits);
   if (end_of_bitstream(stream))
      printf ("end of bitstream\n");
   else
      printf ("NOT end of bitstream\n");

   while (nb_bits && !end_of_bitstream(stream)) {
      /* Utilisation des bits restants mis dans buf */
      /* Le buffer contient plus ou autant de bits que demandé */
      if (nb_bits <= stream->buf_len) {
	 printf ("nb_bits %d <= buf_len %d \n", nb_bits, stream->buf_len);
	 printf ("buf = 0x%x \n", stream->buf);
	 *dest += (stream->buf >> (stream->buf_len - nb_bits)) & BITMASK(nb_bits);
	 stream->buf_len -= nb_bits;
	 bits_lu += nb_bits;
	 nb_bits = 0;
      } else {
	 printf ("nb_bits %d > buf_len %d \n", nb_bits, stream->buf_len);
	 /* On charge d'abord les bits du buffer  */
	 *dest += stream->buf & BITMASK(nb_bits);
	 bits_lu += stream->buf_len;
	 nb_bits -= stream->buf_len;
	 stream->buf_len = 0;

	 /* Puis on recharge le buffer avec le stream */
	 uint32_t bstuff = 0;
	 bstuff = fgetc (stream->fp);
	 printf ("0x%x ", bstuff);
	 if (stream->last_byte == 0xff && (bstuff == 0x0)) {
	       printf ("!byte_stuffing : 0x%x 0x%x\n ", stream->last_byte, bstuff);
	       bstuff = fgetc (stream->fp);
	       printf ("0x%x ", bstuff);
	 }

	 stream->last_byte = bstuff;
	 printf ("0x%x ", bstuff);

	 stream->buf = bstuff & BITMASK (8);
	 stream->buf_len = 8;

	 /* On décale pour laisser de la place aux nouveau bits */
	 *dest <<= nb_bits;
	 printf ("0x%x ", stream->buf);
      }

      printf ("\n0x%x \n", *dest);
   }

   return bits_lu;
}

void skip_bitstream_until(struct bitstream *stream,
			  uint8_t byte)
{
   printf ("skip_bitstream_until: 0x%x\n", byte);
   uint8_t lu;

   /* Vidage du buffer */
   stream->buf = 0;
   stream->buf_len = 0;

   /* Recherche de l'octet byte */
   while ((lu = fgetc (stream->fp)) != EOF) {
      if (lu == byte)
      /* On se positionne à l'octet précédent */
      fseek (stream->fp, -1, SEEK_CUR);
   }
}

void free_bitstream(struct bitstream *stream)
{
   if (stream) {
      fclose (stream->fp);
      free (stream);
      stream = NULL;
   }
}
