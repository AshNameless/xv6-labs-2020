// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};


// each cput has its own free list and lock
struct {
  struct spinlock lock[NCPU];
  struct run * freelist[NCPU];
  uint64 cpumsize;
} kmem;



void
kinit()
{
  // each cpu gets only its own memory at the begining
  
  // per cpu mem size
  kmem.cpumsize = ((uint64)PHYSTOP - (uint64)end)/NCPU;

  for(int i = 0; i < NCPU; i++)
    initlock(&kmem.lock[i], "kmem");

  freerange(end, (void*)PHYSTOP);
}






void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int id;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  // determine which cpu this page belongs to 
  for(id = 0; id < NCPU; id++){
    if(
      (uint64)pa >= PGROUNDUP((uint64)end+kmem.cpumsize*id)
      && (uint64)pa < PGROUNDUP((uint64)end+kmem.cpumsize*(id+1)) 
      ) 
      break;
  }


  acquire(&kmem.lock[id]);
  r->next = kmem.freelist[id];
  kmem.freelist[id] = r;
  release(&kmem.lock[id]);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;


  push_off();
  int id = cpuid();

  acquire(&kmem.lock[id]);
  r = kmem.freelist[id];

  if(r){
    // this cpu's own free list has free pages
    kmem.freelist[id] = r->next;
    release(&kmem.lock[id]);
  } else {
    release(&kmem.lock[id]);
    // find a page in other free lists
    for(int i = 0; i < NCPU; i++){
      if(i == id)
        continue;

      acquire(&kmem.lock[i]);
      r = kmem.freelist[i];
      if(r){
        kmem.freelist[i] = r->next;
        release(&kmem.lock[i]);
        break;
      }
      release(&kmem.lock[i]);

    }
  }

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  pop_off();
  return (void*)r;
}
