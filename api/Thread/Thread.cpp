#include <PICoo.h>
#include <stdlib.h>
#include <stdio.h>

extern uint32_t _gp;

struct TCB *ThreadList = NULL;

uint32_t __attribute__((aligned(4))) StackData[STACK_SIZE/4];

volatile struct TCB *currentThread = NULL;

volatile uint32_t _MillisecondCounter = 0;

void taskIsFinished() {
    printf("Thread finished: %08X\n", currentThread);
    currentThread->state = Thread::ZOMBIE;
    while(1);
}

uint32_t _temp_sp;
extern thread IdleThread;
uint32_t _looped = 0;
uint32_t *sp_pos;
uint32_t epos;

uint32_t *Thread::FillStack(threadFunction func, uint32_t *stk, uint32_t param) {
    printf("Entry point is %08X...", func);
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

    printf("Stack is at %08X...", stk);

    return stk;
}

thread Thread::Create(threadFunction entry, uint32_t param, uint32_t stacksize) {
    
    if (ThreadList == NULL) { // This is our first ever thread
        sp_pos = (StackData + STACK_SIZE - 4);
        struct TCB *newTCB = (struct TCB *)malloc(sizeof(struct TCB));
        newTCB->stack_head = (StackData + STACK_SIZE - 4);
        newTCB->sp = newTCB->stack_head;
        newTCB->stack_size = stacksize;

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

    struct TCB *foundThread = 0;
    struct TCB *lastThread = 0;
    struct TCB *smallestThread = 0;
    for (struct TCB *scan = ThreadList; scan != NULL; scan = scan->next) {
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
        newTCB->state = RUN;
        newTCB->next = NULL;
        lastThread->next = newTCB;
        sp_pos -= stacksize;
        return newTCB;
    } else {
        foundThread->sp = foundThread->stack_head;
        foundThread->sp = FillStack(entry, foundThread->sp, param);
        foundThread->state_data = 0;
        foundThread->state = RUN;
        return foundThread;
    }
    return 0;
}

void Thread::Sleep(uint32_t ms) {
    uint32_t now = Uptime();
    currentThread->state_data = now + ms;
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

void __attribute__((interrupt(),nomips16)) ThreadScheduler() {

    asm volatile("la $k0, currentThread");
    asm volatile("lw $k1, 0($k0)");
    asm volatile("beqz noThread");

    asm volatile("mfc0 $k1, $14");
    asm volatile("addiu $sp, $sp, -128");
    asm volatile("sw $k1, 124($sp)");
    asm volatile("mfc0 $k1, $12");
    asm volatile("mfc0 $k0, $13");
    
    asm volatile("sw $k1, 120($sp)");

    asm volatile("ins $k1, $zero, 1, 15");
    asm volatile("ext $k0, $k0, 10, 6");
    asm volatile("ins $k1, $k0, 10, 6");
    asm volatile("mtc0 $k1, $12");
    
    asm volatile("sw $31, 116($sp)");
    asm volatile("sw $30, 112($sp)");
    asm volatile("sw $28, 108($sp)");
    asm volatile("sw $25, 104($sp)");
    asm volatile("sw $24, 100($sp)");
    asm volatile("sw $23,  96($sp)");
    asm volatile("sw $22,  92($sp)");
    asm volatile("sw $21,  88($sp)");
    asm volatile("sw $20,  84($sp)");
    asm volatile("sw $19,  80($sp)");
    asm volatile("sw $18,  76($sp)");
    asm volatile("sw $17,  72($sp)");
    asm volatile("sw $16,  68($sp)");
    asm volatile("sw $15,  64($sp)");
    asm volatile("sw $14,  60($sp)");
    asm volatile("sw $13,  56($sp)");
    asm volatile("sw $12,  52($sp)");
    asm volatile("sw $11,  48($sp)");
    asm volatile("sw $10,  44($sp)");
    asm volatile("sw  $9,  40($sp)");
    asm volatile("sw  $8,  36($sp)");
    asm volatile("sw  $7,  32($sp)");
    asm volatile("sw  $6,  28($sp)");
    asm volatile("sw  $5,  24($sp)");
    asm volatile("sw  $4,  20($sp)");
    asm volatile("sw  $3,  16($sp)");
    asm volatile("sw  $2,  12($sp)");
    asm volatile("sw  $1,   8($sp)");
    asm volatile("mfhi $t0");
    asm volatile("mflo $t1");
    asm volatile("sw $t0, 4($sp)");
    asm volatile("sw $t1, 0($sp)");


    asm volatile("la $t0, currentThread");
    asm volatile("lw $t1, 0($t0)");
    asm volatile("sw $sp, 0($t1)");

noThread:

    _MillisecondCounter++;

    do {
        currentThread = currentThread->next;
        if (currentThread == IdleThread) {
            continue;
        }
        if (currentThread == NULL) {
            currentThread = ThreadList;
            _looped++;
        }

        // We have searched long enough for an active thread - there isn't one, so use the
        // idle thread instead.
        if (_looped > 2) {
            currentThread = IdleThread;
            break;
        }

        if (currentThread->state != Thread::RUN) {
            if (currentThread->state == Thread::SLEEP) {
                if (_MillisecondCounter == currentThread->state_data) {
                    currentThread->state == Thread::RUN;
                }
            } else if (currentThread->state == Thread::MUTEX) {
                volatile uint32_t *mutex = (uint32_t *)(currentThread->state_data);
                if (*mutex == 0) {
                    *mutex = 1;
                    currentThread->state = Thread::RUN;
                }
            }
        }
    } while(currentThread->state != Thread::RUN);

    currentThread->runtime++;

    asm volatile("di $t2");
    asm volatile("ehb");
    asm volatile("mfc0 $k0, $13");
    asm volatile("ins $k0, $zero, 8, 1");
    asm volatile("mtc0 $k0, $13");
    asm volatile("lui $k0, %hi(IFS0CLR)");
    asm volatile("ori $k1, $zero, 2");
    asm volatile("sw $k1, %lo(IFS0CLR)($k0)");
    asm volatile("mtc0 $t2, $12");
    
    asm volatile("la $t0, currentThread");
    asm volatile("lw $t1, 0($t0)");
    asm volatile("lw $sp, 0($t1)");

    asm volatile("la $t0, _temp_sp");
    asm volatile("lw $sp, 0($t0)");

    asm volatile("mfc0 $k0, $13");
    asm volatile("ins $k0, $zero, 8, 1");
    asm volatile("mtc0 $k0, $13");
    
    asm volatile("lw $t0, 0($sp)");
    asm volatile("lw $t1, 4($sp)");
    asm volatile("mtlo $t0");
    asm volatile("mthi $t1");

    asm volatile("lw  $1,   8($sp)");
    asm volatile("lw  $2,  12($sp)");
    asm volatile("lw  $3,  16($sp)");
    asm volatile("lw  $4,  20($sp)");
    asm volatile("lw  $5,  24($sp)");
    asm volatile("lw  $6,  28($sp)");
    asm volatile("lw  $7,  32($sp)");
    asm volatile("lw  $8,  36($sp)");
    asm volatile("lw  $9,  40($sp)");
    asm volatile("lw $10,  44($sp)");
    asm volatile("lw $11,  48($sp)");
    asm volatile("lw $12,  52($sp)");
    asm volatile("lw $13,  56($sp)");
    asm volatile("lw $14,  60($sp)");
    asm volatile("lw $15,  64($sp)");
    asm volatile("lw $16,  68($sp)");
    asm volatile("lw $17,  72($sp)");
    asm volatile("lw $18,  76($sp)");
    asm volatile("lw $19,  80($sp)");
    asm volatile("lw $20,  84($sp)");
    asm volatile("lw $21,  88($sp)");
    asm volatile("lw $22,  92($sp)");
    asm volatile("lw $23,  96($sp)");
    asm volatile("lw $24, 100($sp)");
    asm volatile("lw $25, 104($sp)");
    asm volatile("lw $28, 108($sp)");
    asm volatile("lw $30, 112($sp)");
    asm volatile("lw $31, 116($sp)");

    asm volatile("lw $k1, 124($sp)");
    asm volatile("lw $k0, 120($sp)");
    asm volatile("mtc0 $k1, $14");
    asm volatile("addiu $sp, $sp, 128");
    asm volatile("mtc0 $k0, $12");
}

// This is going to be a fun one to write.  We need to jump into the
// first thread in the list that is capable of running.  In theory
// the function should never return, as control is wrestled away
// from the normal flow of operation and into the threads system.
// The first thread in the list should theoretically always be the
// idle thread, so that is a good place to start.
// We also need to start the core timer, and configure the interrupts.
void __attribute__((nomips16)) Thread::Start() {
    currentThread = ThreadList;

    // Set up the core timer
    uint32_t val = CORE_TICK_RATE;
    asm volatile("mtc0  $0,$9"); // Clear core timer counter
    asm volatile("mtc0  %0,$11" : "+r"(val)); // Set up the tick

    Interrupt::SetPriority(_CORE_TIMER_VECTOR, _CT_IPL_IPC, _CT_SPL_IPC);
    Interrupt::SetVector(_CORE_TIMER_VECTOR, ThreadScheduler);
    Interrupt::EnableIRQ(_CORE_TIMER_IRQ);

    currentThread = IdleThread;
    _temp_sp = currentThread->sp;

    printf("Boink\n");
    asm volatile("la $t0, _temp_sp");
    asm volatile("lw $sp, 0($t0)");

//    asm volatile("mfc0 $k0, $13");
//    asm volatile("ins $k0, $zero, 8, 1");
//    asm volatile("mtc0 $k0, $13");
    
    asm volatile("lw $t0, 0($sp)");
    asm volatile("lw $t1, 4($sp)");
    asm volatile("mtlo $t0");
    asm volatile("mthi $t1");

    asm volatile("lw  $1,   8($sp)");
    asm volatile("lw  $2,  12($sp)");
    asm volatile("lw  $3,  16($sp)");
    asm volatile("lw  $4,  20($sp)");
    asm volatile("lw  $5,  24($sp)");
    asm volatile("lw  $6,  28($sp)");
    asm volatile("lw  $7,  32($sp)");
    asm volatile("lw  $8,  36($sp)");
    asm volatile("lw  $9,  40($sp)");
    asm volatile("lw $10,  44($sp)");
    asm volatile("lw $11,  48($sp)");
    asm volatile("lw $12,  52($sp)");
    asm volatile("lw $13,  56($sp)");
    asm volatile("lw $14,  60($sp)");
    asm volatile("lw $15,  64($sp)");
    asm volatile("lw $16,  68($sp)");
    asm volatile("lw $17,  72($sp)");
    asm volatile("lw $18,  76($sp)");
    asm volatile("lw $19,  80($sp)");
    asm volatile("lw $20,  84($sp)");
    asm volatile("lw $21,  88($sp)");
    asm volatile("lw $22,  92($sp)");
    asm volatile("lw $23,  96($sp)");
    asm volatile("lw $24, 100($sp)");
    asm volatile("lw $25, 104($sp)");
    asm volatile("lw $28, 108($sp)");
    asm volatile("lw $30, 112($sp)");
    asm volatile("lw $31, 116($sp)");

    asm volatile("lw $k1, 124($sp)");
    asm volatile("lw $k0, 120($sp)");
    asm volatile("mtc0 $k1, $14");
    asm volatile("addiu $sp, $sp, 128");
    asm volatile("mtc0 $k0, $12");
}

uint32_t Thread::Uptime() {
    return 0;
}

uint32_t Thread::Runtime() {
    return Runtime((uint32_t)currentThread);
}

uint32_t Thread::Runtime(uint32_t thread) {
    struct TCB *t = (struct TCB *)thread;
    return t->runtime;
}

