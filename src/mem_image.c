/*
 *  mem_image.c
 *  LDMud
 *
 *
 * Black: free words in large blocks
 * Red:   allocated words in large blocks
 * Green: free words in small blocks
 * Blue:  allocated words in small blocks
 */

#ifdef MEM_FRAGMENTATION_IMAGE

#include <fcntl.h>
#include <sys/mman.h>

#include "main.h"

#define IMAGE_WIDTH 1024
#define BYTES_PER_PIXEL 3
static const unsigned char LARGE_FREE[] = { 0, 0, 0 };
static const unsigned char LARGE_ALLOCED[] = { 0xff, 0, 0 };
static const unsigned char SMALL_FREE[] = { 211, 211, 211 };
static const unsigned char SMALL_ALLOCED[] = { 0, 0, 0xff };


static char *mem_image;
static size_t mem_image_hdrlength;
static size_t mem_image_size;   // in words! (raw length: * BYTES_PER_PIXEL)


void
mem_image_free()
{
    if (mem_image)
    {
        mem_image-=mem_image_hdrlength;
        munmap(mem_image, mem_image_hdrlength+(mem_image_size*BYTES_PER_PIXEL));
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

    snprintf(namebuf,sizeof(namebuf),"MEM_ALLOCATION_IMAGE-%ld.ppm",
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

    // create PPM header
    mem_image_hdrlength=snprintf(pgmheader,sizeof(pgmheader),
             "P6\n%ld %ld\n255\n",
             (long)IMAGE_WIDTH, mem_image_size/IMAGE_WIDTH);

    // set the file size
    if (lseek(fd, mem_image_hdrlength+(mem_image_size*BYTES_PER_PIXEL), SEEK_SET) == -1)
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
    mem_image = mmap(0, mem_image_hdrlength+(mem_image_size*BYTES_PER_PIXEL),
                     PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
    // init image with 0xff (white). Not really necessary, but it helps to
    // find bugs.
    memset(mem_image, 0xff,mem_image_size*BYTES_PER_PIXEL);

}

static INLINE void
mem_image_mark_alloced(intptr_t offset, size_t size, const unsigned char * const pattern)
/* marks the words from start to end (including) as allocated
 * both are given in words (and thus have to be multiplied with BYTES_PER_PIXEL).
 * Recursively copy the memory, using the area already filled as a template
 * per iteration (O(log(N) calls to memcpy):
 */
{
    size_t blocksize = BYTES_PER_PIXEL; // initial size of block to write

    if (!mem_image || offset+size > mem_image_size)
    {
        perror("Bad image size!\n");
        return;
    }

    char * start = mem_image+(offset*BYTES_PER_PIXEL);   // start of block data
    char * end = start + (size*BYTES_PER_PIXEL);         // end of block data

    memcpy(start, pattern, BYTES_PER_PIXEL);
    char * current = start + BYTES_PER_PIXEL;

    while(current + blocksize < end) {
        memcpy(current, start, blocksize);
        current += blocksize;
        blocksize *= 2;
    }
    // fill the rest
    memcpy(current, start, end-current);
}
#endif // MEM_FRAGMENTATION_IMAGE
