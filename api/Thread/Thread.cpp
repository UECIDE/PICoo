#include <PICoo.h>
#include <stdlib.h>
#include <stdio.h>

extern uint32_t _gp;

struct TCB *ThreadList = NULL;

uint32_t __attribute__((aligned(4))) StackData[STACK_SIZE/4];

volatile struct TCB *currentThread = NULL;

volatile uint32_t _MillisecondCounter = 0;
volatile uint32_t _MicrosecondCounter = 0;
volatile uint32_t _MicroMillisCounter = 0;

extern const uint32_t _core_tick = (F_CPU / 2 / 1000000UL) * CORE_US;
extern const uint32_t _core_us = CORE_US;

void taskIsFinished() {
    currentThread->state = Thread::ZOMBIE;
    currentThread->sp = currentThread->stack_head;
    while(1);
}

struct TCB KernelBootTCB;

extern thread IdleThread;
extern thread MasterThread;
uint32_t *sp_pos;
uint32_t epos;
uint32_t scratch;

extern const uint32_t _core_tick;
extern const uint32_t _core_us;

extern "C" {
    extern void ThreadScheduler();
    extern void restoreThreadContext();
}

#define c0_read_count(dest)     asm volatile("mfc0 %0,$9" : "=r" (dest))
#define c0_read_comp(dest)      asm volatile("mfc0 %0,$11" : "=r" (dest))
#define c0_read_status(dest)    asm volatile("mfc0 %0,$12" : "=r" (dest))
#define c0_read_cause(dest)     asm volatile("mfc0 %0,$13" : "=r" (dest))
#define c0_read_epc(dest)       asm volatile("mfc0 %0,$14" : "=r" (dest))
#define c0_write_count(src)     asm volatile("mtc0 %0,$9" : : "r" (src))
#define c0_write_comp(src)      asm volatile("mtc0 %0,$11" : : "r" (src))
#define c0_write_status(src)    asm volatile("mtc0 %0,$12" : : "r" (src))
#define c0_write_cause(src)     asm volatile("mtc0 %0, $13" : : "r" (src))
#define c0_write_epc(src)       asm volatile("mtc0 %0,$14" : : "r" (src))

uint32_t *Thread::FillStack(threadFunction func, uint32_t *stk, uint32_t param) {
    *stk-- = 0;
    *stk-- = 0;
    *stk-- = 0;
    *stk-- = 0;
    *stk-- = (uint32_t)func;
    *stk-- = 1;
    *stk-- = (uint32_t)&taskIsFinished;
    *stk-- = 0x30303030UL;
    *stk-- = (uint32_t)&_gp;
    *stk-- = 0x25252525UL;
    *stk-- = 0x24242424UL;
    *stk-- = 0x23232323UL;
    *stk-- = 0x22222222UL;
    *stk-- = 0x21212121UL;
    *stk-- = 0x20202020UL;
    *stk-- = 0x19191919UL;
    *stk-- = 0x18181818UL;
    *stk-- = 0x17171717UL;
    *stk-- = 0x16161616UL;
    *stk-- = 0x15151515UL;
    *stk-- = 0x14141414UL;
    *stk-- = 0x13131313UL;
    *stk-- = 0x12121212UL;
    *stk-- = 0x11111111UL;
    *stk-- = 0x10101010UL;
    *stk-- = 0x09090909UL;
    *stk-- = 0x08080808UL;
    *stk-- = 0x07070707UL;
    *stk-- = 0x06060606UL;
    *stk-- = 0x05050505UL;
    *stk-- = (uint32_t)param;
    *stk-- = 0x03030303UL;
    *stk-- = 0x02020202UL;
    *stk-- = 0x01010101UL;
    *stk-- = 0x33333333UL;
    *stk = 0x32323232UL;

    return stk;
}

