#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

int handle_pgfault() {
    struct proc *p = myproc();
    pagetable_t pagetable = p->pagetable;
    uint64 va = r_stval();
    va = PGROUNDDOWN(va);
    char *memory = kalloc();
    if (memory == 0) return -1;
    memset(memory, 0, PGSIZE);
    pte_t *pte = walk(pagetable, va, 1);
    if ((*pte & PTE_S)) {
        begin_op();
        read_page_from_disk(ROOTDEV, memory, (uint)PTE2BLOCKNO(*pte));
        end_op();
        *pte = (PTE_V | PA2PTE(memory) | PTE_FLAGS(*pte))& ~PTE_S;
    }
    else if (mappages(pagetable, va, PGSIZE, (uint64) memory, PTE_W | PTE_X | PTE_R | PTE_U) != 0)
            return -1;
    return 0;
}