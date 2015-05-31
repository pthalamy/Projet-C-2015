
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct bitstream {
   FILE* fp;			/* FILE Pointer sur fichier à lire */
   uint8_t buf;			/* Reste de la dernier lecture */
   uint8_t buf_len;		/* Nombre de bits dans buf */
   uint8_t last_byte;
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

#define BITMASK(n) (0xFFFFFFFF >> (32 - n))

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
	 printf ("nb_bits %d <= buf_len %d \n", nb_bits, stream->buf_len);
	 printf ("buf = 0x%x \n", stream->buf);
	 *dest += (stream->buf >> (stream->buf_len - nb_bits)) & BITMASK(nb_bits);
	 stream->buf &= stream->buf & BITMASK (stream->buf_len - nb_bits);
	 stream->buf_len -= nb_bits;
	 bits_lu += nb_bits;
	 nb_bits = 0;
      } else {
	 printf ("nb_bits %d > buf_len %d \n", nb_bits, stream->buf_len);
	 /* On charge d'abord les bits du buffer  */
	 if (stream->buf_len) {
	    *dest += (stream->buf << (nb_bits-stream->buf_len));
	    printf ("dest2: 0x%x  = (0x%x << %d)\n", *dest, stream->buf,
		    nb_bits, BITMASK(nb_bits));
	    bits_lu += stream->buf_len;
	    nb_bits -= stream->buf_len;
	    stream->buf = 0;
	    stream->buf_len = 0;
	 }

	 /* Puis on recharge le buffer avec le stream */
	 uint32_t bstuff = 0;
	 bstuff = fgetc (stream->fp);
	 printf ("read: 0x%x \n", bstuff);
	 if (stream->last_byte == 0xff && (bstuff == 0x0) && byte_stuffing) {
	       printf ("!byte_stuffing : 0x%x 0x%x\n ", stream->last_byte, bstuff);
	       bstuff = fgetc (stream->fp);
	       printf ("new read: 0x%x ", bstuff);
	 }

	 stream->last_byte = bstuff;
	 /* printf ("0x%x ", bstuff); */

	 stream->buf = bstuff & BITMASK (8);
	 stream->buf_len = 8;

	 /* On décale pour laisser de la place aux nouveau bits */
	 /* *dest <<= nb_bits; */
	 /* printf ("dest3: 0x%x = *dest2 <<= %d\n", *dest, nb_bits); */
	 /* printf ("0x%x ", stream->buf); */
      }
   }
   printf ("\ndest4: 0x%x\n", *dest);
   printf ("====\n", *dest);

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
      if (lu == byte) {
	 /* On se positionne à l'octet précédent */
	 fseek (stream->fp, -1, SEEK_CUR);
	 return;
      }
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

int main(void)
{
   struct bitstream *bs = create_bitstream ("babilon_amphi.jpg");
   uint32_t read = 0;

   /* Lecture des 4 premiers octets */
   read_bitstream (bs, 8, &read, false);
   read_bitstream (bs, 8, &read, false);
   read_bitstream (bs, 8, &read, false);
   read_bitstream (bs, 8, &read, false);
   /* Lecture de deux octets d'un coup */
   read_bitstream (bs, 16, &read, false);
   read_bitstream (bs, 16, &read, false);
   /* Lecture de trois octets d'un coup */
   read_bitstream (bs, 24, &read, false);
   /* Lecture de quatre octets d'un coup */
   read_bitstream (bs, 32, &read, false);

   /* Avance jusqu'à 0xffc4 */
   skip_bitstream_until (bs, 0xc4);
   /* Lecture de l'octet 0xc4 */
   read_bitstream (bs, 8, &read, false);
   /* Avance jusqu'à 0x0613 */
   skip_bitstream_until (bs, 0x13);
   /* Lecture de l'octet 0x13 */
   read_bitstream (bs, 8, &read, false);

   /* 5161 0722 7114 3281 91a1 0823 42b1 */
   read_bitstream (bs, 4, &read, false); /* doit être 0x5 */
   read_bitstream (bs, 4, &read, false); /* doit être 0x1 */
   read_bitstream (bs, 2, &read, false); /* doit être 0x1 */
   read_bitstream (bs, 1, &read, false); /* doit être 0x1 */
   read_bitstream (bs, 1, &read, false); /* doit être 0x0 */
   read_bitstream (bs, 4, &read, false); /* doit être 0x0 */
   read_bitstream (bs, 3, &read, false); /* doit être 0x3 */



   free_bitstream (bs);

   return 0;
}