thread Thread::Create(const char *n, threadFunction entry, uint32_t param, uint32_t stacksize) {
    
    if (ThreadList == NULL) { // This is our first ever thread
        sp_pos = (StackData + (STACK_SIZE/4) - 1);
        struct TCB *newTCB = (struct TCB *)malloc(sizeof(struct TCB));
        newTCB->stack_head = (StackData + (STACK_SIZE/4) - 1);
        newTCB->sp = newTCB->stack_head;
        newTCB->stack_size = stacksize;
        newTCB->entry = entry;
        newTCB->name = n;

        newTCB->sp = FillStack(entry, newTCB->sp, param);

        newTCB->state_data = 0;
        newTCB->state = RUN;
        newTCB->next = NULL;
        ThreadList = newTCB;
        sp_pos -= stacksize/4;
        return newTCB;
    }

    // It wasn't the first thread, so let's look for a suitable
    // zombie thread to hijack.  We scan the threads, and if we
    // find one that has the right size stack, we use it immediately.
    // If not, we take the thread with the smallest stack that
    // is bigger than the one we want.
    // A proper space allocation system would be better, which
    // keeps track of stack holes and reuses them for other threads
    // but for now "this will do".

    thread foundThread = 0;
    thread lastThread = 0;
    thread smallestThread = 0;
    for (thread scan = ThreadList; scan != NULL; scan = scan->next) {
        if (scan->state == ZOMBIE) {
            if (scan->stack_size == stacksize) {
                foundThread = scan;
                break;
            }
            if (scan->stack_size > stacksize) {
                if (smallestThread == 0) {
                    smallestThread = scan;
                } else if (scan->stack_size < smallestThread->stack_size) {
                    smallestThread = scan;
                }
            }
        }
        if (!scan->next) {
            lastThread = scan;
        }
    }

    if (foundThread == 0) {
        foundThread = smallestThread;
    }

    if (foundThread == 0) {
        struct TCB *newTCB = (struct TCB *)malloc(sizeof(struct TCB));
        newTCB->stack_head = sp_pos;
        newTCB->sp = newTCB->stack_head;
        newTCB->stack_size = stacksize;

        newTCB->sp = FillStack(entry, newTCB->sp, param);

        newTCB->state_data = 0;
        newTCB->entry = entry;
        newTCB->state = RUN;
        newTCB->next = NULL;
        newTCB->name = n;
        lastThread->next = newTCB;
        sp_pos -= stacksize/4;
        return newTCB;
    } else {
        foundThread->sp = foundThread->stack_head;
        foundThread->sp = FillStack(entry, foundThread->sp, param);
        foundThread->state_data = 0;
        foundThread->state = RUN;
        foundThread->name = n;
        foundThread->entry = entry;
        return foundThread;
    }
    return 0;
}

void Thread::Sleep(uint32_t ms) {
    USleep(ms * 1000);
}

void Thread::USleep(uint32_t us) {
    uint32_t now = Microseconds();
    currentThread->state_data = now;
    currentThread->state_data_extra = us;
    currentThread->state = SLEEP;

    // If we don't have this here we'll be executing some more
    // instructions in our thread before the SLEEP state actually
    // takes effect.
    while (currentThread->state == SLEEP) {
        continue;
    }
}

void Thread::Hibernate() {
    currentThread->state = HIBER;
    while (currentThread->state == HIBER) {
        continue;
    }
}

