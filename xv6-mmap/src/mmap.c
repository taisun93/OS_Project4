#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#define MMAPBASE 0x10000000

void *mmap(void *addr, int length, int prot, int flags, int fd, int offset)
{
    
    if (addr < (void *)0 || addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return (void *)-1;
    }

    // Get pointer to current process
    struct proc *p = myproc();
    // uint oldsz = p->sz;
    // Expand process size
    
    p->sz = p->sz + length;
    

    // new item in linked list
    // mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));

    // Fill the item
    //problem child
    

    
    // r->start_addr = addr;
    // r->length = length;
    // r->region_type = flags;
    // r->offset = offset;
    // r->prot = prot;
    // r->next = 0;

    // if (p->nregions == 0)
    // {
    //     p->region_head = r;
    // }

    // p->nregions++;

    return (void *)PGROUNDDOWN(p->sz);
}

int munmap(void *addr, int length)
{
    
    return -1;
}