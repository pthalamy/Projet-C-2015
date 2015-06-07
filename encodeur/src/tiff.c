
#include "tiff.h"
#include "utils.h"

#include <stdio.h>
#include <stdint.h>

enum endianness {
   LE = 0x4949,
   BE = 0x4d4d
};

struct tiff_file_desc {
   enum endianness e_ness;
   FILE *tiff;
};

void read_nbytes (FILE *fp, enum endianness en, size_t nbytes, void *dest)
{
   fread (dest, nbytes, 1, fp);

   /* Passage à la bonne endianness */
   switch (nbytes) {
   case 1:
      return;
   case 2:
      if (en == LE)
	 *(uint16_t*)dest = le16_to_cpu (*(uint16_t*)dest);
      else
	 *(uint16_t*)dest = be16_to_cpu (*(uint16_t*)dest);
      return;
   case 3:
            fprintf(stderr, "tiff erreur: 3 octets d'un coup ?\n");
	    exit (EXIT_FAILURE);
   case 4:
      if (en == LE)
	 *(uint32_t*)dest = le32_to_cpu (*(uint32_t*)dest);
      else
	 *(uint32_t*)dest = be32_to_cpu (*(uint32_t*)dest);
      return;
   default:
      fprintf(stderr, "tiff erreur: Vous ne pouvez lire plus de 4 octets d'un coup \n");
      exit (EXIT_FAILURE);
   }

}

struct tiff_file_desc *create_tfd_and_read_header (const char *file_name)
{
   struct tiff_file_desc *tfd = smalloc (sizeof(struct tiff_file_desc));
   tfd->tiff = fopen(file_name,"rb");
   if (!tfd->tiff)
      return NULL;

   /* HEADER */

   /* Lecture de l'endianness */
   fread (&tfd->e_ness, 2, 1, tfd->tiff);
   printf ("endianness: %#x\n", tfd->e_ness);

   /* Lecture de l'identificateur TIFF */
   uint16_t tiff_id;
   read_nbytes (tfd->tiff, tfd->e_ness, 2, &tiff_id);
   printf ("id: %#x\n", tiff_id);
   if (tiff_id != 42) {
      fprintf(stderr, "tiff erreur: Identificateur TIFF différent de 42, le fichier n'est pas valide. \n");
      exit (EXIT_FAILURE);
   }

   /* Offset premier IFD */
   uint16_t ifd_offset;
   read_nbytes (tfd->tiff, tfd->e_ness, 4, &ifd_offset);
   printf ("ifd_offset: %#x\n", ifd_offset);

   /* FIN DU HEADER */

   /* Positionnement sur la première IFD */
   fseek (tfd->tiff, ifd_offset, SEEK_SET);

   return tfd;
}

void free_tfd (struct tiff_file_desc *tfd)
{
   if (tfd) {
      fclose (tfd->tiff);
      free (tfd);
      tfd = NULL;
   }
}
