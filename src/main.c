#include "bitstream.h"
#include "huffman.h"
#include "unpack.h"
#include "iqzz.h"
#include "idct.h"
#include "upsampler.h"
#include "conv.h"
#include "tiff.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct table_quantif {
   uint8_t val[64] ;
   uint8_t ind ;
   uint8_t prec;
};



void read_nbits(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool byte_stuffing)
{
   uint8_t nbLus = read_bitstream(stream, nb_bits, dest, byte_stuffing);
   if (nbLus != nb_bits)
      fprintf(stderr, "Erreur lecture bitstream : %d / %d\n", nbLus, nb_bits);

   printf ("0x%x\n", *dest);
}

void read_nbytes(struct bitstream *stream, uint8_t nb_bytes, uint32_t *dest, bool byte_stuffing)
{
   uint8_t nbLus = read_bitstream(stream, 8*nb_bytes, dest, byte_stuffing);
   if (nbLus != 8*nb_bytes)
      fprintf(stderr, "Erreur lecture bitstream : %d / %d\n", nbLus, nb_bytes * 8);
   printf ("0x%x\n", *dest);
}




/* void read_huffman(struct bitstream *stream, struct huffman table_AC[], struct huffman table_DC[]) { */
/*    uint32_t buffer; */
/*    uint32_t longueur_section ; */
/*    read_nbits(stream, 1, &buffer, false); */
/*    if ( buffer = 0 )             // type DC */
/*       table_DC[0] = load_huffman_table(stream, &nb_byte_read); */
/*    else if ( buffer = 1 )        // type AC */
/*       table_AC[0] = load_huffman_table(stream, &nb_byte_read); */
/*    while ( nb_byte_read < longueur_section ) */
/*       read_huffman(stream, table_AC[], table_DC[]); */
/* } */

