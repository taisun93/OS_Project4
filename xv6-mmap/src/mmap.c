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
    allocuvm(p->pgdir, PGROUNDUP(oldsz), PGROUNDUP(newsz));

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
    // add for subsequent regions

    p->nregions++;

    return (void *)newsz; // fix this when I start freeing regions
}

int munmap(void *addr, int length)
{
    struct proc *p = myproc();
    // if ((int)addr % 4096 != 0)
    // {
    //     panic("too big");
    // }
    if (p->nregions == 0)
    {
        return -1;
    }

    mmapped_region *active = p->first_region;
    // mmapped_region *previous = 0;
    int counter = 0;

    while (counter < p->nregions)
    {
        // cprintf((char*) (p->nregions));
        if ((int)(active->start_addr) == (int)addr && (int)(active->length) == (int)length)
        {
            //xv6_2
            //git pull && make clean && make qemu-nox

            return((int)(length));

            // if (previous == 0)
            // {
            //     p->first_region = active->next;
            // }
            // else
            // {
            //     previous->next = active->next;
            // }
            p->nregions--;

            return 1;
        }

        if (active->next == 0)
        {
            // fails here
            return -1;
        }
        // previous = active;
        active = active->next;
        counter++;
    }

    return -1;
}