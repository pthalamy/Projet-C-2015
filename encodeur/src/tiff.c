
#include "tiff.h"
#include "utils.h"

#include <stdio.h>

/* TYPES DE DONNEES */
#define SHORT							 0x0003
#define LONG	 						 0x0004
#define RATIONNAL 	 					 0x0005

/* TAGS */
#define IMAGEWIDTH 	 				 0x0100
#define IMAGELENGTH 	 				 0x0101
#define BITS_PER_SAMPLE 	 			 0x0102
#define COMPRESSION  	 			         0x0103
#define PHOTOMETRIC_INTERPRETATION       0x0106
#define STRIP_OFFSET  	 			         0x0111
#define SAMPLE_PER_PIXEL  	 			 0x0115
#define ROWSPERSTRIP  	 			 0x0116
#define STRIP_BYTE_COUNTS  	 		         0x0117
#define X_RESOLUTION  	 			         0x011a
#define Y_RESOLUTION  	 			         0x011b
#define RESOLUTION_UNIT  	 			 0x0128

/* CONSTANTS */
#define NUM_ENTRIES 0x000c
#define OFFSET_FIRST_IFD 0x00000008
#define OFFSET_SUIV 0x0000
#define BPS_OFFSET 0x0000009e
#define NO_COMPRESSION 0x10000
#define RGB_IMAGE 0x20000
#define TIFF_ID 0x002a
#define SPP_3 0x3
#define X_RES_OFFSET 0xa4
#define Y_RES_OFFSET 0xac
#define RES_UNIT_CM 0x20000
#define SBC_OFFSET 0xb4
#define FIRST_STRIP_OFFSET 0x00b4

#define BYTES_PER_PIXEL 3
#define BYTE_SIZE 8
#define PIXELS_PER_CM 100

enum endianness {
   LE = 0x4949,
   BE = 0x4d4d
};

struct tiff_file_desc {
   enum endianness en;
   FILE *tiff;
   uint32_t imageWidth;
   uint32_t imageLength;
   uint32_t rowsPerStrip;
   uint32_t sbcOffset;
   uint32_t soOffset;
   uint32_t **imageScan;
   uint32_t nbStrips;
   uint32_t *stripByteCounts;
   uint32_t *stripOffsets;
   uint32_t scanWidth;
   uint32_t scanHeight;
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
      if (en == LE)
	 *(uint32_t*)dest = le32_to_cpu (*(uint32_t*)dest);
      else
	 *(uint32_t*)dest = be32_to_cpu (*(uint32_t*)dest) >> 8;
      return;
   case 4:
      if (en == LE)
	 *(uint32_t*)dest = le32_to_cpu (*(uint32_t*)dest);
      else
	 *(uint32_t*)dest = be32_to_cpu (*(uint32_t*)dest);
      return;
   default:
      fprintf(stderr, "TIFF erreur: Vous ne pouvez lire plus de 4 octets d'un coup \n");
      exit (EXIT_FAILURE);
   }

}

void read_value_of_type (FILE *fp, enum endianness en, uint16_t type, uint32_t *dest)
{
   if (type == SHORT)
      read_nbytes (fp, en, 2, dest);
   else if (type == LONG)
      read_nbytes (fp, en, 4, dest);
   else {
      fprintf(stderr, "TIFF erreur: Tentative de lecture d'un type non supporté "
	      "par la fonction read_value_of_type \n");
      exit (EXIT_FAILURE);
   }
}

void read_ifd_entry(FILE *fp, enum endianness en, uint16_t *tag,
		    uint16_t *type, uint32_t *nb_val, uint32_t *val)
{
   read_nbytes (fp, en, 2, tag);
   read_nbytes (fp, en, 2, type);
   read_nbytes (fp, en, 4, nb_val);
   read_nbytes (fp, en, 4, val);
}

struct tiff_file_desc *create_tfd_and_read_header (const char *file_name)
{
   struct tiff_file_desc *tfd = smalloc (sizeof(struct tiff_file_desc));
   tfd->tiff = fopen(file_name,"rb");
   if (!tfd->tiff)
      return NULL;

   /* Initialize attributes to default values */
   tfd->en = 0;
   tfd->imageWidth = 0;
   tfd->imageLength = 0;
   tfd->rowsPerStrip = 0;
   tfd->sbcOffset = 0;
   tfd->soOffset = 0;

   /* HEADER */

   /* Lecture de l'endianness */
   fread (&tfd->en, 2, 1, tfd->tiff);
   /* printf ("\tendianness: %#x\n", tfd->en); */

   /* Lecture de l'identificateur TIFF */
   uint16_t tiff_id;
   read_nbytes (tfd->tiff, tfd->en, 2, &tiff_id);
   /* printf ("\tid: %#x\n", tiff_id); */
   if (tiff_id != 42) {
      fprintf(stderr, "TIFF erreur: Identificateur TIFF différent de 42, le fichier n'est pas valide. \n");
      exit (EXIT_FAILURE);
   }

   /* Offset premier IFD */
   uint16_t ifd_offset;
   read_nbytes (tfd->tiff, tfd->en, 4, &ifd_offset);
   /* printf ("\tifd_offset: %#x\n", ifd_offset); */

