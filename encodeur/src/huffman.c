#include "huffman.h"

#define ZRL 0xf0
#define EOB 0x00

enum AC_DC {
   AC,
   DC
};

struct abr {
   uint32_t sym;
   bool est_feuille;
   struct abr *gauche, *droite;
};

struct huff_table {
   struct abr *huff_tree;
};

void affiche_huffman_rec (struct abr *huff, uint8_t code[16], uint8_t nbits)
{
   /* Huff est une feuille, affichage du code et de la valeur du noeud */
   if (huff->est_feuille) {
      for (uint8_t i = 0; i < nbits; i++)
	 printf ("%d", code[i]);

      printf (" -> 0x%x\n", huff->sym);
   } else {
      /* Visite fils gauche */
      if (huff->gauche) {
	 code[nbits] = 0;
	 affiche_huffman_rec (huff->gauche, code, nbits+1);
      }

      /* Visite fils droit */
      if (huff->droite) {
	 code[nbits] = 1;
	 affiche_huffman_rec (huff->droite, code, nbits+1);
      }
   }
}

void affiche_huffman (struct huff_table *huff)
{
   if (huff) {
      uint8_t code[16];
      uint8_t nbits = 0;
      affiche_huffman_rec (huff->huff_tree, code, nbits);
   }
}

bool insertion(struct abr **abr, uint8_t depth, uint8_t sym)
{
   /* On descend jusqu'à la profondeur à atteindre */
   /* printf ("depth: %d\t", depth); */
   /* printf ("abr: %p\n", (*abr)); */
   if (depth) {

      if (!(*abr)) {
	 /* printf ("Création non-feuille: \n"); */
	 /* Création d'un noeud intermédiaire */
	 (*abr) = smalloc (sizeof(struct abr));
	 (*abr)->est_feuille = false;
	 (*abr)->gauche = NULL;
	 (*abr)->droite = NULL;
      }

      if (!(*abr)->est_feuille) {
	 /* Tentative d'insertion dans le sous-arbre, avec priorité à gauche */
	 if (insertion (&((*abr)->gauche), depth - 1, sym)) return true;
	 else return insertion (&((*abr)->droite), depth - 1, sym);
      }

   } else {

      /* Si le noeud est inexistant, on créée la feuille ici, sinon on tente un autre sous-arbre*/
      if (!(*abr)) {
	 /* printf ("Création feuille: 0x%x \t %p\n", sym, abr); */
	 (*abr) = smalloc (sizeof(struct abr));
	 (*abr)->est_feuille = true;
	 (*abr)->sym = sym;
	 (*abr)->gauche = NULL;
	 (*abr)->droite = NULL;
	 return true;
      }

   }
   return false;
}

struct huff_table *create_huffman_table(const uint8_t codeLengths[16],
					const uint8_t *symboles)
{
   struct huff_table *ht = smalloc (sizeof(struct huff_table));
   // Construction de l'arbre
   ht->huff_tree = NULL;
   uint8_t curSym = 0;		/* Indice du symbole courant */
   /* Pour chaque longueur de code, insertion des symboles correspondant dans l'arbre */
   for (uint32_t i = 1; i <= 16; i++) {
      for (uint32_t j = 0; j < codeLengths[i-1]; j++) {
	 /* printf ("Insertion: depth = %d | symbole = 0x%x\n", i, symboles[curSym]); */
	 insertion (&ht->huff_tree, i, symboles[curSym++]);
      }
   }

   return ht;
}

void libere_huffman (struct abr *huff)
{
   if (huff) {
      /* libere sous-arbre gauche */
      libere_huffman (huff->gauche);

      /* Libere sous-arbre droit */
      libere_huffman (huff->droite);

      free (huff);
      huff = NULL;
   }
}

void free_huffman_table (struct huff_table *table)
{
   if (table) {
      libere_huffman (table->huff_tree);
      free (table);
      table = NULL;
   }
}

/* /\*Recherche un symbole dans un tableau tab[256] *\/ */
/* /\*renvoie -1 si l'element n'y est pas, i si tab[i].abr->abr->symbole =symb  *\/ */
/* int32_t recherche_tab(struct elt *tab[256], uint8_t symb ) */
/* { */
/*    for (uint16_t i = 0; i  < 256; i++) { */
/*       if (tab[i] != NULL) */
/* 	 if (tab[i]->abr->symbole==symb) */
/* 	    return i; */
/*    } */
/*    /\* Pas trouvé *\/ */
/*       return -1 ; */
/* } */