extern "C" {
    extern void _mon_putc(int);
    void selectNextThread() {
        if (currentThread == NULL) {
            currentThread = IdleThread;
        }

        _MicrosecondCounter += _core_us;
        _MicroMillisCounter += _core_us;
        while (_MicroMillisCounter >= 1000) {
            _MillisecondCounter++;
            _MicroMillisCounter -= 1000;
        }

        // Activate any threads that should wake from sleep, and check any
        // mutex locks and wake the first threads that want an available one

        for (thread scan = ThreadList; scan; scan = scan->next) {
            if (scan->state == Thread::SEMWAIT) {
                volatile uint32_t *sem = (uint32_t *)(scan->state_data);
                if (*sem == 1) {
                    for (thread scan2 = scan; scan2; scan2 = scan2->next) {
                        if ((scan2->state == Thread::SEMWAIT) && (scan2->state_data == scan->state_data)) {
                            scan2->state = Thread::RUN;
                        }
                    }
                    *sem = 0;
                }
                continue;
            }
            if (scan->state == Thread::SLEEP) {
                if (_MicrosecondCounter - scan->state_data >= scan->state_data_extra) {
                    scan->state = Thread::RUN;
                }
                continue;
            }
            if (scan->state == Thread::MUTEX) {
                volatile uint32_t *mutex = (uint32_t *)(scan->state_data);
                if (*mutex == 0) {
                    *mutex = 1;
                    scan->state = Thread::RUN;
                }
            }
        }

        // Now look for the next running thread in the list.
        // If none is found that is active, then use the idle thread.

        thread lastThread = (thread)currentThread;
        int loops = 0;
        while (1) {
            loops++;
            if (loops > 10) {
                currentThread = IdleThread;
                break;
            }
            currentThread = currentThread->next;

            if (currentThread == NULL) {
                currentThread = ThreadList;
            }

            if (currentThread == IdleThread) {
                currentThread = currentThread->next;

                if (currentThread == NULL) {
                    currentThread = ThreadList;
                }
            }

            if (currentThread == lastThread) {
                if (currentThread->state == Thread::RUN) {
                    break;
                } else {
                    currentThread = IdleThread;
                    break;
                }
            }

            if (currentThread->state == Thread::RUN) {
                break;
            }

        }

        currentThread->runtime += _core_us;
    }
}

// This is going to be a fun one to write.  We need to jump into the
// first thread in the list that is capable of running.  In theory
// the function should never return, as control is wrestled away
// from the normal flow of operation and into the threads system.
// The first thread in the list should theoretically always be the
// idle thread, so that is a good place to start.
// We also need to start the core timer, and configure the interrupts.
void __attribute__((nomips16)) Thread::Start() {
    currentThread = NULL;

    // Set up the core timer
    uint32_t val = _core_tick;
    asm volatile("mtc0  $0,$9"); // Clear core timer counter
    asm volatile("mtc0  %0,$11" :: "r"(_core_tick)); // Set up the tick

    Interrupt::SetPriority(_CORE_TIMER_VECTOR, _CT_IPL_IPC, _CT_SPL_IPC);
    Interrupt::SetVector(_CORE_TIMER_VECTOR, ThreadScheduler);
    Interrupt::EnableIRQ(_CORE_TIMER_IRQ);

}

uint32_t Thread::Runtime() {
    return Runtime((struct TCB *)currentThread);
}

uint32_t Thread::Runtime(thread t) {
    return t->runtime / 1000;
}

uint32_t Thread::Milliseconds() {
    return _MillisecondCounter;
}

uint32_t Thread::Microseconds() {
    return _MicrosecondCounter;
}

void Thread::Terminate() {
    currentThread->state = Thread::ZOMBIE;
    currentThread->sp = currentThread->stack_head;
}

void Thread::Wake(thread t) {
    if (t->state == Thread::HIBER || t->state == Thread::SLEEP) {
        t->state = Thread::RUN;
    }
}

void Thread::Lock(mutex& m) {
    currentThread->state_data = (uint32_t)&m;
    currentThread->state = Thread::MUTEX;
    while (currentThread->state == Thread::MUTEX) {
        continue;
    }
}

void Thread::Unlock(mutex& m) {
    m = 0;
    Thread::USleep(0);
}

void Thread::Signal(semaphore& s) {
    s = 1;
    Thread::USleep(0);
}

void Thread::Wait(semaphore& s) {
    currentThread->state_data = (uint32_t)&s;
    currentThread->state = Thread::SEMWAIT;
    while(currentThread->state == Thread::SEMWAIT) {
        continue;
    }
}

