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
#include <libgen.h>

/* MARQUEURS DE SECTIOn */
#define APP0 0xe0
#define COM 0xfe
#define DQT 0xdb
#define SOF0 0xc0
#define DHT 0xc4
#define SOS 0xda
#define EOI 0xd9

/* STRUCTURES */

struct table_quantif {
   uint8_t val[64] ;
   uint8_t ind ;
};

struct unit{
   uint8_t ic;
   uint8_t iq;
   uint8_t sampling_factor_h ;
   uint8_t sampling_factor_v ;
   uint8_t ih_ac;		/* indice de huffman AC */
   uint8_t ih_dc; 		/* indice de huffman DC  */
};

/* PROTOTYPES */
/* Vérifie que le fichier d'entrée à l'extension jpeg ou jpg et génère le nom en .tiff */
char *check_and_gen_name(const char *input_name);
/* Crée un bloc de sfh*sfv blocs par concaténation pour upsampling  */
uint8_t *rearrange_blocs(uint8_t **blocs, uint32_t i, uint8_t sfh, uint8_t sfv);
/* Affiche un bloc 8*8 */
void print_block(uint8_t *bloc, uint32_t num_bloc);
/* Affiche une mcu de taille sfh*sfv blocs 8*8 */
void print_mcu(uint8_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv);
/* Lit nb_bits bits dans le fichier d'entrée et lève une erreur si impossible */
void read_nbits(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool byte_stuffing);
/* Lit nb_bytes octets dans le fichier d'entrée et lève une erreur si impossible */
void read_nbytes(struct bitstream *stream, uint8_t nb_bytes, uint32_t *dest, bool byte_stuffing);
/* Permet de trouver l'index de composante correspondant à l'indice ic en cas de désordre */
uint8_t ic_to_i(struct unit *composantes, uint32_t N, uint32_t ic);
/* Vérifie qu'une allocation a bien été effectué */
void check_alloc_main(void* ptr);
void Y_to_Grayscale(uint8_t  *mcu_Y, uint32_t *mcu_RGB,
		    uint32_t nb_blocks_h, uint32_t nb_blocks_v);

