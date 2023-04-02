#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

#define NULL (mmapped_region *)0
#define MAP_ANONYMOUS 0
#define MAP_FILE 1
#define PROT_WRITE 1
#define MMAPBASE 0x40000000


int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

void *mmap(void *addr, uint length, int prot, int flags, int fd, int offset)
{
    // Check argument inputs (only addr and length for now...)
    if (addr < (void *)0 || addr == (void *)KERNBASE || addr > (void *)KERNBASE || length < 1)
    {
        return (void *)-1;
    }

    // Get pointer to current process
    struct proc *p = myproc();
    uint oldsz = p->sz;
    uint newsz = p->sz + length;

    // Expand process size
    p->sz = newsz;

    // Allocate a new region for our mmap (w/ kmalloc)
    mmapped_region *r = (mmapped_region *)kmalloc(sizeof(mmapped_region));
    if (r == NULL)
    {
        return (void *)-1;
    }

    // Assign list-data and meta-data to the new region
    r->start_addr = (addr = (void *)(PGROUNDDOWN(oldsz) + MMAPBASE));
    r->length = length;
    r->region_type = flags;
    r->offset = offset;
    r->prot = prot;
    r->next = 0;

    // Check the flags and file descriptor argument (flags, fd)
    if (flags == MAP_ANONYMOUS)
    {
        if (fd != -1) // fd must be -1 in this case (mmap man page sugestion for mobility)
        {
            kmfree(r);
            return (void *)-1;
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
        return (void *)-1;
        }
    }

    // Handle first call to mmap
    if (p->nregions == 0)
    {
        p->region_head = r;
    }
    else // Add region to an already existing mapped_regions list
    {
        mmapped_region *cursor = p->region_head;
        while (cursor->next != 0)
        {
            if (addr == cursor->start_addr)
            {
                addr += PGROUNDDOWN(PGSIZE + cursor->length);
                cursor = p->region_head; // start over, we may overlap past regions now...
            }
            else if (addr == (void *)KERNBASE || addr > (void *)KERNBASE) // we've run out of memory!
            {
                kmfree(r);
                return (void *)-1;
            }
            cursor = cursor->next;
        }
        // Catch the final node that isn't checked in the loop
        if (addr == cursor->start_addr)
        {
            addr += PGROUNDDOWN(PGSIZE + cursor->length);
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
  // Sanity check on addr and length
  if (addr == (void*)KERNBASE || addr > (void*)KERNBASE || length < 1)
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
    if(p->region_head->region_type == MAP_FILE)
    {
      fileclose(p->ofile[p->region_head->fd]);
      p->ofile[p->region_head->fd] = 0;
    }

    if(p->region_head->next != 0)
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

  while(next != 0)
  {
    if (next->start_addr == addr && next->length == length)
    {
      /*deallocate the memory from the current process*/
      p->sz = deallocuvm(p->pgdir, p->sz, p->sz - length);
      switchuvm(p);
      p->nregions--;  
      
      // close the file we were mapping to
      if(next->region_type == MAP_FILE)
      {
        fileclose(p->ofile[next->fd]);
        p->ofile[next->fd] = 0;
      }

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