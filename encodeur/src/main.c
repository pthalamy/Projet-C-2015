
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

   /* TIFF */
   printf ("\nTIFF\n");

   /* Initialisation du lecteur de TIFF */
   struct tiff_file_desc *tfd = create_tfd_and_read_header (argv[1]);
   if (!tfd) {
      fprintf(stderr, "Impossible d'ouvrir le fichier TIFF. Le fichier spécifié n'existe pas.\n");
      exit (EXIT_FAILURE);
   }

   /* Lecure de l'IFD du fichier TIFF */
   uint32_t width, height;
   read_TIFF_ifd (tfd, &height, &width);

   /* Lecture des données des l'image et découpage en MCU 16x16 */
   get_tiff_scan_data (tfd);
   uint32_t nbMCUH, nbMCUV;
   uint32_t **MCUScan = split_scan_into_16x16_MCU(tfd, &nbMCUH, &nbMCUV);
   uint32_t nbMCU_scan = nbMCUV * nbMCUH;
   printf ("nbMCU_scan: %d\n", nbMCU_scan);

   /* INIT JPEG */
   printf ("\nINIT JPEG\n");
   /* Initialisation du jpeg de sortie */
   struct jpeg_file_desc *jfd = init_jpeg_file (output_name, 0, 0, 0, 0);
   if (!jfd) {
      fprintf(stderr, "Impossible de créer le fichier de sortie.\n");
      exit (EXIT_FAILURE);
   }

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

   export_DQT (jfd, table_quantif);
   export_SOF0 (jfd);

   /* HUFFMAN */
   printf ("\nHUFFMAN\n");

   export_DHT(jfd, DC_Y_len, DC_Y_sym, DC_Y_NB_SYM, 0, 0);
   export_DHT(jfd, DC_C_len, DC_C_sym, DC_C_NB_SYM, 0, 1);
   export_DHT(jfd, AC_Y_len, AC_Y_sym, AC_Y_NB_SYM, 1, 2);
   export_DHT(jfd, AC_C_len, AC_C_sym, AC_C_NB_SYM, 1, 3);

   struct huff_table *table_DC_Y = create_huffman_table(DC_Y_len, DC_Y_sym);
   /* affiche_huffman (table_DC_Y); */
   struct huff_table *table_AC_Y = create_huffman_table(AC_Y_len, AC_Y_sym);
   /* affiche_huffman (table_AC_Y); */
   struct huff_table *table_DC_C = create_huffman_table(DC_C_len, DC_C_sym);
   /* affiche_huffman (table_DC_C); */
   struct huff_table *table_AC_C = create_huffman_table(AC_C_len, AC_C_sym);
   /* affiche_huffman (table_AC_C); */

   /* struct elt **freq_DC_Y = scalloc (256, sizeof(struct elt*)); */
   /* struct elt **freq_AC_Y = scalloc (256, sizeof(struct elt*)); */
   /* struct elt **freq_DC_C = scalloc (256, sizeof(struct elt*)); */
   /* struct elt **freq_AC_C = scalloc (256, sizeof(struct elt*)); */
   /* uint8_t ind_DC_Y; */
   /* uint8_t ind_AC_Y; */
   /* uint8_t ind_DC_C; */
   /* uint8_t ind_AC_C; */
   /* int32_t pred_DC_Y; */
   /* int32_t pred_DC_C; */

   /* for (uint32_t i = 0; i < nbBlocs; ) { */
   /*    /\* 4 Blocs de Luminance *\/ */
   /*    for (uint32_t j = i; j < i + 4; j++) { */
   /* 	 /\* print_int32_t_block (qzzBlocs + j * (64 * sizeof(int32_t)), j); *\/ */
   /* 	 init_freq(qzzBlocs + j * (64 * sizeof(int32_t)), freq_DC_Y, &ind_DC_Y, freq_AC_Y, &ind_AC_Y, &pred_DC_Y); */
   /*    } */
   /*    i += 4; */
   /*    /\* 1 Bloc Cb *\/ */
   /*    init_freq(qzzBlocs + i * (64 * sizeof(int32_t)), freq_DC_C, &ind_DC_C, freq_AC_C, &ind_AC_C, &pred_DC_C); */
   /*    i++; */
   /*    /\* 1 Bloc Cr *\/ */
   /*    init_freq(qzzBlocs + i * (64 * sizeof(int32_t)), freq_DC_C, &ind_DC_C, freq_AC_C, &ind_AC_C, &pred_DC_C); */
   /*    i++; */
   /* } */

   /* printf ("Nombre de symboles: %d\n", ind_DC_Y); */
   /* printf("< "); */
   /* for (uint8_t i = 0; i < ind_DC_Y; i++) { */
   /*    printf("%d ", freq_DC_Y[i]->occ); */
   /* } */
   /* printf(">\n"); */
   /* printf("[ "); */
   /* for (uint8_t i = 0; i < ind_DC_Y; i++) { */
   /*    printf("%#x ", freq_DC_Y[i]->abr->symbole); */
   /* } */
   /* printf("]\n"); */

   /* struct abr *table_DC_Y = create_huffman_table(freq_DC_Y, &ind_DC_Y); */
   /* printf ("DC Y:\n"); */
   /* affiche_huffman (table_DC_Y); */
   /* struct abr *table_AC_Y = create_huffman_table(freq_AC_Y, &ind_AC_Y); */
   /* printf ("AC Y:\n"); */
   /* affiche_huffman (table_DC_Y); */
   /* struct abr *table_DC_C = create_huffman_table(freq_DC_C, &ind_DC_C); */
   /* printf ("DC C:\n"); */
   /* affiche_huffman (table_DC_Y); */
   /* struct abr *table_AC_C = create_huffman_table(freq_AC_C, &ind_AC_C); */
   /* printf ("AC C:\n"); */
   /* affiche_huffman (table_DC_Y); */
   /* exit (EXIT_FAILURE); */

   /* PACK */
   printf ("\nPACK\n");

   export_SOS_Header (jfd);

   int32_t pred_DC_C = 0;
   int32_t pred_DC_Y = 0;
   for (uint32_t i = 0; i < nbBlocs; ) {
      /* 4 Blocs de Luminance */
      for (uint32_t j = i; j < i + 4; j++) {
   	 diff_DC(jfd->stream, &pred_DC_Y, table_DC_Y,  qzzBlocs + j * (64 * sizeof(int32_t)));
   	 RLE_AC(jfd->stream, qzzBlocs + j * (64 * sizeof(int32_t)), table_AC_Y);
      }
      i += 4;
      /* 1 Bloc Cb */
      diff_DC(jfd->stream, &pred_DC_C, table_DC_C,  qzzBlocs + i * (64 * sizeof(int32_t)));
      RLE_AC(jfd->stream, qzzBlocs + i * (64 * sizeof(int32_t)), table_AC_C);
      i++;
      /* 1 Bloc Cr */
      diff_DC(jfd->stream, &pred_DC_C, table_DC_C,  qzzBlocs + i * (64 * sizeof(int32_t)));
      RLE_AC(jfd->stream, qzzBlocs + i * (64 * sizeof(int32_t)), table_AC_C);
      i++;
   }

   close_jpeg_file (jfd);

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

   free_huffman_table (table_DC_Y);
   free_huffman_table (table_DC_C);
   free_huffman_table (table_AC_Y);
   free_huffman_table (table_AC_C);

   /* free (freq_AC_Y); */
   /* free (freq_DC_Y); */
   /* free (freq_AC_C); */
   /* free (freq_DC_C); */

   free_tfd (tfd);
   free (output_name);

   printf ("\nDONE \n");

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
