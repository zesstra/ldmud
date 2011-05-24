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

static char *mem_image;
static size_t mem_image_size;


void
mem_image_free()
{
    if (mem_image)
    {
        munmap(mem_image, mem_image_size);
        mem_image=NULL;
        mem_image_size=0;
    }
}

void
mem_image_new(size_t size)
{
    char namebuf[256];
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
    // set the file size
    if (lseek(fd, size-1, SEEK_SET) == -1)
    {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        return;
    }
    // write a \0 at the end of the file.
    if (write(fd, "", 1) == -1)
    {
        close(fd);
        perror("Error writing to end of file.");
        return;
    }
    //mmap
    mem_image = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem_image == MAP_FAILED)
    {
        mem_image=NULL;
        close(fd);
        perror("Error writing to end of file.");
        return;
    }
    close(fd);  // don't need the file descriptor anymore.
    mem_image_size=size;
    ++serialno;

    // init image with 0xff (white)
    memset(mem_image,0xff,mem_image_size);
}

static INLINE void
mem_image_mark_alloced(intptr_t start, size_t size)
/* marks the words from start to end (including) as allocated
 * both are given in words.
 */
{
    if (mem_image && start+size <= mem_image_size)
        memset(mem_image+start, 0, size);
}
#endif // MEM_FRAGMENTATION_IMAGE
