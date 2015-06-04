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

struct unit{
   uint8_t ic;
   uint8_t iq;
   uint8_t sampling_factor_h ;
   uint8_t sampling_factor_v ;
   uint8_t ih_ac;		/* indice de huffman AC */
   uint8_t ih_dc; 		/* indice de huffman DC  */
};

/* struct mcu{ */
/*    uint32_t ic; */
/*    uint32_t ih_dc ; */
/*    uint32_t ih_ac; */
/* }; */

void read_nbits(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool byte_stuffing)
{
   uint8_t nbLus = read_bitstream(stream, nb_bits, dest, byte_stuffing);
   if (nbLus != nb_bits)
      fprintf(stderr, "Erreur lecture bitstream : %d / %d\n", nbLus, nb_bits);

   printf ("%#.2x ", *dest);
}

void read_nbytes(struct bitstream *stream, uint8_t nb_bytes, uint32_t *dest, bool byte_stuffing)
{
   uint8_t nbLus = read_bitstream(stream, 8*nb_bytes, dest, byte_stuffing);
   if (nbLus != 8*nb_bytes)
      fprintf(stderr, "Erreur lecture bitstream : %d / %d\n", nbLus, nb_bytes * 8);
   printf ("%#.2x ", *dest);
}

/* Renvoie l'indice correspondant à l'élément d'ic en argument
   dans le tableau composantes de N élements */
