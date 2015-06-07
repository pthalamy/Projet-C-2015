
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "utils.h"

struct bitstream {
   FILE* fp;			/* FILE Pointer sur fichier à lire */
   uint8_t buf;			/* Reste de la dernier écriture */
   uint8_t buf_len;		/* Nombre de bits dans buf */
};

struct bitstream *create_bitstream(const char *filename)
{
   /* Allocation du bitstream */
   struct bitstream *stream = smalloc (sizeof(struct bitstream));

   /* Ouverture du flux */
   stream->fp = fopen (filename, "wb");
   if (!stream->fp) {
      fprintf(stderr, "fopen: filename invalide\n");
      free (stream);
      return NULL;
   }

   stream->buf = 0;
   stream->buf_len = 0;

   return stream;
}

void write_bitstream(struct bitstream *stream, uint8_t nb_bits, uint32_t src)
{
   while (nb_bits) {
      printf ("buf: %#x | buf_len: %d\n", stream->buf, stream->buf_len);

      /*  ecriture des bits restants mis dans buf */
      if (nb_bits <= 8 - stream->buf_len) {
	 printf ("nb_bits: %d <= buf_len: %d\n", nb_bits, stream->buf_len);
	 stream->buf = (stream->buf << (stream->buf_len - nb_bits)) | ((src & BITMASK_L(nb_bits)) >> (32 - nb_bits));
	 stream->buf_len += nb_bits;
	 nb_bits = 0;
      } else {
	 printf ("nb_bits: %d > buf_len: %d\n", nb_bits, stream->buf_len);
	 /* On  charge les bits dans le buffer et on écrit au fur et à mesure  */
	 stream->buf = (stream->buf << (8 - stream->buf_len))
	    | ((src & BITMASK_L (8 - stream->buf_len)) >> (32 - 8 - stream->buf_len));
	 src = src << (8 -stream->buf_len);
	 nb_bits -= (8 -stream->buf_len);
	 stream->buf_len = 8;
	 }

      if (stream->buf_len == 8) {
	 fputc (stream->buf, stream->fp);
	 stream->buf = 0;
	 stream->buf_len = 0;
      }

   }
   printf ("\n");
}

void free_bitstream(struct bitstream *stream)
{
   if (stream) {
      fclose (stream->fp);
      free (stream);
      stream = NULL;
   }
}

int main(void)
{
   struct bitstream *stream = create_bitstream ("test.b");

   write_bitstream (stream, 8, 0xff);
   write_bitstream (stream, 32, 0x12345678);

   free_bitstream (stream);

   return 0;
}