int main(int argc, char *argv[]){

   /* vérification de l'entrée */
   if ( argc != 2 ) {
      fprintf(stderr, "Veuillez entrer le fichier JPEG à décoder en argument. \n");
      exit (1);
   }

   /* Vérification de la validité du nom et création du nom du fichier de sortie */
   char *output_name = check_and_gen_name (argv[1]);

   /* Initialisation du bitstream */
   struct bitstream *stream = create_bitstream(argv[1]);
   if (!stream) {
      fprintf(stderr, "Impossible de créer le bitstream. Le fichier spécifié n'existe pas.\n");
      exit (1);
   }

   /* Variables de table de Huffman */
   uint8_t compteur_huff_AC = 0 ; /* Nombre de tables AC */
   uint8_t compteur_huff_DC = 0; /* Nombre de tables DC */
   struct huff_table *huff_AC[4]; /* Tables AC */
   struct huff_table *huff_DC[4]; /* Tables DC */

   /* Variables de propriétés d'images et d'échantillonnage */
   uint32_t nb_mcus;		/* Nombre de MCUs YCbCr total */
   uint32_t nb_mcus_RGB;	/* Nombre de MCUs RGB */
   uint32_t nb_blocks_scan;	/* Nombre de blocs 8*8 dans l'image jpeg compressée */
   uint8_t sampling = 0;	/* Facteur d'échantillonnage (Donne la taille des MCUs) */
   int32_t **blocs ;		    /* Conteneur des blocs 8*8 extraits de l'image JFIF */
   uint32_t height;		    /* Hauteur de l'image */
   uint32_t width ;		    /* Largeur de l'image */
   bool decoded_sos = false;	    /* Indique si on a déjà décodé la section SOS, et donc récupéré les données d'image */

   /* Variables de quantification */
   struct table_quantif *quantif = NULL; /* Conteneur de tables d'échantillonnage */
   uint32_t nb_tables = 0 ;		 /* Nombre de tables */
   bool unicite = true ; 	/* Vérifie qu'on ne peut avoir plusieures sections DQT de plus d'une table */

   /* Variables de composantes */
   struct unit *composantes = NULL; /* Conteneur d'attributs des composantes YCbCr (ou autres si extension ?) */
   uint32_t N;			    /* Nombre de composantes */
   uint8_t *ordre_composantes;	    /* Ordre d'apparition des composantes dans le fichier compressé */
   uint8_t index;		/* Indice de la composante d'indice ic dans le tableau composantes en cas de désordre */

   /* Attributs de section redondants et conteneurs */
   uint32_t buf;		/* Conteneur brut */
   uint32_t longueur_section ;	/* Longueur de la section courante */
   uint32_t ic;			/* Indice d'une composante */
   uint32_t iq;			/* Indice d'une table de quantification */
   uint32_t sampling_factor_h;	/* Facteur d'échantillonnage horizontal d'une composante */
   uint32_t sampling_factor_v;        /* Facteur d'échantillonnage horizontal d'une composante */
   uint32_t ih_ac;		      /* Indice de table de Huffman ac */
   uint32_t ih_dc;		      /* Indice de table de Huffman dc */
   uint32_t precision;		      /* Précision. Inutilisée */
   uint32_t marqueur;		      /* Marqueur de section JPEG */

   /* EXTRACTION DE L ENTETE */

   /* Vérification de présence du marqueur SOI */
   read_nbytes(stream, 2, &buf, false);
   if (buf != 0xffd8) {
      fprintf(stderr, "Le fichier ne commence pas par un SOI mais 0x%x !\n", buf);
      exit (1);
   }

   /* Lecture des sections jusqu'à récupération des blocs */
   while (!decoded_sos) {

      // Lecture du premier marqueur de section
      read_nbytes(stream, 1, &buf, false); /* On ignore 0xff */
      read_nbytes(stream, 1, &marqueur, false); /* Marqueur effectif */

      switch(marqueur) {
      case APP0 :		/* 0xe0 : encapsulation JFIF */
	 /* printf ("APP0: \n"); */
	 read_nbytes(stream, 2, &longueur_section, false);
	 /* printf (" longeur section: %d\n", longueur_section); */

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

	 /* Lecture des données additionnelles */
	 uint32_t version;
	 read_nbytes(stream, 2, &version, false);
	 /* printf (" version JFIF: %d\n", version); */

	 uint32_t densityUnits;
	 read_nbytes(stream, 1, &densityUnits, false);
	 /* printf ("  density unit: %d\n", densityUnits); */

	 uint32_t xDensity;
	 read_nbytes(stream, 2, &xDensity, false);
	 /* printf ("  x density: %d\n", xDensity); */

	 uint32_t yDensity;
	 read_nbytes(stream, 2, &yDensity, false);
	 /* printf ("  y density: %d\n", yDensity); */

	 uint32_t tw;
	 read_nbytes(stream, 1, &tw, false);
	 /* printf ("  thumbnail width: %d\n", tw); */

	 uint32_t th;
	 read_nbytes(stream, 1, &th, false);
	 /* printf ("  thumbnail height: %d\n", th); */

	 uint32_t t_data;
	 for (uint8_t i = 0; i < tw*th ; i++){
	    read_nbytes(stream, 3, &t_data, false);
	 }
	 break ;

      case COM :			/* 0xfe */
	 /* printf ("COM: \n"); */
	 read_nbytes(stream, 2, &longueur_section, false);
	 for (uint8_t i =0; i<longueur_section-2 ; i++){
	    read_nbytes(stream, 1, &buf, false);
	    /* printf("%c", buf) ; */
	 }
	    /* printf("\n") ; */
	 break ;

      case DQT:			/* 0xdb */
	 /* printf ("DQT: \n"); */

	 if (!unicite){
	    /* printf("erreur: plusieurs définitions des tablesde quantification \n"); */
	    exit (1);
	 }

	 //* calcul du nombre de tables de la section */
	 read_nbytes(stream, 2, &longueur_section, false);
	 /* printf (" longeur section: %d\n", longueur_section); */
	 uint32_t nb_tables_section = (longueur_section-2) / 65;
	 /* printf (" nombre de tables: %d\n", nb_tables_section); */

	 if (nb_tables_section != 1) {
	    unicite = false ;
	 }

	 if (quantif) {
	    /* Ce n'est pas la premiere section DQT rencontrée,
	       on alloue un tableau de struct quantif plus grand */
	    struct table_quantif *temp = malloc ((nb_tables_section + nb_tables) * sizeof(struct table_quantif));
	    check_alloc_main (temp);
	    /* Recopie des ancients elements de quantif dans temp */
	    for (uint8_t i  = 0 ; i < nb_tables; i++) {
	       temp[i].ind = quantif[i].ind;
	       for (uint8_t j = 0; j < 64; j++) {
		  temp[i].val[j] = quantif[i].val[j];
	       }
	    }
	    /* Libération de l'ancien conteneur de tables */
	    free (quantif);
	    quantif = temp;
	 } else {
	    quantif = malloc (nb_tables_section * sizeof(struct table_quantif));
	    check_alloc_main (quantif);
	 }

	 for (uint8_t i  = nb_tables ; i < nb_tables + nb_tables_section; i++) {
	    /* printf (" Table %d\n", i); */
	    read_nbits(stream, 4, &precision, false);
	    /* printf ("  précision: %d\n", precision); */
	    read_nbits(stream, 4, & iq, false);
	    /* printf ("  indice: %d\n  ", iq); */
	    quantif[i].ind = iq ;

	    for (uint8_t j = 0; j < 64; j++) {
	       read_nbytes(stream, 1, &buf, false);
	       quantif[i].val[j] = buf;
	    }

	 }

	 nb_tables += nb_tables_section;
      break ;

      case SOF0:			/* 0xc0 */
	 /* printf ("SOF0: \n"); */

	 read_nbytes(stream, 2, &longueur_section, false);
	 /* printf (" longeur section: %d\n", longueur_section); */
	 read_nbytes(stream, 1, &precision, false);
	 /* printf (" precision: %d\n", precision); */

	 read_nbytes(stream, 2, &height, false);
	 /* printf (" height: %d\n", height); */
	 read_nbytes(stream, 2, &width, false);
	 /* printf (" width: %d\n", width); */

	 read_nbytes(stream, 1, &N, false);
	 /* printf (" N: %d\n", N); */

	 composantes = malloc(N*sizeof(struct unit));
	 check_alloc_main (composantes);

	 for(uint8_t i = 0; i < N; i++) {
	    /* printf (" Composante %d\n", i); */

	    read_nbytes(stream, 1, &ic, false);
	    /* printf (" ic: %d\n", ic); */
	    read_nbits(stream, 4, &sampling_factor_h , false);
	    /* printf (" horizontal sampling factor: %d\n", sampling_factor_h); */
	    read_nbits(stream, 4, &sampling_factor_v, false);
	    /* printf (" vertical sampling factor: %d\n", sampling_factor_v); */
	    read_nbytes(stream, 1, &iq, false);
	    /* printf (" iq: %d\n", iq); */

	    composantes[i].ic = ic;
	    composantes[i].iq = iq;
	    composantes[i].sampling_factor_h = sampling_factor_h;
	    composantes[i].sampling_factor_v = sampling_factor_v;
	 }

	 sampling = composantes[0].sampling_factor_h * composantes[0].sampling_factor_v;

	 break;
      case DHT:			/* 0xc4 */
	 /* printf ("DHT: \n"); */
	 read_nbytes(stream, 2, &longueur_section, false);
	 /* printf (" longeur section: %d\n", longueur_section); */
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
	    /* printf (" type: %d\n", type); */

	    uint32_t indice;
	    read_nbits(stream, 4, &indice, false);
	    /* printf (" indice: %d\n", indice); */
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
	    /* printf ("nb bytes read: %d\n", nb_byte_read); */
	 }
	 break ;

      case SOS:			/* 0xda */
      {
	 /* printf ("SOS: \n"); */

	 read_nbytes(stream, 2, &longueur_section, false);
	 /* printf (" longeur section: %d\n", longueur_section); */

	 read_nbytes(stream,1, &N, false );
	 /* printf (" N: %d\n", N); */

	 ordre_composantes = malloc (N * sizeof(uint8_t));
	 check_alloc_main (composantes);

	 for (uint8_t i = 0; i < N; i++){
	    read_nbytes(stream, 1, &ic, false);
	    /* printf (" indice: %d\n", ic); */
	    ordre_composantes[i] = ic;
	    read_nbits(stream, 4, &ih_ac, false);
	    /* printf (" indice huffman AC: %d\n", ih_ac); */
	    read_nbits(stream, 4, &ih_dc, false);
	    /* printf (" indice huffman DC: %d\n", N); */

	    index = ic_to_i (composantes, N, ic);
	    composantes[index].ih_ac = ih_ac;
	    composantes[index].ih_dc = ih_dc ;
	 }

	 /* Calcul du nombre blocs 8x8 dans l'image */
	 uint32_t nb_mcus_h = (width / (8*composantes[0].sampling_factor_h))
	    + (width % (8*composantes[0].sampling_factor_h) ? 1 : 0);
	 uint32_t nb_mcus_v = (height / (8*composantes[0].sampling_factor_v))
	    + (height % (8*composantes[0].sampling_factor_v) ? 1 : 0);
	 /* printf ("nb_mcus_h: %d | nb_mcus_v: %d\n", nb_mcus_h, nb_mcus_v); */
	 nb_mcus_RGB = nb_mcus_h * nb_mcus_v;

	 nb_blocks_scan = 0;
	 for (uint8_t i = 0; i < N; i++) {
	    nb_blocks_scan += nb_mcus_RGB * composantes[i].sampling_factor_h * composantes[i].sampling_factor_v;
	 }

	 nb_mcus = nb_mcus_RGB * N;
	 /* printf ("nb_mcus_rgb: %d\n", nb_mcus_RGB); */
	 /* printf ("nb_mcus: %d\n", nb_mcus); */
	 /* printf ("nb_blocks_scan: %d\n", nb_blocks_scan); */

	 uint32_t bits_inutiles;
	 read_nbytes(stream, 3, &bits_inutiles, false);

	 blocs = malloc(nb_blocks_scan*sizeof(int32_t *));
	 check_alloc_main (blocs);
	 for (uint32_t i = 0; i < nb_blocks_scan; i++) {
	    blocs[i] = malloc(64*sizeof(int32_t));
	    check_alloc_main (blocs[i]);
	 }

	 int32_t pred_DC[N];
	 for (uint32_t i = 0; i < N; i++)
	    pred_DC[i] = 0;

	 /* printf ("UNPACK:  \n"); */

	 /* Récupération des blocs 8*8 du fichier d'entrée selon l'échantillonnage utilisé */
	 uint32_t i = 0;
	 while (i < nb_blocks_scan) {
	    for (uint8_t c = 0; c < N; c++) {
	       /* printf ("N: %d\n", N); */
	       index = ic_to_i (composantes, N, ordre_composantes[c]);
	       /* printf ("index: %d\n", index); */
	       for (uint8_t j = 0;
		    j < composantes[index].sampling_factor_h*composantes[index].sampling_factor_v;
		    j++) {
		  /* printf ("unpack %d | j = %d | pred_DC = %d | index = %d\n", i, j, pred_DC[index], index); */
		  unpack_block(stream, huff_DC[composantes[index].ih_dc], &pred_DC[index],
			       huff_AC[composantes[index].ih_ac], blocs[i]);
		  i++;
		  /* print_block (blocs[i-1], i-1); */
	       }
	    }
	 }

	 decoded_sos = true;
      }
      break ;

      case EOI:		/* 0xd9 */
	 /* printf ("EOI: fin de fichier  \n"); */
	 return 0;

      default :
	 fprintf(stderr, "erreur, marqueur de section non reconnu \n");
	 exit (1);
      }
   }

   /* IQZZ */
   /* printf ("IQZZ: \n"); */
   int32_t **blocs_iqzz = malloc(nb_blocks_scan*sizeof(int32_t *));
   check_alloc_main (blocs_iqzz);
   for (uint32_t i = 0; i < nb_blocks_scan; i++) {
      blocs_iqzz[i] = malloc(64*sizeof(int32_t));
      check_alloc_main (blocs_iqzz[i]);
   }

   uint32_t i = 0;
   while (i < nb_blocks_scan) {
      for (uint8_t c = 0; c < N; c++) {
	 index = ic_to_i (composantes, N, ordre_composantes[c]);
	 for (uint8_t j = 0;
	      j < composantes[index].sampling_factor_h*composantes[index].sampling_factor_v;
	      j++) {
	    /* printf ("iqzz %d | iq = %d\n", i, composantes[index].iq); */
	    /* Tables de quantif peut être pas rangées par indice */
	    iqzz_block (blocs[i], blocs_iqzz[i], quantif[composantes[index].iq].val);
	    /* print_block (blocs_iqzz[i], i); */
	    i++;
	 }
      }
   }

   for (uint32_t i = 0; i < nb_blocks_scan; i++)
      free (blocs[i]);
   free (blocs);

   /* IDCT */
   /* printf ("IDCT: \n"); */
   uint8_t **blocs_idct = malloc(nb_blocks_scan*sizeof(uint8_t *));
   check_alloc_main (blocs_idct);
   for (uint32_t i = 0; i < nb_blocks_scan; i++) {
      blocs_idct[i] = malloc(64*sizeof(uint8_t));
      check_alloc_main (blocs_idct[i]);
   }

   for (uint32_t i = 0; i < nb_blocks_scan; i++) {
      /* printf ("idct %d\n", i); */
      idct_block(blocs_iqzz[i], blocs_idct[i]);
      /* print_block (blocs_idct[i], i); */
   }

   for (uint32_t i = 0; i < nb_blocks_scan; i++)
      free (blocs_iqzz[i]);
   free (blocs_iqzz);


   /* UPSAMPLING */
   /* TODO: Gerer indices non fixés */
   /* printf ("UPSAMPLING: \n"); */
   uint8_t ***mcus = malloc(nb_mcus_RGB*sizeof(uint8_t **));
   check_alloc_main (mcus);
   for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
      mcus[i] = malloc(N*sizeof(uint8_t *));
      check_alloc_main (mcus[i]);
      for (uint32_t j = 0; j < N; j++) {
	 mcus[i][j] = malloc (64*sampling*sizeof(uint8_t));
	 check_alloc_main (mcus[i][j]);
      }
   }

   i = 0;
   uint32_t k = 0;
   uint32_t l = 0;
   index = 0;
   while (i < nb_blocks_scan) {
      /* printf ("upsampling %d | ic = %d | k = %d \n", i, index, k); */
      /* Rearrangement des blocs pour upsampling */
      uint8_t *up_blocs = rearrange_blocs (blocs_idct, i,
					   composantes[index].sampling_factor_h,
					   composantes[index].sampling_factor_v);

      /* for (uint32_t l = 0; */
      /* 	   l < composantes[index].sampling_factor_h * composantes[index].sampling_factor_v; */
      /* 	   l++) { */
      /* 	 print_block (blocs_idct[i+l], i+l); */
      /* } */

      /* print_mcu (up_blocs, i, composantes[index].sampling_factor_h, composantes[index].sampling_factor_v); */

      upsampler (up_blocs, composantes[index].sampling_factor_h, composantes[index].sampling_factor_v,
		mcus[l][index], composantes[0].sampling_factor_h,  composantes[0].sampling_factor_v);

      free (up_blocs);

      /* printf ("num_blocks_in_h: %d | _v: %d\n", */
      /* 	      composantes[index].sampling_factor_h, composantes[index].sampling_factor_v); */
      /* printf ("num_blocks_out_h: %d | _v: %d\n", */
      /* 	      composantes[0].sampling_factor_h, composantes[0].sampling_factor_v); */
      /* print_mcu (mcus[k], k, composantes[0].sampling_factor_h, composantes[0].sampling_factor_v); */


      if (++k == N) {
	 l++;
	 k = 0;
      }

      i += composantes[index].sampling_factor_h * composantes[index].sampling_factor_v;
      index = (index + 1) % N;
   }

   for (uint32_t i = 0; i < nb_blocks_scan; i++)
      free (blocs_idct[i]);
   free (blocs_idct);

   /* YCbCr to ARGB ou grayscale */
   /* printf ("YCbCr2ARGB: \n"); */
   /* printf ("N: %d\n", N); */
   uint32_t **mcus_gray = NULL;
   uint32_t **mcus_RGB = NULL;
   if (N == 1)  {
      mcus_gray = malloc(nb_mcus_RGB*sizeof(uint32_t *));
      check_alloc_main (mcus_gray);
      for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
	 mcus_gray[i] = malloc(64*sampling*sizeof(uint32_t));
	 check_alloc_main (mcus_gray[i]);
      }

      k = 0;
      for (uint32_t i = 0; i < nb_mcus_RGB; i ++) {
	 /* printf ("Y_to_grayscale %d  \n", i); */
	 Y_to_Grayscale(mcus[i][0], mcus_gray[i],
			composantes[0].sampling_factor_h, composantes[0].sampling_factor_v);
      }
   } else if (N == 3) {
      mcus_RGB = malloc(nb_mcus_RGB*sizeof(uint32_t *));
      check_alloc_main (mcus_RGB);
      for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
	 mcus_RGB[i] = malloc(64*sampling*sizeof(uint32_t));
	 check_alloc_main (mcus_RGB[i]);
      }

      k = 0;
      for (uint32_t i = 0; i < nb_mcus_RGB; i ++) {
	 /* printf ("YCbCr2ARGB %d | k = %d \n", i, k); */
	 YCbCr_to_ARGB(mcus[i], mcus_RGB[k++],
		       composantes[0].sampling_factor_h, composantes[0].sampling_factor_v);
      }
   }

   /* ecriture dans le TIFF */
   /* printf ("TIFF: \n"); */
   struct tiff_file_desc *tfd = init_tiff_file(output_name, width, height, 8*composantes[0].sampling_factor_v);

   if (N == 1) {
      for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
	 /* printf ("Write TIFF %d\n", i); */
	 write_tiff_file(tfd, mcus_gray[i],  composantes[0].sampling_factor_h,  composantes[0].sampling_factor_v);
      }
   } else if (N == 3) {
      for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
	 /* printf ("Write TIFF %d\n", i); */
	 write_tiff_file(tfd, mcus_RGB[i],  composantes[0].sampling_factor_h,  composantes[0].sampling_factor_v);
      }
   }

   /* Libération de variables allouées sur le tas */
   close_tiff_file (tfd);
   free_bitstream(stream);

   free (output_name);

   for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
      for (uint32_t j = 0; j < N; j++)
	 free(mcus[i][j]);
      free (mcus[i]);
   }
   free (mcus);

   if (N == 1) {
      for (uint32_t i = 0; i < nb_mcus_RGB; i++)
	 free (mcus_gray[i]);
      free (mcus_gray);
   } else if (N == 3) {
      for (uint32_t i = 0; i < nb_mcus_RGB; i++)
	 free (mcus_RGB[i]);
      free (mcus_RGB);
   }

   for (uint32_t i = 0; i < compteur_huff_AC; i++)
      free_huffman_table (huff_AC[i]);
   for (uint32_t i = 0; i < compteur_huff_DC; i++)
      free_huffman_table (huff_DC[i]);

   free (composantes);
   free (ordre_composantes);
   free (quantif);

   return 0;
}


