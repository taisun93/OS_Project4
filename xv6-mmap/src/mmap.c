#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
// #include "mman.h"
#include "proc.h"


// #define NULL (mmapped_region*)0

void *mmap(void *addr, uint length, int prot, int flags, int fd, int offset)
{
    return (void*)-1; 
}

int munmap(void *addr, uint length)
{
    return 0;
}