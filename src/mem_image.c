/*
 *  mem_image.c
 *  LDMud
 *
 *
 */

#ifdef MEM_FRAGMENTATION_IMAGE

#include <fcntl.h>
#include <sys/mman.h>

#include "main.h"

#define IMAGE_WIDTH 1024

static char *mem_image;
static size_t mem_image_hdrlength;
static size_t mem_image_size;


void
mem_image_free()
{
    if (mem_image)
    {
        mem_image-=mem_image_hdrlength;
        munmap(mem_image, mem_image_size);
        mem_image=NULL;
    }
}

void
mem_image_new(size_t size)
{
    char namebuf[256];
    char pgmheader[256];
    int fd;

    // just to be on the safe side... I just care of leaked memory, not
    // about incomplete images in the unlikely case of mis-use.
    mem_image_free();

    snprintf(namebuf,sizeof(namebuf),"MEM_ALLOCATION_IMAGE-%ld.pgm",
             time(NULL)-boot_time);
    fd = ixopen3(namebuf,O_RDWR|O_CREAT|O_EXCL|O_TRUNC|O_EXLOCK,0640);
    if (fd <0)
    {
        perror("Error opening file ... ");
        return; // no image...
    }
    // calculate the size of the image to be the next-higher multiple of
    // IMAGE_WIDTH.
    mem_image_size = (size_t)ceil(size/(double)IMAGE_WIDTH) * IMAGE_WIDTH;

    // create PGM header
    mem_image_hdrlength=snprintf(pgmheader,sizeof(pgmheader),
             "P5\n%ld %ld\n255\n",
             (long)IMAGE_WIDTH, mem_image_size/IMAGE_WIDTH);

    // set the file size
    if (lseek(fd, mem_image_hdrlength+mem_image_size, SEEK_SET) == -1)
    {
        close(fd);
        perror("Error calling lseek() to the end of image file.");
        return;
    }
    // write a \n at the end of the file.
    if (write(fd, "\n", 1) == -1)
    {
        close(fd);
        perror("Error writing linefeed to end of file.");
        return;
    }
    //mmap
    mem_image = mmap(0, mem_image_hdrlength+mem_image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem_image == MAP_FAILED)
    {
        mem_image=NULL;
        close(fd);
        perror("Error mapping the image file into memory.");
        return;
    }
    close(fd);  // don't need the file descriptor anymore.

    // write header and set the pointer to the beginning of the bitmap data
    memcpy(mem_image,pgmheader,mem_image_hdrlength);
    mem_image+=mem_image_hdrlength;
    // init image with 0xff (black)
    memset(mem_image,0x0,mem_image_size);

}

static INLINE void
mem_image_mark_alloced(intptr_t start, size_t size, unsigned char val)
/* marks the words from start to end (including) as allocated
 * both are given in words.
 */
{
    if (mem_image && start+size <= mem_image_size)
        memset(mem_image+start, val, size);
}
#endif // MEM_FRAGMENTATION_IMAGE