char *check_and_gen_name(const char *input_name)
{
   /* On repere l'extension actuelle */
   char *ext = strrchr (input_name, '.');

   if (strcmp(ext, ".jpeg") && strcmp(ext, ".jpg")) {
      fprintf(stderr, "erreur: L'extension de fichier n'est ni .jpeg, ni .jpg !\n");
      exit (1);
   }

   /* On remplace l'extension .jp(e)g par .tiff */
   char *tiff_ext = ".tiff";
   char *output_name = calloc ((strlen(input_name) - strlen(ext) + strlen(tiff_ext) + 1), sizeof(char));
   check_alloc_main (output_name);
   strncpy (output_name, input_name, strlen(input_name) - strlen(ext));
   strcat (output_name, tiff_ext);
   /* printf ("output_name: %s\n", output_name); */

   return output_name;
}

void print_block(uint8_t *bloc, uint32_t num_bloc)
{
   printf ("bloc numéro %d\n", num_bloc);
   for (uint32_t i = 0; i < 8; i++) {
      for (uint32_t j = 0; j < 8; j++) {
	 printf ("%d ", bloc[8*i + j]);
      }
      printf ("\n ");
   }
   printf ("\n");
}

void print_mcu(uint8_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv)
{
   printf ("mcu numéro %d\n", num_mcu);
   for (uint32_t i = 0; i < (uint32_t)(64*sfv*sfh); i++) {
      if (!(i % (sfh * 8)))
	 printf ("\n");
      printf ("%d ", mcu[i]);
   }
   printf ("\n");
}

