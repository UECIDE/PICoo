#ifndef _TUNING_H
#define _TUNING_H

// This is how often the core timer interrupts.  The core timer
// switches the thread contexts, and also updates the millisecond
// counters.  Increasing this will allow each thread more time to
// execute at the expense of multi tasking responsivenetss and
// granularity of the millisecond counters.

#ifndef THREAD_MS
# define THREAD_MS 1
#endif

#endif
