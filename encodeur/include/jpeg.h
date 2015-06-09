#ifndef __JPEG_H__
#define __JPEG_H__

#include "bitstream.h"

#include <stdint.h>
#include <stdio.h>


/* Structure permettant de stocker les informations nécessaire à
 * l'écriture des données de l'image dans un fichier jpeg. */
struct jpeg_file_desc;

/* Initialisation du fichier JPEG résultat, avec les paramètres suivants:
   - width: la largeur de l'image ;
   - height: la hauteur de l'image ;
   - sfh, sfv : facteurs d'échantillonage horizontaux et verticaux
*/
extern struct jpeg_file_desc *init_jpeg_file (const char *output_name,
                                              uint32_t width,
                                              uint32_t height,
                                              uint8_t sfh,
					      uint8_t sfv);

extern void export_DQT(struct jpeg_file_desc *jfd,
		       const uint8_t table_quantif[2][64]);

extern void export_SOF0(struct jpeg_file_desc *jfd);

/* Ferme le fichier associé à la structure jpeg_file_desc passée en
 * paramètre et désalloue la mémoire occupée par cette structure. */
extern void close_jpeg_file(struct jpeg_file_desc *jfd);

/* /\* Ecrit le contenu de la MCU passée en paramètre dans le fichier TIFF */
/*  * représenté par la structure tiff_file_desc tfd. nb_blocks_h et */
/*  * nb_blocks_v représentent les nombres de blocs 8x8 composant la MCU */
/*  * en horizontal et en vertical. *\/ */
/* extern int32_t write_tiff_file (struct tiff_file_desc *tfd, */
/*                                 uint32_t *mcu_rgb, */
/*                                 uint8_t nb_blocks_h, */
/*                                 uint8_t nb_blocks_v) ; */

#endif
