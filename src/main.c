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

/* struct mcu{ */
/*    uint32_t ic; */
/*    uint32_t ih_dc ; */
/*    uint32_t ih_ac; */
/* }; */

char *check_and_gen_name(char *input_name)
{
   char *pch = strrchr (input_name, '.');
   uint16_t pos_pt = pch - input_name ;
   /* Récupération de l'extension seule, max 6 chars pour ".jpeg" */
   char *ext = malloc (6 * sizeof(char));
   uint16_t j = 0;
   for (uint16_t i = pos_pt; (input_name[i] != '\0') && (j < 5); i++) {
      ext[j++] = input_name[i];
   }
   ext[j] = '\0';
   /* printf ("ext = %s\n", ext); */

   if (strcmp(ext, ".jpeg") && strcmp(ext, ".jpg")) {
      fprintf(stderr, "erreur: L'extension de fichier n'est ni .jpeg, ni .jpg !\n");
      exit (1);
   }

   /* On remplace l'extension .jpeg par .tiff */
   char *output_name = malloc (sizeof(char) * (strlen(input_name)));
   strcpy (output_name, input_name);
   output_name[pos_pt] = '\0';
   char *tiff_ext = ".tiff";
   strcat (output_name, tiff_ext);
   /* printf ("output_name: %s\n", output_name); */

   return output_name;
}

void print_block (uint8_t *bloc, uint32_t num_bloc)
{
   /* printf ("bloc numéro %d\n", num_bloc); */
   for (uint32_t i = 0; i < 8; i++) {
      for (uint32_t j = 0; j < 8; j++) {
	 /* printf ("%d ", bloc[8*i + j]); */
      }
      /* printf ("\n "); */
   }
   /* printf ("\n"); */
}

void print_mcu (uint8_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv)
{
   /* printf ("mcu numéro %d\n", num_mcu); */
   for (uint32_t i = 0; i < 64*sfv*sfh; i++) {
      /* if (!(i % (sfh * 8))) */
	 /* printf ("\n"); */
      /* printf ("%d ", mcu[i]); */
   }
   /* printf ("\n"); */
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
   /* if (!composantes) */
      /* printf ("dégout\n"); */

   for (uint8_t i = 0; i < N; i++)
      if (composantes[i].ic == ic)
	 return i;

   fprintf  (stderr, "erreur: l'ic %d n'existe pas parmis les composantes\n", ic);
   exit (1);
}

uint8_t *rearrange_blocs(uint8_t **blocs, uint32_t i, uint8_t sfh, uint8_t sfv)
{
   uint8_t *out = malloc (64*sfh*sfv*sizeof(uint8_t));
   /* Pour chaque bloc de la matrice finale */
   for (uint32_t x = 0; x < sfh*sfv; x++) {
      for (uint32_t j = 0; j < 64; j++) {
	    out[64*x + j] = blocs[i + x][j];
	 }
   }

   return out;
}

