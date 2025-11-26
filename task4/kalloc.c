// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

//IMPLEMENTACAO TASK 4 - Referencias nas contagens
struct {
  struct spinlock lock;
  int use_lock;
  int count[PHYSTOP >> PGSHIFT]; // Array para contar refs de cada pagina fisica
} ref;

// Funcoes auxiliares para gerenciar referencias
void
inc_ref(uint pa)
{
  if(pa >= PHYSTOP)
    return;
  
  if(ref.use_lock)
    acquire(&ref.lock);
  
  ref.count[pa >> PGSHIFT]++;
  
  if(ref.use_lock)
    release(&ref.lock);
}

void
dec_ref(uint pa)
{
  if(pa >= PHYSTOP)
    return;
  
  if(ref.use_lock)
    acquire(&ref.lock);
  
  ref.count[pa >> PGSHIFT]--;
  
  if(ref.use_lock)
    release(&ref.lock);
}

int
get_ref(uint pa)
{
  int c;
  
  if(ref.use_lock)
    acquire(&ref.lock);
  
  c = ref.count[pa >> PGSHIFT];
  
  if(ref.use_lock)
    release(&ref.lock);
  
  return c;
}
// -----------------------------------------------------

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  
  // Inicializa o lock das referencias
  initlock(&ref.lock, "ref");
  ref.use_lock = 0;
  
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
  
  // Ativa o lock das referencias
  ref.use_lock = 1;
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
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // --- LÓGICA TASK 4 ---
  // Decrementa a referencia ao liberar
  dec_ref(V2P(v));

  // Se o contador ainda for maior que 0, significa que
  // outro processo (pai ou filho) ainda esta usando essa pagina.
  // Entao NAO liberamos a memoria fisica real ainda.
  if(get_ref(V2P(v)) > 0)
    return;
  // ---------------------

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

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    
    // --- LÓGICA TASK 4 ---
    // Quando alocamos uma pagina nova, ela passa a ter 1 dono.
    ref.count[V2P((char*)r) >> PGSHIFT] = 1;
    // ---------------------
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}