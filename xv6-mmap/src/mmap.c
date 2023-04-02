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
      if (addr < (void*)0 || addr == (void*)KERNBASE || addr > (void*)KERNBASE || length < 1)
  {
    return (void*)-1;
  }

  // Get pointer to current process
  struct proc *p = myproc();
  uint oldsz = p->sz;
  uint newsz = p->sz + length;

  // Expand process size
  p->sz = newsz;

  // Allocate a new region for our mmap (w/ kmalloc)
  mmapped_region* r = (mmapped_region*)kmalloc(sizeof(mmapped_region));
  if (r == NULL)
  {
    return (void*)-1;
  }

  // Assign list-data and meta-data to the new region
  r->start_addr = (addr = (void*)(PGROUNDDOWN(oldsz) + MMAPBASE));
  r->length = length;
  r->region_type = flags;
  r->offset = offset;
  r->prot = prot;
  r->next = 0;

  // Check the flags and file descriptor argument (flags, fd)
  if (flags == MAP_ANONYMOUS)
  {
    if (fd != -1) //fd must be -1 in this case (mmap man page sugestion for mobility)
    {
      kmfree(r);
      return (void*)-1;
    }
    // do not set r->fd. Not needed for Anonymous mmap
  }
  else if (flags == MAP_FILE)
  {
    if (fd > -1)
    {
      if((fd=fdalloc(p->ofile[fd])) < 0)
        return (void*)-1;
      filedup(p->ofile[fd]);
      r->fd = fd;
    }
    else
    {
      kmfree(r);
      return (void*)-1;
    }
  }

  // Handle first call to mmap
  if (p->nregions == 0)
  {
    p->region_head = r;
  }
  else // Add region to an already existing mapped_regions list
  {
    mmapped_region* cursor = p->region_head;
    while (cursor->next != 0)
    {
      if (addr == cursor->start_addr)
      {
        addr += PGROUNDDOWN(PGSIZE+cursor->length);
        cursor = p->region_head; // start over, we may overlap past regions now...
      }
      else if (addr == (void*)KERNBASE || addr > (void*)KERNBASE) //we've run out of memory!
      {
        kmfree(r);
        return (void*)-1;
      }
      cursor = cursor->next;
    }
    // Catch the final node that isn't checked in the loop
    if (addr == cursor->start_addr)
    {
      addr += PGROUNDDOWN(PGSIZE+cursor->length);
    }

    // Add new region to the end of our mmapped_regions list
    cursor->next = r;
  }

  // Increment region count and retrun the new region's starting address
  p->nregions++;
  r->start_addr = addr;

  return r->start_addr;
}

int munmap(void *addr, uint length)
{
    return 0;
}