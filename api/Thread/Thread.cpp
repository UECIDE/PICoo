#include <PICoo.h>
#include <stdlib.h>

struct TCB *ThreadList = NULL;

uint8_t __attribute__((aligned(4))) StackData[STACK_SIZE];

struct TCB *currentThread = NULL;

volatile uint32_t _MillisecondCounter = 0;

uint32_t Thread::Create(void (*entry)(), uint32_t stacksize) {
    
    if (ThreadList == NULL) { // This is our first ever thread
        struct TCB *newTCB = (struct TCB *)malloc(sizeof(struct TCB));
        newTCB->stack_head = (uint32_t)&StackData[STACK_SIZE-1];
        newTCB->sp = newTCB->stack_head;
        newTCB->stack_size = stacksize;

        newTCB->context.ra = (uint32_t)entry;
        newTCB->context.r1 = 0;     // $at
        newTCB->context.r2 = 0;     // $v0
        newTCB->context.r3 = 0;     // $v1
        newTCB->context.r4 = 0;     // $a0  < If we ever support thread
        newTCB->context.r5 = 0;     // $a1  < args then they will go in
        newTCB->context.r6 = 0;     // $a2  < these 4 registers $a0-$a4
        newTCB->context.r7 = 0;     // $a3  <--------------------------
        newTCB->context.r8 = 0;     // $t0
        newTCB->context.r9 = 0;     // $t1
        newTCB->context.r10 = 0;    // $t2
        newTCB->context.r11 = 0;    // $t3
        newTCB->context.r12 = 0;    // $t4
        newTCB->context.r13 = 0;    // $t5
        newTCB->context.r14 = 0;    // $t6
        newTCB->context.r15 = 0;    // $t7
        newTCB->context.r16 = 0;    // $s0
        newTCB->context.r17 = 0;    // $s1
        newTCB->context.r18 = 0;    // $s2
        newTCB->context.r19 = 0;    // $s3  
        newTCB->context.r20 = 0;    // $s4
        newTCB->context.r21 = 0;    // $s5
        newTCB->context.r22 = 0;    // $s6
        newTCB->context.r23 = 0;    // $s7
        newTCB->context.r24 = 0;    // $t8
        newTCB->context.r25 = 0;    // $t9
        newTCB->context.r26 = 0;    // $k0
        newTCB->context.r27 = 0;    // $k1
        newTCB->context.r28 = 0;    // $gp
        newTCB->context.r29 = newTCB->sp;    // $sp
        newTCB->context.r30 = 0;    // $s8
        newTCB->context.r31 = (uint32_t)entry;    // $ra
        newTCB->context.hi = 0;     // MUL High
        newTCB->context.lo = 0;     // MUL Low
        newTCB->context.c0status = 0;

        newTCB->state_data = 0;
        newTCB->state = RUN;
        newTCB->next = NULL;
        ThreadList = newTCB;
        return (uint32_t)newTCB;
    }
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
    Interrupt::SetVector(_CORE_TIMER_VECTOR, Scheduler);
    Interrupt::EnableIRQ(_CORE_TIMER_IRQ);
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

void __attribute__((interrupt(),nomips16)) Thread::Scheduler() {
    asm volatile("la $k1, currentThread");
    asm volatile("lw $k0, 0($k1)");
    asm volatile("sw $sp, 0($k0)");
    asm volatile("mfc0 $k1, $14");
    asm volatile("sw $k1, 24($k0)");
    asm volatile("mfc0 $k1, $12");
    asm volatile("sw $k1, 28($k0)");
    asm volatile("sw $31, 32($k0)");
    asm volatile("sw $30, 36($k0)");
    asm volatile("sw $29, 40($k0)");
    asm volatile("sw $28, 44($k0)");
    asm volatile("sw $27, 48($k0)");
    asm volatile("sw $26, 52($k0)");
    asm volatile("sw $25, 56($k0)");
    asm volatile("sw $24, 60($k0)");
    asm volatile("sw $23, 64($k0)");
    asm volatile("sw $22, 68($k0)");
    asm volatile("sw $21, 72($k0)");
    asm volatile("sw $20, 76($k0)");
    asm volatile("sw $19, 80($k0)");
    asm volatile("sw $18, 84($k0)");
    asm volatile("sw $17, 88($k0)");
    asm volatile("sw $16, 92($k0)");
    asm volatile("sw $15, 96($k0)");
    asm volatile("sw $14, 100($k0)");
    asm volatile("sw $13, 104($k0)");
    asm volatile("sw $12, 108($k0)");
    asm volatile("sw $11, 112($k0)");
    asm volatile("sw $10, 116($k0)");
    asm volatile("sw $9, 120($k0)");
    asm volatile("sw $8, 124($k0)");
    asm volatile("sw $7, 128($k0)");
    asm volatile("sw $6, 132($k0)");
    asm volatile("sw $5, 136($k0)");
    asm volatile("sw $4, 140($k0)");
    asm volatile("sw $3, 144($k0)");
    asm volatile("sw $2, 148($k0)");
    asm volatile("sw $1, 152($k0)");
    asm volatile("mfhi $t0");
    asm volatile("mflo $t1");
    asm volatile("sw $t0, 156($k0)");
    asm volatile("sw $t1, 160($k0)");
     
    _MillisecondCounter++;

    do {
        currentThread = currentThread->next;
        if (currentThread == NULL) {
            currentThread = ThreadList;
        }

        if (currentThread->state != RUN) {
            if (currentThread->state == SLEEP) {
                if (_MillisecondCounter == currentThread->state_data) {
                    currentThread->state == RUN;
                }
            } else if (currentThread->state == MUTEX) {
                volatile uint32_t *mutex = (uint32_t *)(currentThread->state_data);
                if (*mutex == 0) {
                    *mutex = 1;
                    currentThread->state = RUN;
                }
            }
        }
    } while(currentThread->state != RUN);

    currentThread->runtime++;

    asm volatile("la $k1, currentThread");
    asm volatile("lw $k0, 0($k1)");

    asm volatile("lw $t1, 160($k0)");
    asm volatile("lw $t0, 156($k0)");
    asm volatile("mtlo $t1");
    asm volatile("mthi $t0");
    asm volatile("lw $1, 152($k0)");
    asm volatile("lw $2, 148($k0)");
    asm volatile("lw $3, 144($k0)");
    asm volatile("lw $4, 140($k0)");
    asm volatile("lw $5, 136($k0)");
    asm volatile("lw $6, 132($k0)");
    asm volatile("lw $7, 128($k0)");
    asm volatile("lw $8, 124($k0)");
    asm volatile("lw $9, 120($k0)");
    asm volatile("lw $10, 116($k0)");
    asm volatile("lw $11, 112($k0)");
    asm volatile("lw $12, 108($k0)");
    asm volatile("lw $13, 104($k0)");
    asm volatile("lw $14, 100($k0)");
    asm volatile("lw $15, 96($k0)");
    asm volatile("lw $16, 92($k0)");
    asm volatile("lw $17, 88($k0)");
    asm volatile("lw $18, 84($k0)");
    asm volatile("lw $19, 80($k0)");
    asm volatile("lw $20, 76($k0)");
    asm volatile("lw $21, 72($k0)");
    asm volatile("lw $22, 68($k0)");
    asm volatile("lw $23, 64($k0)");
    asm volatile("lw $24, 60($k0)");
    asm volatile("lw $25, 56($k0)");
    asm volatile("lw $26, 52($k0)");
    asm volatile("lw $27, 48($k0)");
    asm volatile("lw $28, 44($k0)");
    asm volatile("lw $29, 40($k0)");
    asm volatile("lw $30, 36($k0)");
    asm volatile("lw $31, 32($k0)");

    asm volatile("lw $k1, 28($k0)");
    asm volatile("mtc0 $k1, $12");
    asm volatile("lw $k1, 24($k0)");
    asm volatile("mtc0 $k1, $14");

    asm volatile("lw $sp, 0($k0)");
}

