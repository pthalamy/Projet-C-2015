
#include "jpeg.h"

#include <stdint.h>
#include <stdio.h>


struct jpeg_file_desc {

};

struct jpeg_file_desc *init_jpeg_file (const char *file_name,
				       uint32_t width,
				       uint32_t height,
				       uint8_t sfh,
				       uint8_t sfv)
{
   return NULL;
}

void close_jpeg_file(struct jpeg_file_desc *jfd)
{

}

/* extern int32_t write_tiff_file (struct tiff_file_desc *tfd, */
/*                                 uint32_t *mcu_rgb, */
/*                                 uint8_t nb_blocks_h, */
/*                                 uint8_t nb_blocks_v)
{

} */
