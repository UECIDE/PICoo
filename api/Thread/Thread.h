#ifndef _THREAD_H
#define _THREAD_H

#include <PICoo.h>

#define CORE_TICK_RATE (F_CPU / 2 / (1000/THREAD_MS))

typedef void (*threadFunction)();


struct TCB {
    uint32_t sp;            // Stored stack pointer
    uint32_t state;         // Current thread state
    uint32_t state_data;    // Data associated with state (wake time, mutex address, etc)
    uint32_t stack_head;    // Location in RAM of the top of the stack
    uint32_t stack_size;    // Amount of memory allocated to the stack
    uint32_t runtime;
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

        static uint32_t Create(void (*entry)(), uint32_t param = 0, uint32_t stacksize = 2048);

        static void Sleep(uint32_t ms);
        static void Hibernate();
        static void __attribute__((nomips16)) Start();
        static uint32_t Uptime();
        static uint32_t Runtime();
        static uint32_t Runtime(uint32_t thread);

        static uint32_t FillStack(threadFunction func, uint32_t sp, uint32_t param);


    //    static void __attribute__((interrupt(),nomips16)) Scheduler();

};

#endif
