/*
 *  mem_image.c
 *  LDMud
 *
 *  Created by Dominik Schäfer on 21.05.11.
 *  Copyright 2011 Ruhr-Uni-Bochum. All rights reserved.
 *
 */

#ifdef MEM_FRAGMENTATION_IMAGE

#include <fcntl.h>
#include <sys/mman.h>
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
    static int serialno=1;  // give the image dumps a serial no.
    int fd;

    mem_image_free(); // just to be on the safe side...

    snprintf(namebuf,256,"MEM_ALLOCATION_IMAGE-%d",serialno);
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
