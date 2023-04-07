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

static void ll_delete(mmapped_region *node, mmapped_region *prev)
{
    if (node == myproc()->first_region)
    {
        if (myproc()->first_region->next != 0)
        {
            myproc()->first_region = myproc()->first_region->next;
        }
        else
        {
            myproc()->first_region = 0;
        }
    }
    else
    {
        prev->next = node->next;
    }
    kmfree(node);
}

int munmap(void *addr, int length)
{
    // Sanity check on addr and length
    if (addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return -1;
    }

    struct proc *p = myproc();

    // If nothing has been allocated, there is nothing to munmap
    if (p->nregions == 0)
    {
        return -1;
    }

    // Travese our mmap dll to see if address and length are valid
    mmapped_region *prev = p->first_region;
    mmapped_region *next = p->first_region->next;
    int size = 0;

    // Check the head
    if (p->first_region->start_addr == addr && p->first_region->length == length)
    {
        /*deallocate the memory from the current process*/
        p->sz = deallocuvm(p->pgdir, p->sz, p->sz - length);
        switchuvm(p);
        p->nregions--;


        if (p->first_region->next != 0)
        {
            /* Calls to kmfree were changing the node->next's length value
             * in the linked-list. This is a hacky fix, but I don't know
             * what is really causing that problem... */
            size = p->first_region->next->length;
            ll_delete(p->first_region, 0);
            p->first_region->length = size;
        }
        else
        {
            ll_delete(p->first_region, 0);
        }

        /*return success*/
        return 0;
    }

    while (next != 0)
    {
        if (next->start_addr == addr && next->length == length)
        {
            /*deallocate the memory from the current process*/
            p->sz = deallocuvm(p->pgdir, p->sz, p->sz - length);
            switchuvm(p);
            p->nregions--;

            // close the file we were mapping to

            /*remove the node from our ll*/
            size = next->next->length;
            ll_delete(next, prev);
            prev->next->length = size;

            /*return success*/
            return 0;
        }
        prev = next;
        next = prev->next;
    }

    // if there was no match, return -1
    return -1;
}