   /* FIN DU HEADER */

   /* Positionnement sur la première IFD */
   fseek (tfd->tiff, ifd_offset, SEEK_SET);

   return tfd;
}

void read_TIFF_ifd(struct tiff_file_desc *tfd)
{
   /* Nombre d'entrées dans l'IFD */
   uint16_t nb_entries;
   read_nbytes (tfd->tiff, tfd->en, 2, &nb_entries);
   /* printf ("\tifd_entries: %#x\n", nb_entries); */

   /* Lecture des entrées ifd */
   uint16_t tag = 0;
   uint16_t type = 0;
   uint32_t nb_val = 0;
   uint32_t val = 0;
   for (uint32_t i = 0; i < nb_entries; i++) {
      read_ifd_entry (tfd->tiff, tfd->en, &tag, &type, &nb_val, &val);

      switch (tag) {
      case IMAGEWIDTH:
	 /* printf ("IMAGEWIDTH:\n"); */
	 tfd->imageWidth = val;
	 /* printf ("\timageWidth: %#x <=> %d\n", tfd->imageWidth, tfd->imageWidth); */
	 break;
      case IMAGELENGTH:
	 /* printf ("IMAGELENGTH:\n"); */
	 tfd->imageLength = val;
	 /* printf ("\timageLength: %#x <=> %d\n", tfd->imageLength, tfd->imageLength); */
	 break;
      case BITS_PER_SAMPLE:
	 /* printf ("BITS_PER_SAMPLE:\n"); */
      	 break;
      case COMPRESSION:
	 /* printf ("COMPRESSION:\n"); */
	 if ((val >> 16) != 1) {
	    fprintf(stderr, "TIFF erreur: Unsupported compression.\n");
	    exit (EXIT_FAILURE);
	 }
      	 break;
      case PHOTOMETRIC_INTERPRETATION:
	 /* printf ("PHOTOMETRIC_INTERPRETATION:\n"); */
      	 break;
      case STRIP_OFFSET:
	 /* printf ("STRIP_OFFSET:\n"); */
	 tfd->soOffset = val;
	 /* printf ("\tsoOffset: %#x\n", tfd->soOffset); */
	 break;
      case SAMPLE_PER_PIXEL:
	 /* printf ("SAMPLE_PER_PIXEL:\n"); */
	 if (val != 3) {
	    fprintf(stderr, "TIFF erreur: Unsupported color space.\n");
	    exit (EXIT_FAILURE);
	 }
      	 break;
      case ROWSPERSTRIP:
	 /* printf ("ROWSPERSTRIP:\n"); */
	 tfd->rowsPerStrip = val;
	 /* printf ("\trowsPerStrip: %#x <=> %d\n", tfd->rowsPerStrip, tfd->rowsPerStrip); */
	 break;
      case STRIP_BYTE_COUNTS:
	 /* printf ("STRIP_BYTE_COUNTS:\n"); */
	 tfd->sbcOffset = val;
	 /* printf ("\tsbcOffset: %#x\n", tfd->sbcOffset); */
	 break;
      case X_RESOLUTION:
	 /* printf ("X_RESOLUTION:\n"); */
      	 break;
      case Y_RESOLUTION:
	 /* printf ("Y_RESOLUTION:\n"); */
	 break;
      case RESOLUTION_UNIT:
	 /* printf ("RESOLUTION_UNIT:\n"); */
	 break;
      default:
	 fprintf(stderr, "TIFF erreur: TAG TIFF inconnu: %#x\n", tag);
	 break;
      }

   }

   /* Vérification de l'unicité de l'ifd */
   uint32_t offset_to_next_ifd;
   read_nbytes (tfd->tiff, tfd->en, 4, &offset_to_next_ifd);
   if (offset_to_next_ifd) {
      fprintf(stderr, "TIFF erreur: Plus d'une IFD dans le fichier, non supporté.\n");
      exit (EXIT_FAILURE);
   }
}

