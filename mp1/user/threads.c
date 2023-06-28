#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"

#define NULL 0


static struct thread *current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg) {
    struct thread *t = (struct thread *) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long) * 0x100);
    new_stack_p = new_stack + 0x100 * 8 - 0x2 * 8;
    t->fp = f;
    t->arg = arg;
    t->ID = id;
    t->buf_set = 0;
    t->stack = (void *) new_stack;
    t->stack_p = (void *) new_stack_p;
    id++;

    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    return t;
}

void thread_add_runqueue(struct thread *t) {
    if (current_thread == NULL) {
        current_thread = t;
        t->previous = t;
        t->next = t;
    } else {
        t->sig_handler[0] = current_thread->sig_handler[0];
        t->sig_handler[1] = current_thread->sig_handler[1];
        current_thread->previous->next = t;
        t->previous = current_thread->previous;
        current_thread->previous = t;
        t->next = current_thread;
    }
    //let the child thread t inherit the signal handlers from current thread.
}

void thread_yield(void) {
    //saving its context to the jmp buf in struct thread using setjmp.
    if (current_thread->signo == -1) {//no signo
        int ret = setjmp(current_thread->env);
        if (ret == 0) {//setjmp
            schedule();
            dispatch();
        }
    } else {
        int ret = setjmp(current_thread->handler_env);
        if (ret == 0) {//setjmp
            schedule();
            dispatch();
        }
    }
    //save the context in different jmpbufs according to
    //whether the thread is executing the signal handler or not.
}

void dispatch(void) {
    // TODO
    //moving the stack pointer sp to the allocated stack of the thread.
    if (current_thread->signo != -1) {
        if (current_thread->sig_handler[current_thread->signo] == NULL_FUNC ) {
            thread_exit();
        }

        //In case the handler has never run before, you may need to do some initialization.
        if (current_thread->handler_buf_set == 0) {//signo not run before
            current_thread->handler_buf_set = 1;
            if (setjmp(env_tmp) == 0) {
                if (current_thread->buf_set == 0){//not called before
                    env_tmp->sp = (unsigned long) current_thread->stack_p;
                }
                else{//called
                    env_tmp->sp = (unsigned long) current_thread->env->sp;
                }
                //current_thread->handler_env->sp = (unsigned long) current_thread->stack_p - 1024;
                longjmp(env_tmp, 1);
            }
            current_thread->sig_handler[current_thread->signo](current_thread->signo);
            current_thread->signo = -1;
            current_thread->handler_buf_set = 0;

        } else {//signo run before
            //If the signal handler was executed before,longjmp
            longjmp(current_thread->handler_env, 1);
        }

    }
        // stack pointer sp could be accessed and modified using setjmp and longjmp.
        //If the thread was executed before, restoring the context with longjmp is enough.
        //the thread needs to be removed from the runqueue and the next one has to be dispatched.
        //  The easiest way to do this is to call thread exit().
    if (current_thread->buf_set == 1) {
        longjmp(current_thread->env, 1);
    } else {
        current_thread->buf_set = 1;
        if (setjmp(env_tmp) == 0) {
            env_tmp->sp = (unsigned long) current_thread->stack_p;
            longjmp(env_tmp, 1);
        }
        current_thread->fp(current_thread->arg);
        thread_exit();
    }
}


void schedule(void) {
    current_thread = current_thread->next;
}

void thread_exit(void) {
    if (current_thread->next != current_thread) {
        // TODO
        current_thread->previous->next = current_thread->next;
        current_thread->next->previous = current_thread->previous;
        struct thread *temp = current_thread;
        current_thread = current_thread->next;
        free(temp->stack);
        free(temp);
        dispatch();
    } else {
        // TODO
        // Hint: No more thread to execute
        free(current_thread->stack);
        free(current_thread);
        longjmp(env_st, 1);
    }
}

void thread_start_threading(void) {
    // TODO
    if (setjmp(env_st) == 0) {
        dispatch();
    }


}

// part 2
void thread_register_handler(int signo, void (*handler)(int)) {
    // TODO
    //first argument as an integer which represents signal number
    //Second argument as a pointer to the signal-handling function.
    //sets handler to current thread for signal signo.
    current_thread->sig_handler[signo] = handler;
}

void thread_kill(struct thread *t, int signo) {
    // TODO
    //sends a signal signo to t
    //If t has a signal handler for the signal signo, it will be executed firstly
    //Otherwise, t will be killed, that is, thread exit() is called
    //thread returns from its signal handler, it should continue executing
    //only sends a signal and does not trigger any context switch.
    t->signo = signo;
}