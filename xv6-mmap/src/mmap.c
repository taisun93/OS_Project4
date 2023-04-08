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
    // remember to test fork.
    //  Get pointer to current process
    struct proc *p = myproc();
    // uint oldsz = MMAPBASE;
    // uint newsz = oldsz + length;
    
    
    // check for fail

    // uint total_length = 0;

    // new item in linked list
    mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));

    // fill up
    r->start_addr = (void *)PGROUNDUP(MMAPBASE);
    r->length = length;
    r->region_type = flags;
    r->offset = offset;
    r->prot = prot;
    r->next = 0;

    int total_length = length;
    if (p->nregions == 0)
    {
        p->first_region = r;
    }
    else
    {
        // can probably just use this instead of the if statement but be safe for now
        int i;
        mmapped_region *active = p->first_region;

        for (i = 0; i < p->nregions; i++)
        {
            active = active->next;
            total_length += PGROUNDUP(active->length);
            r->start_addr += PGROUNDUP(active->length);
        }
        active->next = r;
    }
    uint start = (uint)r->start_addr;
    
    allocuvm(p->pgdir, start, start+total_length);
    p->nregions++;

    return r->start_addr; // fix this when I start freeing regions
}

int munmap(void *addr, int length)
{
    struct proc *p = myproc();
    if (p->nregions == 0)
    {
        return -1;
    }

    mmapped_region *active = p->first_region;
    mmapped_region *previous = 0;
    int counter = 0;

    while (counter < p->nregions)
    {

        // return (int)(active->start_addr);
        if ((active->start_addr) == addr)
        {

            // xv6_2
            // make clean && make qemu-nox

            if ((active->length) == length)
            {

                // deallocuvm(p->pgdir, PGROUNDUP((int)addr), PGROUNDUP((int)addr - length));
                deallocuvm(p->pgdir, PGROUNDUP((int)addr + length), PGROUNDUP((int)addr));
                // first node
                if (previous == 0)
                {
                    p->first_region = active->next;
                }
                // not first node
                else
                {
                    previous->next = active->next;
                }
                kmfree(active);
                p->nregions--;

                return 42;
            }
        }

        if (active->next == 0)
        {
            return -1;
        }
        // previous = active;
        active = active->next;
        counter++;
    }

    return -1;
}