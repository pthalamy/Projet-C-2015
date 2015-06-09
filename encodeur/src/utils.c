
#include "utils.h"

const uint8_t z = 1;
#define cpu_is_bigendian() ( (*(char*)&z) == 0 )

void *smalloc(size_t size)
{
   void *ptr = malloc (size);

   if (!ptr) {
      fprintf (stderr, "error: OUT OF MEMORY\n");
      exit (EXIT_FAILURE);
   }

   return ptr;
}

void *scalloc(size_t num, size_t size)
{
   void *ptr = calloc (num, size);

   if (!ptr) {
      fprintf (stderr, "error: OUT OF MEMORY\n");
      exit (EXIT_FAILURE);
   }

   return ptr;
}

uint32_t le32_to_cpu(const uint32_t v)
{
   if (cpu_is_bigendian())
      return ((v & 0xff000000) >> 24) | ((v & 0x00ff0000) >> 8)
	 | ((v & 0x0000ff00) << 8) | ((v & 0x000000ff) << 24);
   else
      return v;
}

uint32_t be32_to_cpu(const uint32_t v)
{
   if (cpu_is_bigendian())
      return v;
   else
      return ((v & 0xff000000) >> 24) | ((v & 0x00ff0000) >> 8)
	 | ((v & 0x0000ff00) << 8) | ((v & 0x000000ff) << 24);
}

uint16_t be16_to_cpu(const uint16_t v)
{
   if (cpu_is_bigendian())
      return v;
   else
      return ((v & 0xff00) >> 8) | ((v & 0x00ff) << 8);
}

uint16_t le16_to_cpu(const uint16_t v)
{
   if (cpu_is_bigendian())
      return ((v & 0xff00) >> 8) | ((v & 0x00ff) << 8);
   else
      return v;
}

void print_uint8_t_block(uint8_t *bloc, uint32_t num_bloc)
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

void print_uint32_t_block(uint32_t *bloc, uint32_t num_bloc)
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

void print_int32_t_block(int32_t *bloc, uint32_t num_bloc)
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


void print_int8_t_mcu(uint8_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv)
{
   printf ("mcu numéro %d\n", num_mcu);
   for (uint32_t i = 0; i < (uint32_t)(64*sfv*sfh); i++) {
      if (!(i % (sfh * 8)))
	 printf ("\n");
      printf ("%d ", mcu[i]);
   }
   printf ("\n");
}

void print_uint32_t_mcu(uint32_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv)
{
   printf ("mcu numéro %d\n", num_mcu);
   for (uint32_t i = 0; i < (uint32_t)(64*sfv*sfh); i++) {
      if (!(i % (sfh * 8)))
	 printf ("\n");
      printf ("%d ", mcu[i]);
   }
   printf ("\n");
}

void print_int32_t_mcu(int32_t *mcu, uint32_t num_mcu, uint8_t sfh, uint8_t sfv)
{
   printf ("mcu numéro %d\n", num_mcu);
   for (uint32_t i = 0; i < (uint32_t)(64*sfv*sfh); i++) {
      if (!(i % (sfh * 8)))
	 printf ("\n");
      printf ("%d ", mcu[i]);
   }
   printf ("\n");
}
