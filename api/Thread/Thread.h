#ifndef _THREAD_H
#define _THREAD_H

#include <PICoo.h>

typedef void (*threadFunction)(uint32_t);

typedef struct TCB * thread;
typedef volatile uint32_t mutex;
typedef volatile uint32_t semaphore;

extern const uint32_t _core_tick;

struct threadContext {
    uint32_t epc;
    uint32_t status;
    uint32_t ra;
    uint32_t fp;
    uint32_t gp;
    uint32_t t9;
    uint32_t t8;
    uint32_t s7;
    uint32_t s6;
    uint32_t s5;
    uint32_t s4;
    uint32_t s3;
    uint32_t s2;
    uint32_t s1;
    uint32_t s0;
    uint32_t t7;
    uint32_t t6;
    uint32_t t5;
    uint32_t t4;
    uint32_t t3;
    uint32_t t2;
    uint32_t t1;
    uint32_t t0;
    uint32_t a3;
    uint32_t a2;
    uint32_t a1;
    uint32_t a0;
    uint32_t v1;
    uint32_t v0;
    uint32_t at;
    uint32_t hi;
    uint32_t lo;
};



struct TCB {
    uint32_t *sp;            // Stored stack pointer
    struct threadContext context; // Stored context
    uint32_t runtime;
    uint32_t state;         // Current thread state
    uint32_t state_data;    // Data associated with state (wake time, mutex address, etc)
    uint32_t state_data_extra;
    uint32_t *stack_head;    // Location in RAM of the top of the stack
    uint32_t stack_size;    // Amount of memory allocated to the stack
    uint32_t runtime_us;
    uint32_t runtime_ms;
    threadFunction entry;
    const char *name;
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
        static const uint32_t SEMWAIT = 5;

        static thread Create(const char *, threadFunction entry, uint32_t param = 0, uint32_t stacksize = DEFAULT_STACK_BLOCK);

        static void Sleep(uint32_t ms);
        static void USleep(uint32_t us);
        static void Hibernate();
        static void __attribute__((nomips16)) Start();
        static uint32_t Runtime();
        static uint32_t Runtime(thread t);
        static uint32_t Milliseconds();
        static uint32_t Microseconds();
        static void Terminate();
        static void Wake(thread t);

        static void FillContext(threadFunction func, thread t, uint32_t param);

        static void Lock(mutex& m);
        static void Unlock(mutex& m);

        static void Signal(semaphore& s);
        static void Wait(semaphore& s);

        static void SelectNextThread();

};

#endif
