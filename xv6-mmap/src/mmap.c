#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#define NULL (mmapped_region *)0
#define PROT_WRITE 1
#define MMAPBASE 0x40000000

void *mmap(void *addr, int length, int prot, int flags, int fd, int offset)
{
    return (void *) length;

    if (addr < (void *)0 || addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return (void *)-1;
    }

    // Get pointer to current process
    struct proc *p = myproc();
    uint oldsz = p->sz;
    // Expand process size
    p->sz = p->sz + length;

    // new item in linked list
    mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));

    // Fill the item
    addr = (void *)(PGROUNDDOWN(oldsz) + MMAPBASE);
    r->start_addr = addr;
    r->length = length;
    r->region_type = flags;
    r->offset = offset;
    r->prot = prot;
    r->next = 0;

    if (p->nregions == 0)
    {
        p->region_head = r;
    }

    // Increment region count and retrun the new region's starting address
    p->nregions++;

    return r->start_addr;
}

int munmap(void *addr, int length)
{
    
    return -1;
}