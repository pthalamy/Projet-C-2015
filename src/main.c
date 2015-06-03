#include "bitstream.h" 
#include "huffan.h"
#include "unpack.h"
#include "iqzz.h"
#include "idct.h"
#include "upsampler.h"
#include "conv.h"
#include "tiff.h"

void read_nbits(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool byte_stuffing){
   uint8_t ok;
   if read_bitsream(stream, nb_bits, det, byte_stuffing) != nb_bits 
      printf("Erreur"); 
}

void read_huffman(struct bitstream *stream, struct huffman table_AC[], struct huffman table_DC[]) {
   uint32_t buffer;
   read_nbits(stream, 1, &buffer, false);
   if ( buffer = 0 )             // type DC 
      table_DC[0] = load_hufman_table(stream, &nb_byte_read);
   else if ( buffer = 1 )        // type AC
      table_AC[0] = load_huffman_table(stream, &nb_byte_read);
   while ( nb_byte_read < longeur_section )
      read_huffman(stream, table_AC[], table_DC[]);
} 

int main(int argc, char *argv[]){
   if ( arg != 2 ) 
      printf("Veuillez entrer le fichier JPEG à décoder en argument. \n");
   struct bitstream *steam;
   stream= create_bitstream(&argv[1]);

   /*extraction de l'entete*/
   skip_bitstream(stream, 0xd8);
   uint32_t buffer;
   uint32_t longeur_en_tete;
   uint32_t en_tete;
   read_nbits(stream, 8, &buffer, false); 
   read_nbits(stream, 16, &longeur_en_tete, false);
   read_nbits(stream, longeur_en_tete, &en_tete, false); 

   /*recuperation de la table de quantification*/   
   skip_bitstream(stream, 0xdb);
   uint8_t quantif[64];
   /*uint32_t i_q;
     read_nbits(stream, 28, &buffer, false); 
     read_nbits(stream, 4, &i_q, false); */
   read_nbits(stream, 24, &buffer, false);
   for ( uint32_t i=0; i < 64; i++) 
      read_nbits(stream, 8, &quantif[i], false); 

   /*recuperation des facteurs d'echantillonage*/
   skip_bitstream(stream, 0xc0);
   uint32_t nb_composants;
   read_nbits(stream, 9*8, &buffer, false);
   read_nbits(stream, 8, &nb_composants, false);
   uint32_t sampling_factor_h[nb_composants];
   uint32_t sampling_factor_v[nb_composants];
   for ( uint32_t i=0; i < nb_composants; i++) {
      read_nbits(stream, 8, &buffer, false);
      read_nbits(stream, 4, &sampling_factor_h[i], false);
      read_nbits(stream, 4, &sampling_factor_v[i], false);
   }

   /*recuperation des tables de Huffman*/
   skip_be=itstream(stream, 0xc4);
   uint32_t longeur_section;
   uint32_t val_erreur;
   read_nbits(stream, 8, &buffer, false); 
   read_nbits(stream, 16, &longeur_section, false);
   read_nbits(stream, 3, &val_erreur, false); //3bits non utilisés, doit valoir 0 
   if ( val_erreur != 0) 
      printf("Erreur!"); 
   uint16_t nb_byte_read;
   struct huffman table_AC[4];
   struct huffman table_DC[4];
   read_huffman(stream, table_AC[], table_DC[]);

   /*extraction, decompression, multiplication par les facteurs et 
    * réorganisation zizgag des données des blocs
    * + transform&e en cosinus discrete inverse*/
   for ( uint32_t i=0; i < 4; i++) {
      uint32_t bloc[64];
      uint32_t out_iqzz[64];
      uint32_t out_idct[64]; 
      uint32_t *prec_DC;
      unpack_block(stream, &table_DC[i], prec_DC, &table_AC[i], bloc); //extraction et decompression
      iqzz_block(bloc, out_iqzz, quantif); //reorganisation zigzag
      idct_block(out_iqzz, out_idct); //calcul transformée en cosinus discrete inverse
   }

   /*Reconstitution des MCU*/

   /*Conversion YCbCr vers RGB de chaque pixel*/

   /*ecriture dans le TIFF*/
   struct tiff_file_desc *TIFF;
   TIFF = init_tiff_file(&argv[1], );
   write_tiff_file(TIFF, mcu_rgb, nb_blocks_h, nb_blocks_v);
   clos_tifffile(TIFF);

   /*On desalloue le flux de bit */
   free_bistream(stream);
   return 0;
}
