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
#define MMAPBASE 0x10000000

int fdalloc(struct file *f)
{
    int fd;
    struct proc *curproc = myproc();

    for (fd = 0; fd < NOFILE; fd++)
    {
        if (curproc->ofile[fd] == 0)
        {
            curproc->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

static void ll_delete(mmapped_region *node, mmapped_region *prev)
{
    if (node == myproc()->region_head)
    {
        if (myproc()->region_head->next != 0)
        {
            myproc()->region_head = myproc()->region_head->next;
        }
        else
        {
            myproc()->region_head = 0;
        }
    }
    else
    {
        prev->next = node->next;
    }
    kmfree(node);
}

void *mmap(void *addr, int length, int prot, int flags, int fd, int offset)
{
    return addr;

    if (addr < (void *)0 || addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return (void *)-1;
    }

    // Get pointer to current process
    struct proc *p = myproc();
    uint oldsz = p->sz;
    // Expand process size
    p->sz = p->sz + length;

    // Allocate a new region for our mmap 
    mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));

    // Assign list-data and meta-data to the new region
    r->start_addr = (addr = (void *)(PGROUNDDOWN(oldsz) + MMAPBASE));
    r->length = length;
    r->region_type = flags;
    r->offset = offset;
    r->prot = prot;
    r->next = 0;

    // Handle first call to mmap
    if (p->nregions == 0)
    {
        p->region_head = r;
    }

    // Increment region count and retrun the new region's starting address
    p->nregions++;
    r->start_addr = addr;

    return addr;
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
    mmapped_region *prev = p->region_head;
    mmapped_region *next = p->region_head->next;
    int size = 0;

    // Check the head
    if (p->region_head->start_addr == addr && p->region_head->length == length)
    {
        /*deallocate the memory from the current process*/
        p->sz = deallocuvm(p->pgdir, p->sz, p->sz - length);
        switchuvm(p);
        p->nregions--;

        // close the file we were mapping to

        if (p->region_head->next != 0)
        {
            /* Calls to kmfree were changing the node->next's length value
             * in the linked-list. This is a hacky fix, but I don't know
             * what is really causing that problem... */
            size = p->region_head->next->length;
            ll_delete(p->region_head, 0);
            p->region_head->length = size;
        }
        else
        {
            ll_delete(p->region_head, 0);
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