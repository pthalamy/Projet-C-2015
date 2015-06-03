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
   uint8_t compteur_huff_AC = 0 ; // comptent le nb de tables définies
   uint8_t compteur_huff_DC = 0;
   struct huff_table *huff_AC[4];
   struct huff_table *huff_DC[4];

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

   case 0xFE :			/* COM */
      read_nbytes(stream, 2, &longueur_section, false);
      for (uint8_t i =0; i<longueur_section-2 ; i++){
	 read_nbytes(stream, 1, &buf, false);
	 printf("%c", buf) ;
      }
      // passer en section suivante : skip ?
      break ;

   case 0xdb:			/* DQT */
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

      // passer en section suivante : skip ?
      break ;


   case 0xc0:			/* SOF0 */

      break;

   case 0xc4:			/* DHT */
      read_nbytes(stream, 2, &longueur_section, false);
      longueur_section=longueur_section-2;

      read_nbytes(stream, 1, &longueur_section, false);
      longueur_section=longueur_section-2;

      break ;
   case 0xda:			/* SOS */
      break ;
   default :
      printf("erreur, marqueur de section non reconnu \n");

   }

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


   /* /\*ecriture dans le TIFF*\/ */
   /* struct tiff_file_desc *TIFF; */
   /* TIFF = init_tiff_file(&argv[1], ); */
   /* write_tiff_file(TIFF, mcu_rgb, nb_blocks_h, nb_blocks_v); */
   /* clos_tifffile(TIFF); */

   /* /\*On desalloue le flux de bit *\/ */
   /* free_bistream(stream); */
   return 0;
}