void get_tiff_scan_data (struct tiff_file_desc *tfd)
{
   /* Lecture des longueurs de strip */
   /* printf ("\n-- sbc_offsets --\n"); */
   tfd->nbStrips = ((tfd->imageLength + tfd->rowsPerStrip - 1) / tfd->rowsPerStrip);
   /* printf ("nb_strips: %d\n", tfd->nbStrips); */
   tfd->stripByteCounts = smalloc (tfd->nbStrips * sizeof(uint32_t));
   fseek (tfd->tiff, tfd->sbcOffset, SEEK_SET);
   for (uint32_t i = 0; i < tfd->nbStrips; i++) {
      read_nbytes (tfd->tiff, tfd->en, 4, &tfd->stripByteCounts[i]);
      /* printf ("sbc[%d]: %d\n", i, tfd->stripByteCounts[i]); */
   }

   /* Lecture des offset de strip */
   /* printf ("\n-- so_offsets --\n"); */
   tfd->stripOffsets = smalloc (tfd->nbStrips * sizeof(uint32_t));
   fseek (tfd->tiff, tfd->soOffset, SEEK_SET);
   for (uint32_t i = 0; i < tfd->nbStrips; i++) {
      read_nbytes (tfd->tiff, tfd->en, 4, &tfd->stripOffsets[i]);
      /* printf ("so[%d]: %#x\n", i, tfd->stripOffsets[i]); */
   }

   /* Stockage des strips dans un tableau de pixel représentant le scan  */
   tfd->scanHeight = tfd->nbStrips * tfd->rowsPerStrip;
   tfd->scanWidth = ((tfd->imageWidth + tfd->rowsPerStrip - 1)
		     / tfd->rowsPerStrip) * tfd->rowsPerStrip;
   /* printf ("H: %d W: %d\n", tfd->scanHeight, tfd->scanWidth); */
   tfd->imageScan = smalloc (tfd->scanHeight * sizeof(uint32_t*));
   for (uint32_t i = 0; i < tfd->scanHeight; i++)
      tfd->imageScan[i] = smalloc (tfd->scanWidth * sizeof(uint32_t));

   uint32_t pix_x = 0;
   uint32_t pix_y = 0;

   for (uint32_t i = 0; i < tfd->nbStrips; i++) {
      fseek (tfd->tiff, tfd->stripOffsets[i], SEEK_SET);
      for (uint32_t j = 0; j < tfd->stripByteCounts[i]; j  += 3) {
	 if (pix_x >= tfd->imageWidth) {
	    /* Remplissage avec la valeur de pixels de bordure */
	    while (pix_x < tfd->scanWidth) {
	       tfd->imageScan[pix_y][pix_x] = tfd->imageScan[pix_y][pix_x-1];
	       pix_x++;
	    }
	    pix_y++;
	    pix_x = 0;
	 }
	 /* printf ("i: %d j: %d pix_x = %d pix_y = %d\n", i, j, pix_x, pix_y); */
   	 read_nbytes (tfd->tiff, tfd->en, 3, &tfd->imageScan[pix_y][pix_x++]);
	 /* printf ("scan[%d][%d] = %#x\n", pix_y, pix_x - 1, tfd->imageScan[pix_y][pix_x-1]); */
      }
   }

   /* Ajout du padding vertical si nécessaire */
   while (pix_y < tfd->scanHeight) {
      	 if (pix_x >= tfd->scanWidth) {
	    pix_y++;

	    if (pix_y >= tfd->scanHeight)
	       break;

	    pix_x = 0;
	 }

	 tfd->imageScan[pix_y][pix_x] = tfd->imageScan[pix_y-1][pix_x];
	 /* printf ("scan[%d][%d] = %#x\n", pix_y, pix_x, tfd->imageScan[pix_y-1][pix_x]); */
	 pix_x++;
   }

   /* printf ("\npix_x = %d pix_y = %d\n", pix_x, pix_y); */
   /* printf ("pix %d at offset %#lx, with %d pixels in image\n", pix_x * pix_y, */
   /* 	   ftell (tfd->tiff), tfd->imageLength * tfd->imageWidth); */
}

uint32_t **split_scan_into_16x16_MCU(struct tiff_file_desc *tfd, uint32_t *nbMCUH, uint32_t *nbMCUV)
{
   *nbMCUH = (tfd->imageWidth + 15) / 16;
   *nbMCUV = (tfd->imageLength + 15) / 16;
   uint32_t nbMCU = *nbMCUH * *nbMCUV;
   printf ("nbMCUH: %d | nbMCUV: %d => nbMCU: %d\n", *nbMCUH, *nbMCUV, nbMCU);

   uint32_t **mcus = smalloc (nbMCU * sizeof (uint32_t *));
   for (uint32_t i = 0; i < nbMCU; i++)
      mcus[i] = smalloc (256 * sizeof (uint32_t));

   uint32_t mcu_id = 0;
   uint32_t pix = 0;
   uint32_t src_pix_x = 0;
   uint32_t src_pix_y = 0;

   for (uint32_t i = 0; i < *nbMCUV; i++) {
      for (uint32_t j = 0; j < *nbMCUH; j++) {

   	 for (uint32_t k = 0; k < 16; k++) {
   	    for (uint32_t l = 0; l < 16; l++) {
   	       mcu_id = *nbMCUH * i + j;
   	       pix = 16*k + l;
   	       src_pix_x = 16 * j + l;
   	       src_pix_y = 16 * i + k;
	       /* printf ("MCU[%d][%d] = scan[%d][%d]\n", mcu_id, pix, src_pix_y, src_pix_x); */
	       mcus[mcu_id][pix] = tfd->imageScan[src_pix_y][src_pix_x];
   	    }
   	 }

      }
   }

   return mcus;
}

void free_tfd (struct tiff_file_desc *tfd)
{
   if (tfd) {
      fclose (tfd->tiff);
      for (uint32_t i = 0; i < tfd->scanHeight; i++)
	 free (tfd->imageScan[i]);
      free (tfd->imageScan);
      free (tfd->stripOffsets);
      free (tfd->stripByteCounts);
      free (tfd);
      tfd = NULL;
   }
}
