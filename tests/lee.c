#include <stdlib.h> 
#include <stdio.h>
#include <stdint.h>

/* Structure permettant de stocker les informations nécessaire à
 * l'écriture des données de l'image dans un fichier TIFF. */

struct tiff_file_desc
{
   uint32_t width;
   uint32_t height;
   uint32_t row_per_strip;
   FILE *tiff;
}; 

/* Initialisation du fichier TIFF résultat, avec les paramètres suivants:
   - width: la largeur de l'image ;
   - height: la hauteur de l'image ;
   - row_per_strip: le nombre de lignes de pixels par bande.
 */
struct tiff_file_desc *init_tiff_file (const char *file_name,
      uint32_t width,
      uint32_t height,
      uint32_t row_per_strip)
{
   struct tiff_file_desc *tfd;
   tfd=malloc(sizeof(struct tiff_file_desc));
   tfd->tiff=fopen(file_name,"w");
   tfd->width=width;
   tfd->height=height;
   tfd->row_per_strip=row_per_strip;
   uint8_t buffer[8]={0x4D, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08};
   fwrite(buffer, 1, 8, tfd->tiff);
   return tfd;
}

/* Ferme le fichier associé à la structure tiff_file_desc passée en
 * paramètre et désalloue la mémoire occupée par cette structure. */
void close_tiff_file(struct tiff_file_desc *tfd)
{
   fclose(tfd->tiff);
   free(tfd);
   tfd=NULL;
}
int main(){
   struct tiff_file_desc *tfd=init_tiff_file("gros_lee.tiff",10,10,10);
   close_tiff_file(tfd);
   return 0;
}