uint8_t ic_to_i(struct unit *composantes, uint32_t N, uint32_t ic)
{
   /* if (!composantes) */
   /*    printf ("dégout\n"); */

   for (uint8_t i = 0; i < N; i++)
      if (composantes[i].ic == ic)
	 return i;

   fprintf  (stderr, "erreur: l'ic %d n'existe pas parmis les composantes\n", ic);
   exit (1);
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
   uint32_t nb_blocks_scan;
   struct table_quantif *quantif;
   struct unit *composantes = NULL;
   int32_t **mcus ;

   uint32_t precision;
   uint32_t height;
   uint32_t width ;
   uint32_t N;
   uint32_t ic;
   uint32_t iq;
   uint32_t sampling_factor_h;
   uint32_t sampling_factor_v;

   uint32_t ih_ac;
   uint32_t ih_dc;

   // saut marqueur SOI
   read_nbytes(stream, 2, &buf, false);
   if (buf != 0xffd8) {
      fprintf(stderr, "Le fichier ne commence pas par un SOI mais 0x%x !\n", buf);
      exit (1);
   }

   while (true) {

      // Lecture du premier marqueur de section
      read_nbytes(stream, 1, &buf, false); /* On passe 0xff */
      read_nbytes(stream, 1, &buf, false);
      printf ("\n");

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
	 printf ("COM: \n");
	 read_nbytes(stream, 2, &longueur_section, false);
	 for (uint8_t i =0; i<longueur_section-2 ; i++){
	    read_nbytes(stream, 1, &buf, false);
	    printf("%c", buf) ;
	 }
	 // passer en section suivante : skip ?
	 break ;

      case 0xdb:			/* DQT */
	 printf ("DQT: \n");
	 if ( !unicite){
	    printf("erreur: plusieurs définitions des tables \n");
	    exit (1);
	 }
	 uint32_t nb_tables ;

	 //calcul du nombre de tables de la section
	 read_nbytes(stream, 2, &longueur_section, false);
	 printf (" longeur section: %d\n", longueur_section);
	 nb_tables = (longueur_section-2) / 65;
	 printf (" nombre de tables: %d\n", nb_tables);

	 if (nb_tables != 1) {
	    unicite = false ;
	 }

	 // ok dans le cas ou une section et plusieurs tables
	 // cas une table par section (plusieurs sections a faire)
	 quantif = malloc (nb_tables * sizeof(struct table_quantif));
	 for (uint8_t i  = 0 ; i < nb_tables; i++) {
	    printf (" Table %d\n", i);
	    read_nbits(stream, 4, &precision, false);
	    printf ("  précision: %d\n", precision);
	    read_nbits(stream, 4, & iq, false);
	    printf ("  indice: %d\n  ", iq);
	    quantif[i].prec = precision;
	    quantif[i].ind = iq ;

	    for (uint8_t j = 0; j < 64; j++) {
	       if (!(j % 8)) {
		  printf ("\n  ");
	       }
	       read_nbytes(stream, 1, &buf, false);
	       quantif[i].val[j] = buf;
	    }

	    printf ("\n");
	 }

	 // passer en section suivante : skip ?
	 break ;
      case 0xc0:			/* SOF0 */
	 printf ("SOF0: \n");

	 read_nbytes(stream, 2, &longueur_section, false);
	 printf (" longeur section: %d\n", longueur_section);
	 read_nbytes(stream, 1, &precision, false);
	 printf (" precision: %d\n", precision);

	 read_nbytes(stream, 2, &height, false);
	 printf (" height: %d\n", height);
	 read_nbytes(stream, 2, &width, false);
	 printf (" width: %d\n", width);

	 read_nbytes(stream, 1, &N, false);
	 printf (" N: %d\n", N);

	 composantes = malloc(N*sizeof(struct unit));

	 for(uint8_t i = 0; i < N; i++) {
	    printf (" Composante %d\n", i);

	    read_nbytes(stream, 1, &ic, false);
	    printf (" ic: %d\n", ic);
	    read_nbits(stream, 4, &sampling_factor_h , false);
	    printf (" horizontal sampling factor: %d\n", sampling_factor_h);
	    read_nbits(stream, 4, &sampling_factor_v, false);
	    printf (" vertical sampling factor: %d\n", sampling_factor_v);
	    read_nbytes(stream, 1, &iq, false);
	    printf (" iq: %d\n", iq);

	    composantes[i].ic = ic;
	    composantes[i].iq = iq;
	    composantes[i].sampling_factor_h = sampling_factor_h;
	    composantes[i].sampling_factor_v = sampling_factor_v;
	 }
	 break;
      case 0xc4:			/* DHT */
	 printf ("DHT: \n");
	 read_nbytes(stream, 2, &longueur_section, false);
	 printf (" longeur section: %d\n", longueur_section);
	 uint16_t nb_byte_read = 2;

	 while (nb_byte_read < longueur_section) {
	    uint32_t zeros;
	    read_nbits(stream, 3, &zeros, false);
	    if (zeros) {
	       fprintf (stderr, " erreur: Les 3 premiers bits d'information de"
			" la table de Huffman doivent valoir 0 /= %#x\n", zeros);
	       exit (1);
	    }

	    uint32_t type;
	    read_nbits(stream, 1, &type, false);
	    printf (" type: %d\n", type);

	    uint32_t indice;
	    read_nbits(stream, 4, &indice, false);
	    printf (" indice: %d\n", indice);
	    if (indice > 3) {
	       fprintf (stderr, " erreur: L'indice de la table de Huffman ne peut être %d > 3\n", indice);
	       exit (1);
	    }

	    nb_byte_read++;
	    uint16_t byteCount;

	    if (type) {
	       huff_AC[indice] = load_huffman_table(stream, &byteCount);
	       compteur_huff_AC++;
	    } else {
	       huff_DC[indice] = load_huffman_table(stream, &byteCount);
	       compteur_huff_DC++;
	    }
	    nb_byte_read += byteCount;
	    printf ("nb bytes read: %d\n", nb_byte_read);
	 }
	 break ;
      case 0xda:			/* SOS */
      {
	 printf ("SOS: \n");

	 read_nbytes(stream, 2, &longueur_section, false);
	 printf (" longeur section: %d\n", longueur_section);

	 read_nbytes(stream,1, &N, false );
	 printf (" N: %d\n", N);

	 /* Calcul du nombre blocs 8x8 dans l'image */
	 uint32_t nb_blocks_h = (height / 8) + (height % 8 ? 1 : 0);
	 uint32_t nb_blocks_v = (width / 8) + (width % 8 ? 1 : 0);
	 nb_blocks_scan = N * nb_blocks_h * nb_blocks_v;
	 printf ("nb_blocks_scan: %d\n", nb_blocks_scan);
	 uint8_t ordre_composantes[N];
	 uint8_t index;

	 for (uint8_t i = 0; i < N; i++){
	    read_nbytes(stream, 1, &ic, false);
	    printf (" indice: %d\n", ic);
	    ordre_composantes[i] = ic;
	    read_nbits(stream, 4, &ih_ac, false);
	    printf (" indice huffman AC: %d\n", ih_ac);
	    read_nbits(stream, 4, &ih_dc, false);
	    printf (" indice huffman DC: %d\n", N);

	    index = ic_to_i (composantes, N, ic);
	    composantes[index].ih_ac = ih_ac;
	    composantes[index].ih_dc = ih_dc ;
	 }
	 mcus = malloc(nb_blocks_scan*sizeof(int32_t *));
	 for (uint32_t i = 0; i < nb_blocks_scan; i++) {
	    mcus[i] = malloc(64*sizeof(int32_t));
	 }

	 int32_t pred_DC = 0;

	 /* Récupération des blocs 8*8 du fichier d'entrée selon l'échantillonnage utilisé */
	 uint32_t i = 0;
	 while (i < nb_blocks_scan) {
	    for (uint8_t c = 0; c < N; c++) {
	       printf ("N: %d\n", N);
	       index = ic_to_i (composantes, N, ordre_composantes[c]);
	       printf ("index: %d\n", index);
	       for (uint8_t j = 0;
		    j < composantes[index].sampling_factor_h*composantes[index].sampling_factor_v;
		    j++) {
		  printf ("unpack %d | j = %d | pred_DC = %d\n", i, j, pred_DC);
		  if (i == 1188) {
		     uint32_t buf;
		     read_nbytes(stream, 4, &buf, false);
		     printf ("\n");
		     exit (1);
		  }
		  unpack_block(stream, huff_DC[composantes[index].ih_dc], &pred_DC,
			       huff_AC[composantes[index].ih_ac], mcus[i++]);
	       }
	    }
	 }
      }
	 break ;
      case 0xd9:		/* EOI */
	 printf ("EOI: fin de fichier  \n");
	 return 0;
	 break;
      default :
	 printf("erreur, marqueur de section non reconnu \n");
	 exit (1);
      }
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