int main(int argc, char *argv[]){

   // vérification de l'entrée
   if ( argc != 2 ) {
      fprintf(stderr, "Veuillez entrer le fichier JPEG à décoder en argument. \n");
      exit (1);
   }

   /* Vérification de la validité du nom et création du nom du fichier de sortie */
   char *output_name = check_and_gen_name (argv[1]);

   struct bitstream *stream = create_bitstream(argv[1]);
   if (!stream) {
      fprintf(stderr, "Impossible de créer le bitstream. Le fichier spécifié n'existe pas.\n");
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

   uint32_t nb_mcus;
   uint32_t nb_mcus_RGB;
   uint32_t nb_blocks_scan;
   uint8_t sampling = 0;

   struct table_quantif *quantif = NULL;
   uint32_t nb_tables = 0 ;

   struct unit *composantes = NULL;
   int32_t **blocs ;

   uint32_t precision;
   uint32_t height;
   uint32_t width ;

   uint32_t N;
   uint8_t *ordre_composantes;
   uint8_t index;

   uint32_t ic;
   uint32_t iq;
   uint32_t sampling_factor_h;
   uint32_t sampling_factor_v;

   uint32_t ih_ac;
   uint32_t ih_dc;

   /* uint32_t unused; */

   // saut marqueur SOI
   read_nbytes(stream, 2, &buf, false);
   if (buf != 0xffd8) {
      fprintf(stderr, "Le fichier ne commence pas par un SOI mais 0x%x !\n", buf);
      exit (1);
   }

   bool decoded_sos = false;

   while (!decoded_sos) {

      // Lecture du premier marqueur de section
      read_nbytes(stream, 1, &buf, false); /* On passe 0xff */
      read_nbytes(stream, 1, &buf, false);
      /* printf ("\n"); */

      switch(buf){

	 //APP0 : encapsulation JFIF
      case 0xe0 :
	 /* printf ("APP0: \n"); */
	 read_nbytes(stream, 2, &longueur_section, false);
	 longueur_section = longueur_section;
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

	 // PASSER A LA FIN DE LA SECTION (SKIP ou READBYTES ?)
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

      case 0xFE :			/* COM */
	 /* printf ("COM: \n"); */
	 read_nbytes(stream, 2, &longueur_section, false);
	 for (uint8_t i =0; i<longueur_section-2 ; i++){
	    read_nbytes(stream, 1, &buf, false);
	    /* printf("%c", buf) ; */
	 }
	    /* printf("\n") ; */
	 break ;

      case 0xdb:			/* DQT */
	 /* printf ("DQT: \n"); */

	 if (!unicite){
	    /* printf("erreur: plusieurs définitions des tablesde quantification \n"); */
	    exit (1);
	 }

	 //calcul du nombre de tables de la section
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
	 }

	 for (uint8_t i  = nb_tables ; i < nb_tables + nb_tables_section; i++) {
	    /* printf (" Table %d\n", i); */
	    read_nbits(stream, 4, &precision, false);
	    /* printf ("  précision: %d\n", precision); */
	    read_nbits(stream, 4, & iq, false);
	    /* printf ("  indice: %d\n  ", iq); */
	    quantif[i].ind = iq ;

	    for (uint8_t j = 0; j < 64; j++) {
	       /* if (!(j % 8)) { */
	       /* 	  printf ("\n  "); */
	       /* } */
	       read_nbytes(stream, 1, &buf, false);
	       quantif[i].val[j] = buf;
	    }

	    /* printf ("\n"); */
	 }

	 nb_tables += nb_tables_section;
      break ;
   case 0xc0:			/* SOF0 */
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
      case 0xc4:			/* DHT */
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
      case 0xda:			/* SOS */
      {
	 /* printf ("SOS: \n"); */

	 read_nbytes(stream, 2, &longueur_section, false);
	 /* printf (" longeur section: %d\n", longueur_section); */

	 read_nbytes(stream,1, &N, false );
	 /* printf (" N: %d\n", N); */

	 ordre_composantes = malloc (N * sizeof(uint8_t));

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
	 for (uint32_t i = 0; i < nb_blocks_scan; i++) {
	    blocs[i] = malloc(64*sizeof(int32_t));
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
      case 0xd9:		/* EOI */
	 /* printf ("EOI: fin de fichier  \n"); */
	 return 0;
	 break;
      default :
	 fprintf(stderr, "erreur, marqueur de section non reconnu \n");
	 exit (1);
      }
   }

   /* IQZZ */
   /* printf ("IQZZ: \n"); */
   int32_t **blocs_iqzz = malloc(nb_blocks_scan*sizeof(int32_t *));
   for (uint32_t i = 0; i < nb_blocks_scan; i++) {
      blocs_iqzz[i] = malloc(64*sizeof(int32_t));
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
   for (uint32_t i = 0; i < nb_blocks_scan; i++) {
      blocs_idct[i] = malloc(64*sizeof(uint8_t));
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
   for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
      mcus[i] = malloc(N*sizeof(uint8_t *));
      for (uint32_t j = 0; j < N; j++)
	 mcus[i][j] = malloc (64*sampling*sizeof(uint8_t));
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
      index = (index + 1) % 3;
   }

   for (uint32_t i = 0; i < nb_blocks_scan; i++)
      free (blocs_idct[i]);
   free (blocs_idct);

   /* YCbCr to ARGB */
   /* printf ("YCbCr2ARGB: \n"); */
   uint32_t **mcus_RGB = malloc(nb_mcus_RGB*sizeof(uint32_t *));
   for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
      mcus_RGB[i] = malloc(64*sampling*sizeof(uint32_t));
   }

   k = 0;
   for (uint32_t i = 0; i < nb_mcus_RGB; i ++) {
      /* printf ("YCbCr2ARGB %d | k = %d \n", i, k); */
      YCbCr_to_ARGB(mcus[i], mcus_RGB[k++],
		    composantes[0].sampling_factor_h, composantes[0].sampling_factor_v);
   }

   for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
      for (uint32_t j = 0; j < N; j++)
	 free(mcus[i][j]);
      free (mcus[i]);
   }
   free (mcus);

   /* ecriture dans le TIFF */
   /* printf ("TIFF: \n"); */
   struct tiff_file_desc *tfd = init_tiff_file(output_name, width, height, 8*composantes[0].sampling_factor_v);

   for (uint32_t i = 0; i < nb_mcus_RGB; i++) {
      /* printf ("Write TIFF %d\n", i); */
      write_tiff_file(tfd, mcus_RGB[i],  composantes[0].sampling_factor_h,  composantes[0].sampling_factor_v);
   }

   close_tiff_file (tfd);

   free (output_name);

   for (uint32_t i = 0; i < nb_mcus_RGB; i++)
      free (mcus_RGB[i]);
   free (mcus_RGB);

   for (uint32_t i = 0; i < compteur_huff_AC; i++)
      free_huffman_table (huff_AC[i]);
   for (uint32_t i = 0; i < compteur_huff_DC; i++)
      free_huffman_table (huff_DC[i]);

   free_bitstream(stream);
   free (composantes);
   free (ordre_composantes);
   free (quantif);

   return 0;
}
