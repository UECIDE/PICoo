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
extern const uint32_t _ticks_per_us = F_CPU / 2 / 1000000UL;

thread ISRThread;

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

void isrThreadStub(uint32_t x) {
    while(1) {
        Thread::Hibernate();
    }
}

void Thread::FillContext(threadFunction func, thread t, uint32_t param) {
    t->context.epc = (uint32_t)func;
    t->context.status = 1;
    t->context.ra = (uint32_t)&taskIsFinished;
    t->context.fp = 0x30303030UL;
    t->context.gp = (uint32_t)&_gp;
    t->context.t9 = 0x25252525UL;
    t->context.t8 = 0x24242424UL;
    t->context.s7 = 0x23232323UL;
    t->context.s6 = 0x22222222UL;
    t->context.s5 = 0x21212121UL;
    t->context.s4 = 0x20202020UL;
    t->context.s3 = 0x19191919UL;
    t->context.s2 = 0x18181818UL;
    t->context.s1 = 0x17171717UL;
    t->context.s0 = 0x16161616UL;
    t->context.t7 = 0x15151515UL;
    t->context.t6 = 0x14141414UL;
    t->context.t5 = 0x13131313UL;
    t->context.t4 = 0x12121212UL;
    t->context.t3 = 0x11111111UL;
    t->context.t2 = 0x10101010UL;
    t->context.t1 = 0x09090909UL;
    t->context.t0 = 0x08080808UL;
    t->context.a3 = 0x07070707UL;
    t->context.a2 = 0x06060606UL;
    t->context.a1 = 0x05050505UL;
    t->context.a0 = (uint32_t)param;
    t->context.v1 = 0x03030303UL;
    t->context.v0 = 0x02020202UL;
    t->context.at = 0x01010101UL;
    t->context.hi = 0x33333333UL;
    t->context.lo = 0x32323232UL;
}

thread Thread::Create(const char *n, threadFunction entry, uint32_t param, uint32_t stacksize) {
    
    if (ThreadList == NULL) { // This is our first ever thread
        sp_pos = (StackData + (STACK_SIZE/4) - 1);
        struct TCB *newTCB = (struct TCB *)malloc(sizeof(struct TCB));
        newTCB->stack_head = (StackData + (STACK_SIZE/4) - 1);
        newTCB->sp = newTCB->stack_head;
        *newTCB->sp-- = 0;
        *newTCB->sp-- = 0;
        *newTCB->sp-- = 0;
        *newTCB->sp-- = 0;
        newTCB->stack_size = stacksize;
        newTCB->entry = entry;
        newTCB->name = n;

        FillContext(entry, newTCB, param);

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

        *newTCB->sp-- = 0;
        *newTCB->sp-- = 0;
        *newTCB->sp-- = 0;
        *newTCB->sp-- = 0;

        FillContext(entry, newTCB, param);

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
        *foundThread->sp-- = 0;
        *foundThread->sp-- = 0;
        *foundThread->sp-- = 0;
        *foundThread->sp-- = 0;
        FillContext(entry, foundThread, param);
        foundThread->state_data = 0;
        foundThread->state = RUN;
        foundThread->name = n;
        foundThread->entry = entry;
        return foundThread;
    }
    return 0;
}

void Thread::Sleep(uint32_t ms) {
    USleep(Math::MulU(ms, 1000));
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
    void processInterrupt() {
        TRISCCLR = 1<<1;
        LATCSET = 1<<1;
        for (uint32_t irq = 0; irq < NUM_INT_REQUEST; irq++) {
            if (Interrupt::GetFlag(irq)) {
                Interrupt::ExecuteInterrupt(irq);
                Interrupt::ClearFlag(irq);
                LATCCLR = 1<<1;
                return;
            }
        }
        LATCCLR = 1<<1;
    }
}

void Thread::SelectNextThread() {
    if (currentThread == NULL) {
        currentThread = IdleThread;
    }

    // Activate any threads that should wake from sleep, and check any
    // mutex locks and wake the first threads that want an available one

    _MicrosecondCounter = 0;
    _MillisecondCounter = 0;

    for (thread scan = ThreadList; scan; scan = scan->next) {
        scan->runtime_us += Math::DivU(scan->runtime, _ticks_per_us);
        scan->runtime = Math::ModU(scan->runtime, _ticks_per_us);

        scan->runtime_ms += Math::DivU(scan->runtime_us, 1000);
        scan->runtime_us = Math::ModU(scan->runtime_us, 1000);

        _MicrosecondCounter += scan->runtime_us;
        _MillisecondCounter += scan->runtime_ms;
    }
    _MillisecondCounter += Math::DivU(_MicrosecondCounter, 1000);
    _MicrosecondCounter = Math::ModU(_MicrosecondCounter, 1000);
        
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
            if ((_MicrosecondCounter + Math::MulU(_MillisecondCounter, 1000)) - scan->state_data >= scan->state_data_extra) {
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

    ISRThread = Thread::Create("[kernel_isr]", isrThreadStub);

    Interrupt::SetPriority(_CORE_TIMER_VECTOR, 6, 6); //_CT_IPL_IPC, _CT_SPL_IPC);
    Interrupt::SetVector(0, ThreadScheduler);
    Interrupt::AttachInterrupt(_CORE_TIMER_IRQ, &Thread::SelectNextThread);
    Interrupt::EnableIRQ(_CORE_TIMER_IRQ);

}

uint32_t Thread::Runtime() {
    return Runtime((struct TCB *)currentThread);
}

uint32_t Thread::Runtime(thread t) {
    return t->runtime_ms;
}

uint32_t Thread::Milliseconds() {
    return _MillisecondCounter;
}

uint32_t Thread::Microseconds() {
    return _MicrosecondCounter + (_MillisecondCounter * 1000);
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
