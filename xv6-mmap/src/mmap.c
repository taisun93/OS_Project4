#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

void *mmap(void *addr, int length, int prot, int flags, int fd, int offset)
{

    if (addr < (void *)0 || addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return (void *)-1;
    }

    // Get pointer to current process
    struct proc *p = myproc();
    uint oldsz = (KERNBASE / 2);
    uint newsz = oldsz + length;
    // Expand process size
    // printf("about to alloc\n");
    allocuvm(p->pgdir, PGROUNDUP(oldsz), newsz);

    // p->sz = p->sz + length;

    // new item in linked list
    mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));

    // fill up
    r->start_addr = addr;
    r->length = length;
    r->region_type = flags;
    r->offset = offset;
    r->prot = prot;
    r->next = 0;

    if (p->nregions == 0)
    {
        p->first_region = r;
    }

    p->nregions++;

    return (void *)newsz; // fix this when I start freeing regions
}

int munmap(void *addr, int length)
{
    struct proc *p = myproc();
    if ((int)addr % 4096 != 0)
    {
        panic("too big");
    }
    if (p->nregions == 0)
    {
        return -1;
    }

    mmapped_region *active = p->first_region;

    int counter = p->nregions;

    while (counter > 0)
    {
        if(((active->start_addr) == addr) && (active->length = length)){
            return 1;
        }

        active = active->next;
        counter--;
    }

    return -1;
}