void read_nbits(struct bitstream *stream, uint8_t nb_bits, uint32_t *dest, bool byte_stuffing)
{
   uint8_t nbLus = read_bitstream(stream, nb_bits, dest, byte_stuffing);
   if (nbLus != nb_bits)
      fprintf(stderr, "Erreur lecture bitstream : %d / %d\n", nbLus, nb_bits);

   /* printf ("%#.2x ", *dest); */
}

void read_nbytes(struct bitstream *stream, uint8_t nb_bytes, uint32_t *dest, bool byte_stuffing)
{
   uint8_t nbLus = read_bitstream(stream, 8*nb_bytes, dest, byte_stuffing);
   if (nbLus != 8*nb_bytes)
      fprintf(stderr, "Erreur lecture bitstream : %d / %d\n", nbLus, nb_bytes * 8);

   /* printf ("%#.2x ", *dest); */
}

/* Renvoie l'indice correspondant à l'élément d'ic en argument
   dans le tableau composantes de N élements */
uint8_t ic_to_i(struct unit *composantes, uint32_t N, uint32_t ic)
{
   for (uint8_t i = 0; i < N; i++)
      if (composantes[i].ic == ic)
	 return i;

   fprintf  (stderr, "erreur: l'ic %d n'existe pas parmis les composantes\n", ic);
   exit (1);
}

