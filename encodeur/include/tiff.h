#ifndef __TIFF_H__
#define __TIFF_H__

struct tiff_file_desc;

extern struct tiff_file_desc *create_tfd (const char *file_name);

extern void free_tfd (struct tiff_file_desc *tfd);

#endif
