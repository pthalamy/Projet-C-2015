
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

   /* TIFF */
   printf ("\nTIFF\n");

   /* Initialisation du lecteur de TIFF */
   struct tiff_file_desc *tfd = create_tfd_and_read_header (argv[1]);
   if (!tfd) {
      fprintf(stderr, "Impossible d'ouvrir le fichier TIFF. Le fichier spécifié n'existe pas.\n");
      exit (1);
   }

   /* Lecure de l'IFD du fichier TIFF */
   read_TIFF_ifd (tfd);

   /* Lecture des données des l'image et découpage en MCU 16x16 */
   get_tiff_scan_data (tfd);
   uint32_t nbMCUH, nbMCUV;
   uint32_t **MCUScan = split_scan_into_16x16_MCU(tfd, &nbMCUH, &nbMCUV);
   uint32_t nbMCU_scan = nbMCUV * nbMCUH;
   printf ("nbMCU_scan: %d\n", nbMCU_scan);

   /* YCbCr CONVERSION */
   printf ("\nYCbCr CONVERSION\n");
   uint8_t ***MCU_YCbCr  = smalloc (nbMCU_scan * sizeof (uint8_t **));
   for (uint32_t i = 0; i < nbMCU_scan; i++) {
      MCU_YCbCr[i] = smalloc (3 * sizeof (uint8_t *));
      for (uint32_t j = 0; j < 3; j++)
	 MCU_YCbCr[i][j] = smalloc (256 * sizeof (uint8_t));
   }

   for (uint32_t i = 0; i < nbMCU_scan; i++) {
      RGB_to_YCbCr(MCUScan[i], 2, 2, MCU_YCbCr[i]);
      /* print_mcu (MCU_YCbCr[i][0], i, 2, 2); */
      /* print_mcu (MCU_YCbCr[i][1], i, 2, 2); */
      /* print_mcu (MCU_YCbCr[i][2], i, 2, 2); */
   }

   /* DOWNSAMPLING 4:2:0 */
   printf ("\nDOWNSAMPLING 4:2:0\n");

   /* 4 blocs pour y, 1 pour Cb, 1 pour Cr = 6 blocs 8*8 */
   uint32_t nbBlocs = 6 * nbMCU_scan;
   printf ("nbBlocs: %d\n", nbBlocs);
   uint8_t *downBlocs = smalloc (nbBlocs * 64 * sizeof(uint8_t));

   uint32_t blocId = 0;
   for (uint32_t i = 0; i < nbMCU_scan; i++) {
      /* Downsampling Y */
      downsampler (MCU_YCbCr[i][0], 2, 2, 2, 2, downBlocs + blocId * (64 * sizeof(uint8_t)) );
      /* print_uint8_t_block (downBlocs + blocId * (64 * sizeof(uint8_t)), 0); */
      /* print_uint8_t_block (downBlocs + (blocId + 1) * (64 * sizeof(uint8_t)), 1); */
      /* print_uint8_t_block (downBlocs + (blocId + 2) * (64 * sizeof(uint8_t)), 2); */
      /* print_uint8_t_block (downBlocs + (blocId + 3) * (64 * sizeof(uint8_t)), 3); */
      blocId += 4;
      /* Downsampling Cb */
      downsampler (MCU_YCbCr[i][1], 2, 2, 1, 1, downBlocs + blocId * (64 * sizeof(uint8_t)) );
      /* print_block (downBlocs + blocId * (64 * sizeof(uint8_t)), 0); */
      blocId ++;
      /* Downsampling Cr */
      downsampler (MCU_YCbCr[i][2], 2, 2, 1, 1, downBlocs + blocId * (64 * sizeof(uint8_t)) );
      /* print_block (downBlocs + blocId * (64 * sizeof(uint8_t)), 0); */
      blocId ++;
   }

   /* DCT */
   printf ("\nDCT\n");
   int32_t *dctBlocs = smalloc (nbBlocs * 64 * sizeof(int32_t));
   for (uint32_t i = 0; i < nbBlocs; i++) {
      /* print_uint8_t_block (downBlocs + i * (64 * sizeof(uint8_t)), i); */
      dct(downBlocs + i * (64 * sizeof(uint8_t)), dctBlocs + i * (64 * sizeof(int32_t)));
      /* print_int32_t_block (dctBlocs + i * (64 * sizeof(int32_t)), i); */
   }

   /* QUANTIFICATION ZIGZAG */
   printf ("\nQUANTIFICATION ZIG-ZAG\n");
   int32_t *qzzBlocs = smalloc (nbBlocs * 64 * sizeof(int32_t));
   for (uint32_t i = 0; i < nbBlocs; ) {
      /* 4 Blocs de Luminance */
      for (uint32_t j = i; j < i + 4; j++) {
	 iqzz_enc (dctBlocs + i * (64 * sizeof(int32_t)), qzzBlocs + i * (64 * sizeof(int32_t)), L);
	 /* print_int32_t_block (qzzBlocs + j * (64 * sizeof(int32_t)), j); */
      }
      i += 4;
      /* 1 Bloc Cb */
      iqzz_enc (dctBlocs + i * (64 * sizeof(int32_t)), qzzBlocs + i * (64 * sizeof(int32_t)), C);
      /* print_int32_t_block (qzzBlocs + i * (64 * sizeof(int32_t)), i); */
      i++;
      /* 1 Bloc Cr */
      iqzz_enc (dctBlocs + i * (64 * sizeof(int32_t)), qzzBlocs + i * (64 * sizeof(int32_t)), C);
      /* print_int32_t_block (qzzBlocs + i * (64 * sizeof(int32_t)), i); */
      i++;
   }


   /* HUFFMAN */
   printf ("\nHUFFMAN\n");
   struct elt **freq_DC = scalloc (256, sizeof(struct elt*));
   struct elt **freq_AC = scalloc (256, sizeof(struct elt*));
   uint8_t ind_DC;
   uint8_t ind_AC;
   int32_t pred_DC[3] = {0, 0, 0};

   for (uint32_t i = 0; i < nbBlocs; ) {
      /* 4 Blocs de Luminance */
      for (uint32_t j = i; j < i + 4; j++) {
	 print_int32_t_block (qzzBlocs + j * (64 * sizeof(int32_t)), j);
   	 init_freq(qzzBlocs + j * (64 * sizeof(int32_t)), freq_DC, &ind_DC, freq_AC, &ind_AC, &pred_DC[0]);
	 exit(1);
      }
      i += 4;
      /* 1 Bloc Cb */
      init_freq(qzzBlocs + i * (64 * sizeof(int32_t)), freq_DC, &ind_DC, freq_AC, &ind_AC, &pred_DC[1]);
      i++;
      /* 1 Bloc Cr */
      init_freq(qzzBlocs + i * (64 * sizeof(int32_t)), freq_DC, &ind_DC, freq_AC, &ind_AC, &pred_DC[2]);
      i++;
   }

   /* struct abr *table_DC = create_huffman_table(freq_DC, &int_DC); */
   /* struct abr *table_AC = create_huffman_table(freq_DC, &int_DC); */

   /* FREE */

   for (uint32_t i = 0; i < nbMCU_scan; i++)
      free (MCUScan[i]);
   free (MCUScan);

   for (uint32_t i = 0; i < nbMCU_scan; i++) {
      for (uint32_t j = 0; j < 3; j++)
	 free (MCU_YCbCr[i][j]);
      free (MCU_YCbCr[i]);
   }
   free (MCU_YCbCr);

   free (downBlocs);
   free (dctBlocs);
   free (qzzBlocs);

   free (freq_AC);
   free (freq_DC);

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