/* /\*Parcours d'un bloc et stockage des symboles et de leurs occurrences dans un tableau AC et un tableau DC *\/ */
/* void init_freq(int32_t bloc[64], */
/* 	       struct elt *freq_DC[256], uint8_t *ind_DC, */
/* 	       struct elt *freq_AC[256], uint8_t *ind_AC, */
/* 	       int32_t  *pred_DC) */
/* { */

/* /\* Maj de freq_DC *\/ */

/*    int32_t dc= bloc[0] - *pred_DC ; */
/*    uint8_t mag = magnitude(dc, false); */
/*    int32_t x = recherche_tab( freq_DC, mag); */

/*    if (x == -1 ){ */
/*       freq_DC[*ind_DC] = malloc (sizeof(struct elt)); */
/*       freq_DC[*ind_DC]->abr = malloc (sizeof(struct abr)); */
/*       freq_DC[*ind_DC]->abr->symbole = mag ; */
/*       freq_DC[*ind_DC]->occ =1 ; */
/*       (*ind_DC) ++; */
/*    } else { */
/*       freq_DC[x]->occ++ ; */
/*    } */

/*    *pred_DC = dc; */

/* /\* Maj de freq_AC *\/ */

/*    uint8_t i = 1 ; */
/*    uint8_t nb_zeros=0; */
/*    uint8_t val ; */

/*    while (i < 64) { */
/*       // si on a un zéro on passe au coeff suivant en incrémentant le compteur */
/*       if (bloc[i] == 0 && i < 63){ */
/* 	 nb_zeros++; */
/* 	 i++; */
/*       } else { */
/* 	 if (bloc[i] == 0 || nb_zeros > 16) { */
/* 	    val = EOB; */

/* 	    /\*Maj de freq_AC*\/ */
/* 	    x = recherche_tab(freq_AC, EOB); */
/* 	    if (x == -1) { */
/* 	       freq_AC[*ind_AC] = malloc (sizeof(struct elt)); */
/* 	       freq_AC[*ind_AC]->abr = malloc (sizeof(struct abr)); */
/* 	       freq_AC[*ind_AC]->abr->symbole = EOB ; */
/* 	       freq_AC[*ind_AC]->occ= 1 ; */
/* 	       (*ind_AC)++; */
/* 	    } else { */
/* 	       freq_AC[x]->occ++; */
/* 	    } */

/* 	    return; */
/* 	 } */

/* 	 /\* if (nb_zeros > 16) { *\/ */
/* 	 /\*    fprintf(stderr, "erreur: format incorrect\n"); *\/ */
/* 	 /\*    exit (EXIT_FAILURE); *\/ */
/* 	 /\* } else  *\/ */
/* 	 if (nb_zeros == 16) { */
/* 	    /\*Maj de freq_AC*\/ */
/* 	    x = recherche_tab(freq_AC, ZRL); */
/* 	    if (x == -1) { */
/* 	       freq_AC[*ind_AC] = malloc (sizeof(struct elt)); */
/* 	       freq_AC[*ind_AC]->abr = malloc (sizeof(struct abr)); */
/* 	       freq_AC[*ind_AC]->abr->symbole = ZRL ; */
/* 	       freq_AC[*ind_AC]->occ= 1 ; */
/* 	       (*ind_AC)++; */
/* 	    } else { */
/* 	       freq_AC[x]->occ++; */
/* 	    } */
/* 	    nb_zeros = 0; */
/* 	 } */

/* 	 /\*calcul du symbole (nb zeros + mag) *\/ */
/* 	 mag = magnitude(bloc[i]); */
/* 	 val = (nb_zeros << 4) | (0x0f & mag); */

/* 	 /\*maj indices*\/ */
/* 	 i++; */
/* 	 nb_zeros=0; */

/* 	 /\*Maj de freq_AC*\/ */
/* 	 x = recherche_tab(freq_AC, val); */
/* 	 if (x == -1){ */
/* 	    freq_AC[*ind_AC] = malloc (sizeof(struct elt)); */
/* 	    freq_AC[*ind_AC]->abr = malloc (sizeof(struct abr)); */
/* 	    freq_AC[*ind_AC]->abr->symbole=val ; */
/* 	    freq_AC[*ind_AC]->occ= 1 ; */
/* 	    (*ind_AC)++; */
/* 	 } else { */
/* 	    freq_AC[x]->occ++; */
/* 	 } */
/*       } */

