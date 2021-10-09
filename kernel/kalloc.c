// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// record how many ptes point to a specific physical frame
struct{
  struct spinlock lock;
  int count[PHYSTOP/PGSIZE];
} krefcount;



void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&krefcount.lock, "krefcount");
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
  int hold = 0;
  if(holding(&krefcount.lock))
    hold = 1;
  
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  
  

  if(!hold)
    acquire(&krefcount.lock);
  // if there are more than one ptes reference this page,
  // do not recycle it
  if(krefcount.count[(uint64)pa/PGSIZE] > 1){
    krefcount.count[(uint64)pa/PGSIZE]--;
    if(!hold)
      release(&krefcount.lock);
    return;
  
  // the only process wanna free this page
  }else if(krefcount.count[(uint64)pa/PGSIZE] == 1)
    krefcount.count[(uint64)pa/PGSIZE]--;



  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;

  release(&kmem.lock);
  if(!hold)
    release(&krefcount.lock);


}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  int hold = 0;
  if(holding(&krefcount.lock))
    hold = 1;

  if(!hold)
    acquire(&krefcount.lock);


  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;

  release(&kmem.lock);

  // find a valid frame
  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    // the count will be touched only if allocation succeed.  
    krefcount.count[(uint64)r/PGSIZE]++;
  }

  if(!hold)
    release(&krefcount.lock);

  return (void*)r;
}
