#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#define MMAPBASE (KERNBASE / 2)
void *mmap(void *addr, int length, int prot, int flags, int fd, int offset)
{

    if (addr < (void *)0 || addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return (void *)-1;
    }

    // Get pointer to current process
    struct proc *p = myproc();
    uint oldsz = MMAPBASE;
    uint newsz = oldsz + length;
    // Expand process size
    allocuvm(p->pgdir, PGROUNDUP(oldsz), PGROUNDUP(newsz));

    // p->sz = p->sz + length;

    // new item in linked list
    mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));

    // fill up
    r->start_addr = (addr = (void*)(PGROUNDDOWN(oldsz) + MMAPBASE));
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

    return addr; // fix this when I start freeing regions
}

int munmap(void *addr, int length)
{
    // return length;
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
        return (int)(active->start_addr);
        if ((active->start_addr) == addr)
        {

            //xv6_2
            //git pull && make clean && make qemu-nox

            

            // if (previous == 0)
            // {
            //     p->first_region = active->next;
            // }
            // else
            // {
            //     previous->next = active->next;
            // }
            p->nregions--;

            return 69;
        }
        if((active->length) == length){
            return 7;
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