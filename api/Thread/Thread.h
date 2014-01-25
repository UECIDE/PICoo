#ifndef _THREAD_H
#define _THREAD_H

#include <PICoo.h>

#define STACK_SIZE 16384

#define CORE_TICK_RATE (F_CPU / 2 / (1000/THREAD_MS))

struct ThreadContext {
    uint32_t ra;
    uint32_t c0status;
    uint32_t r31;
    uint32_t r30;
    uint32_t r29;
    uint32_t r28;
    uint32_t r27;
    uint32_t r26;
    uint32_t r25;
    uint32_t r24;
    uint32_t r23;
    uint32_t r22;
    uint32_t r21;
    uint32_t r20;
    uint32_t r19;
    uint32_t r18;
    uint32_t r17;
    uint32_t r16;
    uint32_t r15;
    uint32_t r14;
    uint32_t r13;
    uint32_t r12;
    uint32_t r11;
    uint32_t r10;
    uint32_t r9;
    uint32_t r8;
    uint32_t r7;
    uint32_t r6;
    uint32_t r5;
    uint32_t r4;
    uint32_t r3;
    uint32_t r2;
    uint32_t r1;
    uint32_t hi;
    uint32_t lo;
};

struct TCB {
    uint32_t sp;            // Stored stack pointer
    uint32_t state;         // Current thread state
    uint32_t state_data;    // Data associated with state (wake time, mutex address, etc)
    uint32_t stack_head;    // Location in RAM of the top of the stack
    uint32_t stack_size;    // Amount of memory allocated to the stack
    uint32_t runtime;
    struct ThreadContext context;   // Context storage area
    struct TCB *next;       // Next TCB in list
};

extern struct TCB *ThreadList;

class Thread {
    public:
        static const uint32_t ZOMBIE = 0;
        static const uint32_t RUN = 1;
        static const uint32_t SLEEP = 2;
        static const uint32_t HIBER = 3;
        static const uint32_t MUTEX = 4;

        static uint32_t Create(void (*entry)(), uint32_t stacksize = 2048);

        static void Sleep(uint32_t ms);
        static void Hibernate();
        static void __attribute__((nomips16)) Start();
        static uint32_t Uptime();
        static uint32_t Runtime();
        static uint32_t Runtime(uint32_t thread);

        static void __attribute__((interrupt(),nomips16)) Scheduler();

};

#endif
