#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  // --- IMPLEMENTACAO TASK 4: Tratamento de Page Fault (CoW) ---
  case T_PGFLT:
    {
      struct proc *p = myproc();
      uint va = rcr2(); // Endereco que causou o erro
      pte_t *pte;

      // Verifica se endereco esta dentro dos limites do processo
      if(va >= p->sz){
         p->killed = 1;
         break;
      }
      
      // Busca a entrada na tabela de paginas
      pte = walkpgdir(p->pgdir, (void*)va, 0);

      // Verifica se a pagina existe e se eh COW
      if(pte && (*pte & PTE_P) && (*pte & PTE_COW)){
         uint pa = PTE_ADDR(*pte);
         char *mem = kalloc(); // Aloca nova pagina
         
         if(mem == 0){
            // Falta de memoria: mata o processo
            p->killed = 1;
            break;
         }

         // Copia o conteudo da pagina antiga para a nova
         memmove(mem, (char*)P2V(pa), PGSIZE);
         
         // Ajusta flags: Ativa escrita, desativa COW
         uint flags = PTE_FLAGS(*pte);
         flags |= PTE_W;
         flags &= ~PTE_COW;

         // Atualiza a tabela para apontar para a nova pagina
         *pte = V2P(mem) | flags;
         
         // Decrementa referencia da pagina antiga
         dec_ref(pa);
         
         // Flush TLB
         lcr3(V2P(p->pgdir));
         
         // (Opcional) Log para debug
          //cprintf("COW: Copia realizada. PA: %p -> PID: %d\n", pa, p->pid);
         
      } else {
         // Se nao for COW, eh um erro legitimo (Segmentation Fault)
         cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
         p->killed = 1;
      }
    }
    break;
  // ------------------------------------------------------------

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
