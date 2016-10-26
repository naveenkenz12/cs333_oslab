// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

//lab_6_modification_start
int numfreepages;
//lab_6_modification_end

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

//lab_6_modification_start
//pg_ref_array[i] will keep refernece count of physical location i
//total physical location will range from [0,PHYSTOP/PGSIZE)
char pg_ref_array[PHYSTOP/PGSIZE];

struct {
  struct spinlock lock;
  int use_lock;
} pg_ref;

void
pgcountinit()
{ 
  initlock(&pg_ref.lock, "pg_ref");
  pg_ref.use_lock = 0;
  numfreepages = 0;
  int i = 0;
  for(;i<PHYSTOP/PGSIZE;i++)pg_ref_array[i]= 1;     
  //kernel will free at start to make it 0
}

void
decrese_ref_count(uint location)
{
  if(pg_ref.use_lock)
    acquire(&pg_ref.lock);
  pg_ref_array[location] = pg_ref_array[location] - 1;
  if(pg_ref.use_lock)
    release(&pg_ref.lock);
}

void
increase_ref_count(uint location)
{
  if(pg_ref.use_lock)
    acquire(&pg_ref.lock);
  pg_ref_array[location] = pg_ref_array[location] + 1;
  if(pg_ref.use_lock)
    release(&pg_ref.lock);
}

void
set_ref_count_to_one(uint location)
{
  if(pg_ref.use_lock)
    acquire(&pg_ref.lock);
  pg_ref_array[location] = 1;
  if(pg_ref.use_lock)
    release(&pg_ref.lock);
}

//lab_6_modification_end

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
  //lab_6_modification_start
  pg_ref.use_lock = 1;
  //lab_6_modification_end
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)

//lab_6_modification_start
/* commented the old kfree function
//lab_6_modification_end
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}
//lab_6_modification_start
*/
//lab_6_modification_end

//lab_6_modification_start
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  uint location = V2P(v)>>12;        // divided by PGSIZE(4096)

  if(kmem.use_lock)
    acquire(&kmem.lock);

  decrese_ref_count(location);

  //first decreses the reference count corresponding to physical
  //location by 1.
  //if the refernce count == 0, i.e. now no virtual address 
  //points to that physical location, free that page

  if(pg_ref_array[location]==0)
  {
    // Fill with junk to catch dangling refs.
    memset(v, 1, PGSIZE);

    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    numfreepages = numfreepages + 1;
  }

  if(kmem.use_lock)
    release(&kmem.lock);

}
//lab_6_modification_end

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.

//lab_6_modification_start
/* commented the old kalloc function
//lab_6_modification_end
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}
//lab_6_modification_start
*/
//lab_6_modification_end

//lab_6_modification_start
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = kmem.freelist;
  if(r)
  {
    kmem.freelist = r->next;
    //number of free pages decreases by 1
    numfreepages = numfreepages - 1;
    uint location = V2P((char*)r)>>12;        // divided by PGSIZE(4096)
    //whenever a new page is assigned, its reference count will be 1
    
    set_ref_count_to_one(location);
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}
//lab_6_modification_end