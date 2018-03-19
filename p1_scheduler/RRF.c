#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "mythread.h"
#include "interrupt.h"

#include "queue.h"

TCB* scheduler();
void activator();
void timer_interrupt(int sig);
void network_interrupt(int sig);

/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N];

/* Waiting queues for RRF policy */
struct queue *q_low;
struct queue *q_high;

/* Current running thread */
static TCB* running;
static int current = 0;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init=0;

/* Thread control block for the idle thread */
static TCB idle;
static void idle_function(){
        while(1);
}

/* Initialize the thread library */
void init_mythreadlib() {
        int i;
        /* Create context for the idle thread */
        if(getcontext(&idle.run_env) == -1) {
                perror("*** ERROR: getcontext in init_thread_lib");
                exit(-1);
        }
        idle.state = IDLE;
        idle.priority = SYSTEM;
        idle.function = idle_function;
        idle.run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
        idle.tid = -1;
        if(idle.run_env.uc_stack.ss_sp == NULL) {
                printf("*** ERROR: thread failed to get stack space\n");
                exit(-1);
        }
        idle.run_env.uc_stack.ss_size = STACKSIZE;
        idle.run_env.uc_stack.ss_flags = 0;
        idle.ticks = QUANTUM_TICKS;
        makecontext(&idle.run_env, idle_function, 1);

        t_state[0].state = INIT;
        t_state[0].priority = LOW_PRIORITY;
        t_state[0].ticks = QUANTUM_TICKS;
        if(getcontext(&t_state[0].run_env) == -1) {
                perror("*** ERROR: getcontext in init_thread_lib");
                exit(5);
        }

        for(i=1; i<N; i++) {
                t_state[i].state = FREE;
        }

        t_state[0].tid = 0;

        /* Set thread 0 as running */
        printf("*** THREAD %d READY\n", 0);
        running = &t_state[0];

        /* Initialize queues for RRF */
        q_low = queue_new();
        q_high = queue_new();

        /* Initialize network and clock interrupts */
        init_network_interrupt();
        init_interrupt();
}


/* Create and intialize a new thread with body fun_addr and one integer argument */
int mythread_create (void (*fun_addr)(),int priority)
{
        int i;

        if (!init) { init_mythreadlib(); init=1;}
        for (i=0; i<N; i++)
                if (t_state[i].state == FREE) break;
        if (i == N) return(-1);
        if(getcontext(&t_state[i].run_env) == -1) {
                perror("*** ERROR: getcontext in my_thread_create");
                exit(-1);
        }
        t_state[i].state = INIT;
        t_state[i].priority = priority;
        t_state[i].function = fun_addr;
        t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
        if(t_state[i].run_env.uc_stack.ss_sp == NULL) {
                printf("*** ERROR: thread failed to get stack space\n");
                exit(-1);
        }
        t_state[i].tid = i;
        t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
        t_state[i].run_env.uc_stack.ss_flags = 0;
        makecontext(&t_state[i].run_env, fun_addr, 1);

        if(priority == LOW_PRIORITY) {
                disable_interrupt();
                enqueue(q_low, &t_state[i]);
                printf("*** THREAD %d READY\n", i);
                enable_interrupt();
        }

        if(priority == HIGH_PRIORITY && running->priority == LOW_PRIORITY) {
                printf("*** THREAD %d READY\n", i);
                printf("*** THREAD %d PREEMPTED: SET CONTEXT OF %d\n", running->tid, i);
                activator(&t_state[i]);
        } else if(priority == HIGH_PRIORITY) {
                disable_interrupt();
                enqueue(q_high, &t_state[i]);
                printf("*** THREAD %d READY\n", i);
                enable_interrupt();
        }

        return i;
} /****** End my_thread_create() ******/

/* Read network syscall */
int read_network()
{
        return 1;
}

/* Network interrupt  */
void network_interrupt(int sig)
{
}

/* Free terminated thread and exits */
void mythread_exit() {
        int tid = mythread_gettid();
        printf("*** THREAD %d FINISHED\n", tid);
        t_state[tid].state = FREE;
        free(t_state[tid].run_env.uc_stack.ss_sp);

        TCB* next = scheduler();
        activator(next);
}

/* Sets the priority of the calling thread */
void mythread_setpriority(int priority) {
        int tid = mythread_gettid();
        t_state[tid].priority = priority;
}

/* Returns the priority of the calling thread */
int mythread_getpriority(int priority) {
        int tid = mythread_gettid();
        return t_state[tid].priority;
}


/* Get the current thread id.  */
int mythread_gettid(){
        if (!init) { init_mythreadlib(); init=1;}
        return current;
}


/* FIFO para alta prioridad, RR para baja*/
TCB* scheduler(){
        if(queue_empty(q_low) == 1 && queue_empty(q_high) == 1) {
                /* No threads waiting */
                if(running->state != FREE) {
                    printf("*** THREAD %d FINISHED\n", current);
                }
                printf("FINISH\n");
                exit(1);
        }

        if(queue_empty(q_high) != 1) {
                /* High priority threads waiting to be executed */
                disable_interrupt();
                TCB* nextH = dequeue(q_high);
                enable_interrupt();
                return nextH;
        }

        /* No high priority threads waiting, execute low priority ones */
        disable_interrupt();
        TCB* next = dequeue(q_low);
        enable_interrupt();
        return next;
}


/* Timer interrupt  */
void timer_interrupt(int sig)
{
        /* Running thread has been executed for 1 tick, so we decrease the count */
        running->ticks--;
        /* If the thread has not finished but completed the slice, change to the
         * next thread */
        //printf("Thread %d running with %d ticks remaining, P%d\n", current, running->ticks, running->priority);
        if(running->ticks == 0 && running->priority == LOW_PRIORITY) {
                /* Reset the counter for this thread */
                running->ticks = QUANTUM_TICKS;
                /* Change to the new thread */
                activator(scheduler());
        }
}

/* Activator */
void activator(TCB* next){
        if(running == next) return;
        if(running->state == FREE) {
                /* Thread has finished, we just setcontext the following thread */
                printf("*** THREAD %d FINISHED: SET CONTEXT OF %d\n", running->tid,  next->tid);
                running = next;
                current = next->tid;
                running->ticks = QUANTUM_TICKS;
                setcontext(&(next->run_env));
        } else if(running != next) {
                disable_interrupt();
                TCB* aux;
                memcpy(&aux, &running, sizeof(TCB*));

                enqueue(q_low, running);
                enable_interrupt();

                running = next;
                current = next->tid;
                running->ticks = QUANTUM_TICKS;
                printf("*** SWAPCONTEXT FROM %d TO %d\n", aux->tid, next->tid);
                swapcontext(&(aux->run_env), &(next->run_env));
        }
}
