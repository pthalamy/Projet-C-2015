
#include "qzz.h"
#include "dct.h"
#include "conv.h"
#include "downsampler.h"
#include "huffman.h"
#include "pack.h"
#include "bitstream.h"
#include "jpeg.h"
#include "tiff.h"
#include "utils.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* PROTOTYPES */
/* Vérifie que le fichier d'entrée à l'extension jpeg ou jpg et génère le nom en .tiff */
char *check_and_gen_name(const char *input_name);

int main(int argc, char **argv)
{
   /* vérification de l'entrée */
   if ( argc != 2 ) {
      fprintf(stderr, "Veuillez entrer le fichier TIFF à encoder en argument. \n");
      exit (EXIT_FAILURE);
   }

   /* Vérification de la validité du nom et création du nom du fichier de sortie */
   char *output_name = check_and_gen_name (argv[1]);

   /* Initialisation du bitstream */
   struct bitstream *stream = create_bitstream(output_name);
   if (!stream) {
      fprintf(stderr, "Impossible de créer le bitstream.\n");
      exit (1);
   }

   /* Initialisation du lecteur de TIFF */
   struct tiff_file_desc *tfd = create_tfd_and_read_header (argv[1]);
   if (!tfd) {
      fprintf(stderr, "Impossible d'ouvrir le fichier TIFF. Le fichier spécifié n'existe pas.\n");
      exit (1);
   }

   /* Lecure de l'IFD du fichier TIFF */
   read_TIFF_ifd (tfd);

   /* Lecture des données des l'image et découpage en blocs 8*8 */
   get_tiff_scan_data (tfd);
   uint32_t nbBlocksH, nbBlocksV;
   uint32_t **blocks = split_scan_into_blocks(tfd, &nbBlocksH, &nbBlocksV);
   uint32_t nbBlocks = nbBlocksV * nbBlocksH;

   /* for (uint32_t i = 0; i < nbBlocks; i++) { */
   /*    print_block(blocks[i], i); */
   /* } */

   for (uint32_t i = 0; i < nbBlocks; i++)
      free (blocks[i]);
   free (blocks);
   free_tfd (tfd);
   free_bitstream(stream);
   free (output_name);

   return (EXIT_SUCCESS);
}

char *check_and_gen_name(const char *input_name)
{
   /* On repere l'extension actuelle */
   char *ext = strrchr (input_name, '.');

   if (strcmp(ext, ".tiff")) {
      fprintf(stderr, "erreur: L'extension de fichier n'est pas .tiff !\n");
      exit (EXIT_FAILURE);
   }

   /* On remplace l'extension .tiff par .jpeg */
   char *jpeg_ext = ".jpeg";
   char *output_name = scalloc ((strlen(input_name) + 1), sizeof(char));
   strncpy (output_name, input_name, strlen(input_name) - strlen(ext));
   strcat (output_name, jpeg_ext);
   printf ("output_name: %s\n", output_name);

   return output_name;
}
