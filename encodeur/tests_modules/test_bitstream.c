
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
   /* printf ("preshift_src: %#x\n", src); */
   src = src << (32 - nb_bits);
   /* printf ("src: %#x\n", src); */

   while (nb_bits) {
      /* printf ("buf: %#x | buf_len: %d\n", stream->buf, stream->buf_len); */

      if (!stream->buf_len) {
	 /* printf ("nb_bits: %d <= buf_len: %d\n", nb_bits, stream->buf_len); */
	 if (nb_bits <= 8) {
	    stream->buf = (src & BITMASK_L(nb_bits)) >> 24;
	    /* printf ("%#x = %#x & %#x\n", stream->buf, src, BITMASK_L (nb_bits)); */
	    stream->buf_len += nb_bits;
	    nb_bits = 0;
	 } else {
	    stream->buf = (src & BITMASK_L(8)) >> 24 ;
	    /* printf ("%#x = %#x & %#x\n", stream->buf, src, BITMASK_L (nb_bits)); */
	    stream->buf_len += 8;
	    src = src << 8;
	    nb_bits -= 8;
	 }
      /*  Ajout aux bits restants mis dans buf */
      } else if (nb_bits <= 8 - stream->buf_len) {
	 /* printf ("nb_bits: %d <= buf_len: %d\n", nb_bits, stream->buf_len); */
	 stream->buf |= ((src & BITMASK_L(nb_bits)) >> (24 + stream->buf_len));
	 stream->buf_len += nb_bits;
	 nb_bits = 0;
      } else {
	 /* printf ("nb_bits: %d > buf_len: %d\n", nb_bits, stream->buf_len); */
	 /* On  charge les bits dans le buffer et on écrit au fur et à mesure  */
	 stream->buf |= ( (src & BITMASK_L(8 - stream->buf_len))  >> (24  + stream->buf_len) );
	 /* printf ("src: %#x\n", src); */
	 src = src << (8 - stream->buf_len);
	 /* printf ("src: %#x\n", src); */
	 nb_bits -= (8 -stream->buf_len);
	 stream->buf_len = 8;
      }

      /* printf ("buf: %#x | buf_len: %d\n", stream->buf, stream->buf_len); */

      if (stream->buf_len == 8) {
	 fputc (stream->buf, stream->fp);
	 stream->buf = 0;
	 stream->buf_len = 0;
      }

   }
   /* printf ("\n"); */
}

void free_bitstream(struct bitstream *stream)
{
   if (stream) {
      /* écriture des bits restants avec padding */
      if (stream->buf_len) {
	 /* printf ("buf: %#x | buf_len: %d\n", stream->buf, stream->buf_len); */
	 fputc (stream->buf, stream->fp);
	 stream->buf_len  = 0;
      }

      fclose (stream->fp);
      free (stream);
      stream = NULL;
   }
}

int main(void)
{
   struct bitstream *stream = create_bitstream ("test.b");

   write_bitstream (stream, 13, 0x5bc6);
   write_bitstream (stream, 3, 0x7);
   write_bitstream (stream, 5, 0x12);
   write_bitstream (stream, 32, 0x12345678);
   write_bitstream (stream, 3, 0x07);
   write_bitstream (stream, 1, 0x1);
   write_bitstream (stream, 4, 0xe);
   write_bitstream (stream, 2, 0x1);
   write_bitstream (stream, 1, 0x1);
   write_bitstream (stream, 7, 0x74);
   write_bitstream (stream, 16, 0x1234);

   free_bitstream (stream);

   return 0;
}