/*    } */
/* } */


/* ///////////////////////////////////////////////////////////////// */
/* ////////    FONCTIONS SUR LES TAS ///////////// */

/* /\*Echange deux éléments d'un tas*\/ */
/* void swap_heap(struct elt *a, struct elt *b){ */
/*    struct elt *temp = malloc (sizeof(struct elt)); */
/*    temp->abr=malloc(sizeof(struct abr)); */
/*    temp->occ= a->occ; */
/*    temp->abr->symbole=a->abr->symbole ; */

/*    a->occ=b->occ ; */
/*    a->abr->symbole=b->abr->symbole ; */
/*    b->occ=temp->occ; */
/*    b->abr->symbole=temp->abr->symbole ; */

/*    free(temp->abr); */
/*    free(temp); */
/* } */

/* /\*Insertion d'un élément dans le tas *\/ */

/* void insert_heap(struct elt *x, struct elt *heap, */
/* 		 uint8_t ind, uint8_t taille_heap){ */

/*    uint8_t i =ind ; */
/*    if (ind >taille_heap){ */
/*       printf("Tas plein !\n"); */
/*    } */

/*    /\*insertion à la fin du tas*\/ */
/*    heap[ind].abr=malloc(sizeof(struct abr)); */
/*    heap[ind].abr->est_feuille = true ; */
/*    heap[ind].abr->symbole = x->abr ->symbole ; */
/*    heap[ind].occ= x->occ ; */
/*    ind++;// ind est l'indice de la 1e case vide du tas */

/*    /\*tant que le champ occ du père est supérieur à celui de x on les échange *\/ */
/*    while (i>0){ */
/*       if (heap[i].occ < heap[i/2].occ){ */
/* 	 swap_heap(&heap[i], &heap[i/2]); */
/* 	 i=i/2 ; */
/*       } else { */
/* 	 i=0; */
/*       } */
/*    } */
/* } */


/* /\*Recupère l'élément de plus faible occurrence*\/ */
/* struct elt best_elt(struct elt *heap, uint8_t ind){ */

/*    if (ind==0){ */
/*       printf("tas vide \n"); */
/*    }; */

/*    return heap[0]; */


/* } */

/* /\*Supprime l'élément de plus faible occurrence *\/ */
/* void delete_elt(struct elt *heap, uint8_t *ind){ */

/*    if (*ind==0){ */
/*       printf("Tas vide :impossible de supprimer un élément\n"); */
/*    } */

/*    /\* Derniere feuille passe en racine *\/ */
/*    swap_heap(&heap[0], &heap[*ind-1]) ; */
/*    heap[*ind-1].occ=0 ; */
/*    (*ind)--; */

/*    /\*Tant qu'on n'est pas sur une feuille *\/ */
/*    /\* si un des fils a une occurrence plus faible que le pere, on les echange *\/ */

/*    uint8_t i=0 ; */
/*    uint8_t min ; */
/*    /\* Calcul du fils avec la plus faible occurrence *\/ */
/*    while (!heap[i].abr->est_feuille) { */
/*       /\* printf("Tas pas plein %d %d !\n", *ind, i); *\/ */
/*       if (heap[2*i].occ > heap[2*i+1].occ){ */
/* 	 min=2*i+1; */
/*       } else { */
/* 	 min=2*i ; */
/*       } */

/*       if (heap[i].occ>heap[min].occ){ */
/* 	 swap_heap(&heap[i], &heap[min]); */
/* 	 i=min; */
/*       } */
/*    } */
/* } */

/* /\*Transformation du tableau en file de priorité *\/ */
/* struct elt *tab_to_heap(struct elt *tab[256], uint8_t *nb_elt ){ */

/*    /\* Création d'un tas de bonne taille *\/ */
/*    int32_t i =0 ; */
/*    bool fin_tab=false ; */

/*    while ((! fin_tab)&& (i<256)) { */

/*       if (tab[i]==NULL){ */
/* 	 fin_tab=true; */

/*       } else { */
/* 	 i++; */
/*       } */