int main(int argc, char *argv[]){

   // vérification de l'entrée
   if ( argc != 2 ) {
      fprintf(stderr, "Veuillez entrer le fichier JPEG à décoder en argument. \n");
      exit (1);
   }

   struct bitstream *stream = create_bitstream(argv[1]);
   if (!stream) {
      fprintf(stderr, "Impossible de créer le bitstream\n");
      exit (1);
   }

   // EXTRACTION DE L ENTETE
   uint32_t buf;
   uint32_t longueur_section ;
   bool unicite = true ; // vérifie que la déclaration des DQT est correcte


   // saut marqueur SOI
   read_nbytes(stream, 2, &buf, false);
   if (buf != 0xffd8) {
      fprintf(stderr, "Le fichier en commence par un SOI mais 0x%x !\n", buf);
      exit (1);
   }

   // Lecture du premier marqueur de section
   read_nbytes(stream, 1, &buf, false); /* On passe 0xff */
   read_nbytes(stream, 1, &buf, false);

   struct table_quantif *quantif;

   switch(buf){

      //APP0 : encapsulation JFIF
   case 0xe0 :
      printf ("APP0: \n");
      read_nbytes(stream, 2, &longueur_section, false);
      longueur_section = longueur_section;
      printf (" longeur section: %d\n", longueur_section);

      /* Lecture  de JFIF */
      char jfif[5];
      read_nbytes(stream, 1, (uint32_t*)&jfif[0], false);
      read_nbytes(stream, 1, (uint32_t*)&jfif[1], false);
      read_nbytes(stream, 1, (uint32_t*)&jfif[2], false);
      read_nbytes(stream, 1, (uint32_t*)&jfif[3], false);
      read_nbytes(stream, 1, (uint32_t*)&jfif[4], false);
      if (strcmp (jfif, "JFIF")) {
	 fprintf (stderr, "erreur APP0 : JFIF absent <= %s\n", jfif);
	 exit (1);
      }

      // PASSER A LA FIN DE LA SECTION (SKIP ou READBYTES ?)
      /* Lecture des données additionnelles */
      uint32_t version;
      read_nbytes(stream, 2, &version, false);
      printf (" version JFIF: %d\n", version);

      uint32_t densityUnits;
      read_nbytes(stream, 1, &densityUnits, false);
      printf ("  density unit: %d\n", densityUnits);

      uint32_t xDensity;
      read_nbytes(stream, 2, &xDensity, false);
      printf ("  x density: %d\n", xDensity);

      uint32_t yDensity;
      read_nbytes(stream, 2, &yDensity, false);
      printf ("  y density: %d\n", yDensity);

      uint32_t tw;
      read_nbytes(stream, 1, &tw, false);
      printf ("  thumbnail width: %d\n", tw);

      uint32_t th;
      read_nbytes(stream, 1, &th, false);
      printf ("  thumbnail height: %d\n", th);

      uint32_t t_data;
      for (uint8_t i = 0; i < tw*th ; i++){
	 read_nbytes(stream, 3, &t_data, false);
      }
      break ;

      //section COM
   case 0xFE :
      read_nbytes(stream, 2, &longueur_section, false);
      for (uint8_t i =0; i<longueur_section ; i++){
	 read_nbytes(stream, 1, &buf, false);
	 printf("%c", buf) ;
      }

      break ;

      //section DQT
   case 0Xdb:
      if ( !unicite){
	 printf("erreur: plusieurs définitions des tables \n");
      }
      uint32_t precision;
      uint32_t iq ;
      uint32_t nb_tables ;
      //calcul du nombre de tables de la section
      read_nbytes(stream, 2, &longueur_section, false);
      nb_tables=(longueur_section-2)/65;
      if (nb_tables != 1){
	 unicite=false ;
      }

      // extraction de la précision et l'iq de la 1e table
       read_nbits(stream, 4, &precision, false);
       read_nbits(stream, 4, & iq, false);

      // ok dans le cas ou une section et plusieurs tables
      // cas une table par section (plusieurs sections a faire)
      quantif = malloc (nb_tables * sizeof(struct table_quantif));
      quantif[0].ind=iq ;
      quantif[0].prec=precision ;

      for (uint8_t i =0 ; i<nb_tables; i++){

	 if (i!=0){
	 read_nbits(stream, 4, &precision, false);
	 read_nbits(stream, 4, & iq, false);
	 quantif[i].prec=precision;
	 quantif[i].ind=iq ;
	  }

	 for (uint8_t j =0; j<64; j++){
	    read_nbytes(stream, 1, &buf, false);
	    quantif[i].val[j]=buf;
	 }


      }

      break ;

      // secton SOF0
   case 0xc0:
      break;
      //section DHT (tables huffman)
   case 0xc4:
      break ;
      // section SOS
   case 0xda:
      break ;
   default :
      printf("erreur, marqueur de section non reconnu \n");

   }

   /* /\*extraction de l'entete*\/ */
   /* skip_bitstream(stream, 0xd8); */
   /* uint32_t buffer; */
   /* uint32_t longueur_en_tete; */
   /* uint32_t en_tete; */
   /* read_nbits(stream, 8, &buffer, false); */
   /* read_nbits(stream, 16, &longeur_en_tete, false); */
   /* read_nbits(stream, longeur_en_tete, &en_tete, false); */

   /* /\*recuperation de la table de quantification*\/ */
   /* skip_bitstream(stream, 0xdb); */
   /* uint8_t quantif[64]; */
   /* /\*uint32_t i_q; */
   /*   read_nbits(stream, 28, &buffer, false); */
   /*   read_nbits(stream, 4, &i_q, false); *\/ */
   /* read_nbits(stream, 24, &buffer, false); */
   /* for ( uint32_t i=0; i < 64; i++) */
   /*    read_nbits(stream, 8, &quantif[i], false); */

   /* /\*recuperation des facteurs d'echantillonage*\/ */
   /* skip_bitstream(stream, 0xc0); */
   /* uint32_t nb_composants; */
   /* read_nbits(stream, 9*8, &buffer, false); */
   /* read_nbits(stream, 8, &nb_composants, false); */
   /* uint32_t sampling_factor_h[nb_composants]; */
   /* uint32_t sampling_factor_v[nb_composants]; */
   /* for ( uint32_t i=0; i < nb_composants; i++) { */
   /*    read_nbits(stream, 8, &buffer, false); */
   /*    read_nbits(stream, 4, &sampling_factor_h[i], false); */
   /*    read_nbits(stream, 4, &sampling_factor_v[i], false); */
   /* } */

   /* /\*recuperation des tables de Huffman*\/ */
   /* skip_be=itstream(stream, 0xc4); */
   /* uint32_t longeur_section; */
   /* uint32_t val_erreur; */
   /* read_nbits(stream, 8, &buffer, false); */
   /* read_nbits(stream, 16, &longeur_section, false); */
   /* read_nbits(stream, 3, &val_erreur, false); //3bits non utilisés, doit valoir 0 */
   /* if ( val_erreur != 0) */
   /*    printf("Erreur!"); */
   /* uint16_t nb_byte_read; */
   /* struct huffman table_AC[4]; */
   /* struct huffman table_DC[4]; */
   /* read_huffman(stream, table_AC[], table_DC[]); */

   /* /\*extraction, decompression, multiplication par les facteurs et */
   /*  * réorganisation zizgag des données des blocs */
   /*  * + transform&e en cosinus discrete inverse*\/ */
   /* for ( uint32_t i=0; i < 4; i++) { */
   /*    uint32_t bloc[64]; */
   /*    uint32_t out_iqzz[64]; */
   /*    uint32_t out_idct[64]; */
   /*    uint32_t *prec_DC; */
   /*    unpack_block(stream, &table_DC[i], prec_DC, &table_AC[i], bloc); //extraction et decompression */
   /*    iqzz_block(bloc, out_iqzz, quantif); //reorganisation zigzag */
   /*    idct_block(out_iqzz, out_idct); //calcul transformée en cosinus discrete inverse */
   /* } */

   /* /\*Reconstitution des MCU*\/ */

   /* /\*Conversion YCbCr vers RGB de chaque pixel*\/ */

   /* /\*ecriture dans le TIFF*\/ */
   /* struct tiff_file_desc *TIFF; */
   /* TIFF = init_tiff_file(&argv[1], ); */
   /* write_tiff_file(TIFF, mcu_rgb, nb_blocks_h, nb_blocks_v); */
   /* clos_tifffile(TIFF); */

   /* /\*On desalloue le flux de bit *\/ */
   /* free_bistream(stream); */
   return 0;
}
