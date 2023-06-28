//reference b10902062

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

void store_context(int id) {
    struct proc *proc = myproc();
    proc->thrd_context[id].epc = proc->trapframe->epc;
    proc->thrd_context[id].ra = proc->trapframe->ra;
    proc->thrd_context[id].sp = proc->trapframe->sp;
    proc->thrd_context[id].gp = proc->trapframe->gp;
    proc->thrd_context[id].tp = proc->trapframe->tp;
    proc->thrd_context[id].treg[0] = proc->trapframe->t0;
    proc->thrd_context[id].treg[1] = proc->trapframe->t1;
    proc->thrd_context[id].treg[2] = proc->trapframe->t2;
    proc->thrd_context[id].treg[3] = proc->trapframe->t3;
    proc->thrd_context[id].treg[4] = proc->trapframe->t4;
    proc->thrd_context[id].treg[5] = proc->trapframe->t5;
    proc->thrd_context[id].treg[6] = proc->trapframe->t6;
    proc->thrd_context[id].sreg[0] = proc->trapframe->s0;
    proc->thrd_context[id].sreg[1] = proc->trapframe->s1;
    proc->thrd_context[id].sreg[2] = proc->trapframe->s2;
    proc->thrd_context[id].sreg[3] = proc->trapframe->s3;
    proc->thrd_context[id].sreg[4] = proc->trapframe->s4;
    proc->thrd_context[id].sreg[5] = proc->trapframe->s5;
    proc->thrd_context[id].sreg[6] = proc->trapframe->s6;
    proc->thrd_context[id].sreg[7] = proc->trapframe->s7;
    proc->thrd_context[id].sreg[8] = proc->trapframe->s8;
    proc->thrd_context[id].sreg[8] = proc->trapframe->s8;
    proc->thrd_context[id].sreg[9] = proc->trapframe->s9;
    proc->thrd_context[id].sreg[10] = proc->trapframe->s10;
    proc->thrd_context[id].sreg[11] = proc->trapframe->s11;
    proc->thrd_context[id].areg[0] = proc->trapframe->a0;
    proc->thrd_context[id].areg[1] = proc->trapframe->a1;
    proc->thrd_context[id].areg[2] = proc->trapframe->a2;
    proc->thrd_context[id].areg[3] = proc->trapframe->a3;
    proc->thrd_context[id].areg[4] = proc->trapframe->a4;
    proc->thrd_context[id].areg[5] = proc->trapframe->a5;
    proc->thrd_context[id].areg[6] = proc->trapframe->a6;
    proc->thrd_context[id].areg[7] = proc->trapframe->a7;
}

void restore_context(int id) {
    struct proc *proc = myproc();
    proc->trapframe->epc = proc->thrd_context[id].epc;
    proc->trapframe->ra = proc->thrd_context[id].ra;
    proc->trapframe->sp = proc->thrd_context[id].sp;
    proc->trapframe->gp = proc->thrd_context[id].gp;
    proc->trapframe->tp = proc->thrd_context[id].tp;
    proc->trapframe->t0 = proc->thrd_context[id].treg[0];
    proc->trapframe->t1 = proc->thrd_context[id].treg[1];
    proc->trapframe->t2 = proc->thrd_context[id].treg[2];
    proc->trapframe->t3 = proc->thrd_context[id].treg[3];
    proc->trapframe->t4 = proc->thrd_context[id].treg[4];
    proc->trapframe->t5 = proc->thrd_context[id].treg[5];
    proc->trapframe->t6 = proc->thrd_context[id].treg[6];
    proc->trapframe->s0 = proc->thrd_context[id].sreg[0];
    proc->trapframe->s1 = proc->thrd_context[id].sreg[1];
    proc->trapframe->s2 = proc->thrd_context[id].sreg[2];
    proc->trapframe->s3 = proc->thrd_context[id].sreg[3];
    proc->trapframe->s4 = proc->thrd_context[id].sreg[4];
    proc->trapframe->s5 = proc->thrd_context[id].sreg[5];
    proc->trapframe->s6 = proc->thrd_context[id].sreg[6];
    proc->trapframe->s7 = proc->thrd_context[id].sreg[7];
    proc->trapframe->s8 = proc->thrd_context[id].sreg[8];
    proc->trapframe->s9 = proc->thrd_context[id].sreg[9];
    proc->trapframe->s10 = proc->thrd_context[id].sreg[10];
    proc->trapframe->s11 = proc->thrd_context[id].sreg[11];
    proc->trapframe->a0 = proc->thrd_context[id].areg[0];
    proc->trapframe->a1 = proc->thrd_context[id].areg[1];
    proc->trapframe->a2 = proc->thrd_context[id].areg[2];
    proc->trapframe->a3 = proc->thrd_context[id].areg[3];
    proc->trapframe->a4 = proc->thrd_context[id].areg[4];
    proc->trapframe->a5 = proc->thrd_context[id].areg[5];
    proc->trapframe->a6 = proc->thrd_context[id].areg[6];
    proc->trapframe->a7 = proc->thrd_context[id].areg[7];
}

// for mp3
uint64
sys_thrdstop(void) {
    int delay, context_id;
    uint64 context_id_ptr;
    uint64 handler, handler_arg;
    if (argint(0, &delay) < 0)
        return -1;
    if (argaddr(1, &context_id_ptr) < 0)
        return -1;
    if (argaddr(2, &handler) < 0)
        return -1;
    if (argaddr(3, &handler_arg) < 0)
        return -1;

    struct proc *proc = myproc();
    proc->thrd_delay = delay;
    proc->thrd_handler_ptr = handler;
    proc->thrd_handler_arg = handler_arg;
    proc->tick = 1;
    proc->ticks = 0;
    proc->finish = 0;
    char buf[sizeof(int)];
    if (copyin(proc->pagetable, buf, context_id_ptr, sizeof(int)) == 0)context_id = *((int *) buf);
    else return -1;

    if (context_id == -1) {
        proc->thrd_context_id = -1;
        int i = 0;
        while (proc->thrd_context_id_used[i] != 0) {
            i++;
            if (i == MAX_THRD_NUM)return -1;
        }
        proc->thrd_context_id_used[i] = 1;
        proc->thrd_context_id = i;
        *((int *) buf) = i;
        if (copyout(proc->pagetable, context_id_ptr, buf, sizeof(int)) == 0)return 0;
        else return -1;

    }

    else proc->thrd_context_id = context_id;
    return 0;
    //TODO: mp3

}

// for mp3
uint64
sys_cancelthrdstop(void) {
    int context_id, is_exit;
    if (argint(0, &context_id) < 0)
        return -1;
    if (argint(1, &is_exit) < 0)
        return -1;

    if (context_id < 0 || context_id >= MAX_THRD_NUM) {
        return -1;
    }

    struct proc *proc = myproc();
    if (is_exit == 0)store_context(context_id);
    else {
        proc->thrd_context_id_used[context_id] = 0;
        proc->finish = 0;
    }
    proc->tick = 0;
    //TODO: mp3

    return proc->ticks;
}

// for mp3
uint64
sys_thrdresume(void) {
    int context_id;
    if (argint(0, &context_id) < 0 || context_id < 0 || context_id > MAX_THRD_NUM - 1)
        return -1;


    //TODO: mp3
    restore_context(context_id);
    struct proc *proc = myproc();
    proc->finish = 0;
    return 0;
}