/*       *nb_elt=i; */
/*    } */



/*    /\*Allocation du tas *\/ */
/*    struct elt *heap = smalloc(*nb_elt*sizeof(struct elt)); */


/*    /\* Remplissage du tas *\/ */
/*    for (uint8_t j=0; j<*nb_elt; j++){ */
/*       insert_heap(tab[j], heap, j, *nb_elt) ; */

/*    } */

/*    return heap ; */
/* } */



/* /\*Libération d'un tas*\/ */
/* void free_heap( struct elt *heap){ */
/*    free(heap); */
/* } */


/* //////////////////////////////////////////////////////////////////// */
/* ///////////   FONCTIONS SUR LES ARBRES DE HUFFMAN ///////// */

/* struct abr *create_huffman_table(struct elt *tab[256], uint8_t *nb_elt) */
/* { */
/*    struct abr *gauche ; */
/*    struct abr *droit ; */
/*    uint8_t sum_occ ; */
/*    struct elt *pere ; */

/*    /\*Transformation du tableau en tas *\/ */
/*    struct elt *heap = tab_to_heap(tab,  nb_elt); */

/*    /\*Tant que il reste des elt a traiter*\/ */
/*    while (*nb_elt > 1){ */

/*       sum_occ = 0 ; */

/*       /\*Fusion des deux meilleurs noeuds en un arbre*\/ */

/*       gauche = best_elt(heap, *nb_elt).abr; */
/*       sum_occ = best_elt(heap, *nb_elt).occ ; */
/*       delete_elt(heap, nb_elt); */

/*       droit = best_elt(heap, *nb_elt).abr; */
/*       sum_occ += best_elt(heap, *nb_elt).occ; */
/*       delete_elt(heap, nb_elt); */

/*       pere = smalloc(sizeof(struct elt)); */
/*       pere->abr = smalloc(sizeof(struct abr)); */
/*       pere->abr->gauche = gauche ; */
/*       pere->abr->droit = droit ; */
/*       pere->occ = sum_occ ; */
/*       pere->abr->est_feuille = false ; */

/*       insert_heap(pere, heap, *nb_elt, *nb_elt); */
/*       (*nb_elt)++; */
/*    } */

/*    struct abr* ret = heap[0].abr; */

/*    /\*Désallocation*\/ */
/*    free_heap(heap); */

/*    return ret; */
/* } */


/* bool recherche_symbole(struct abr *abr, uint8_t symbole) */
/* { */
/*    if (abr->sym == symbole){ */
/*       return true; */
/*    } else { */
/*       if(abr->est_feuille){ */
/* 	 return false ; */
/*       } */

/*       if(recherche_symbole(abr->gauche, symbole)){ */
/* 	 return true ; */
/*       } else if (recherche_symbole(abr->droite, symbole)){ */
/* 	 return true ; */
/*       } else { */
/* 	 return false ; */
/*       } */
/*    } */

/* } */

bool huffman_value_rec(struct abr *abr,
		       uint8_t symbole,
		       uint8_t *code,
		       uint8_t *nb_bits)
{
   if (!abr)
      return false;
   else if (abr->est_feuille) {
      return (abr->sym == symbole);
   } else {
      if (huffman_value_rec(abr->gauche, symbole, code, nb_bits)) {
	 *code = (*code << 1 | 0 );
	 (*nb_bits)++;
	 return true ;
      } else if (huffman_value_rec(abr->droite, symbole, code, nb_bits)) {
	 *code = ((*code) << 1 | 1 );
	 (*nb_bits)++;
	 return true ;
      } else {
	 return false ;
      }
   }
}

void huffman_value(struct huff_table *huff, uint8_t symbole, uint8_t *code, uint8_t *nb_bits)
{
   *nb_bits = 0;

   if (!huffman_value_rec (huff->huff_tree, symbole, code, nb_bits)) {
      fprintf(stderr, "Recherche infructueuse du symbole %#x dans l'arbre \n", symbole);
      exit (EXIT_FAILURE);
   /* } else { */
   /*    printf ("code: %#x, nbb: %d\n", *code, *nb_bits); */
   }
}



/* void store_huffman_table(struct bitstream *stream, struct huff_table *ht) */
/* { */

/* } */

/* void free_huffman_table(struct huff_table *table) */
/* { */

/* } */