uint8_t *rearrange_blocs(uint8_t **blocs, uint32_t i, uint8_t sfh, uint8_t sfv)
{
   uint8_t *out = malloc (64*sfh*sfv*sizeof(uint8_t));
   check_alloc_main (out);
   /* Pour chaque bloc de la matrice finale */
   for (uint32_t x = 0; x < sfh*sfv; x++) {
      for (uint32_t j = 0; j < 64; j++) {
	    out[64*x + j] = blocs[i + x][j];
	 }
   }

   return out;
}

void Y_to_Grayscale(uint8_t  *mcu_Y, uint32_t *mcu_RGB,
		    uint32_t nb_blocks_h, uint32_t nb_blocks_v)
{
   /* Pour chaque pixel des MCU Y, passage en uint32_t */
   for (uint32_t i = 0;
	i < (8 * nb_blocks_v) * (8 * nb_blocks_h); /* Nb elts mcu */
	i++) {
      mcu_RGB[i]  = 0;	/* Mise à 0, nécessaire pour A */
      mcu_RGB[i] |= mcu_Y[i] << 16;
      mcu_RGB[i] |= mcu_Y[i] << 8;
      mcu_RGB[i] |= mcu_Y[i];
   }
}
void check_alloc_main(void* ptr)
{
   if (!ptr) {
      fprintf (stderr, "alloc error: OUT OF MEMORY\n");
   }
}
