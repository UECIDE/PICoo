#include <p32xxxx.h>
#include <Tuning.h>

    .set mips32r2
    .set nomips16
    .text
    .align 4

    .set noreorder
    .set noat

    .global ISRWrapper

    .ent ISRWrapper

ISRWrapper:
    // Clear the counter and reset the compare
    //di      zero

    // If currentThread is null then don't do any storing of registers.
    la      k0, currentThread
    lw      k0, 0(k0)
    beq     k0, zero, noThread
    nop

    mfc0    k0, $9  // Load the current tick value
    la      k1, currentThread
    lw      k1, 0(k1)
    lw      k1, 132(k1)
    addu    k0, k1, k0
    la      k1, currentThread
    lw      k1, 0(k1)
    sw      k0, 132(k1)

    // We can now zero the counter again ready to run and time the ISR
    mtc0    zero, $9

    la      k0, currentThread
    lw      k0, 0(k0)
    sw      sp, 0(k0)

    // Store EPC and Status into TCB 
    mfc0    k1, $14
    sw      k1, 4(k0)
    mfc0    k1, $12
    sw      k1, 8(k0)
    mfc0    t0, $13

    // Store all other registers
    sw      ra, 12(k0)
    sw      s8, 16(k0)
    sw      gp, 20(k0)
    sw      t9, 24(k0)
    sw      t8, 28(k0)
    sw      s7, 32(k0)
    sw      s6, 36(k0)
    sw      s5, 40(k0)
    sw      s4, 44(k0)
    sw      s3, 48(k0)
    sw      s2, 52(k0)
    sw      s1, 56(k0)
    sw      s0, 60(k0)
    sw      t7, 64(k0)
    sw      t6, 68(k0)
    sw      t5, 72(k0)
    sw      t4, 76(k0)
    sw      t3, 80(k0)
    sw      t2, 84(k0)
    sw      t1, 88(k0)
    sw      t0, 92(k0)
    sw      a3, 96(k0)
    sw      a2, 100(k0)
    sw      a1, 104(k0)
    sw      a0, 108(k0)
    sw      v1, 112(k0)
    sw      v0, 116(k0)
    sw      $1, 120(k0)
    mfhi    t0
    mflo    t1
    sw      t0, 124(k0)
    sw      t1, 128(k0)

    j       gotThread
noThread:
    mtc0    zero, $9
gotThread:

    // Load the ISR context
    la      k1, ISRThread
    lw      k0, 0(k1)
    lw      sp, 0(k0)

    lw      t0, 128(k0)
    lw      t1, 124(k0)
    mthi    t0
    mtlo    t1

    lw      ra, 12(k0)
    lw      s8, 16(k0)
    lw      gp, 20(k0)
    lw      t9, 24(k0)
    lw      t8, 28(k0)
    lw      s7, 32(k0)
    lw      s6, 36(k0)
    lw      s5, 40(k0)
    lw      s4, 44(k0)
    lw      s3, 48(k0)
    lw      s2, 52(k0)
    lw      s1, 56(k0)
    lw      s0, 60(k0)
    lw      t7, 64(k0)
    lw      t6, 68(k0)
    lw      t5, 72(k0)
    lw      t4, 76(k0)
    lw      t3, 80(k0)
    lw      t2, 84(k0)
    lw      t1, 88(k0)
    lw      t0, 92(k0)
    lw      a3, 96(k0)
    lw      a2, 100(k0)
    lw      a1, 104(k0)
    lw      a0, 108(k0)
    lw      v1, 112(k0)
    lw      v0, 116(k0)
    lw      $1, 120(k0)

    jal     selectNextThread
//    jal      processInterrupt
    nop;

    // Store the ISR context
    la      k1, ISRThread
    lw      k0, 0(k1)

    sw      sp, 0(k0)

    // Store all other registers
    sw      ra, 12(k0)
    sw      s8, 16(k0)
    sw      gp, 20(k0)
    sw      t9, 24(k0)
    sw      t8, 28(k0)
    sw      s7, 32(k0)
    sw      s6, 36(k0)
    sw      s5, 40(k0)
    sw      s4, 44(k0)
    sw      s3, 48(k0)
    sw      s2, 52(k0)
    sw      s1, 56(k0)
    sw      s0, 60(k0)
    sw      t7, 64(k0)
    sw      t6, 68(k0)
    sw      t5, 72(k0)
    sw      t4, 76(k0)
    sw      t3, 80(k0)
    sw      t2, 84(k0)
    sw      t1, 88(k0)
    sw      t0, 92(k0)
    sw      a3, 96(k0)
    sw      a2, 100(k0)
    sw      a1, 104(k0)
    sw      a0, 108(k0)
    sw      v1, 112(k0)
    sw      v0, 116(k0)
    sw      $1, 120(k0)
    mfhi    t0
    mflo    t1
    sw      t0, 124(k0)
    sw      t1, 128(k0)

    // Restore all the data for this thread

restoreThreadContext:

    la      k1, currentThread
    lw      k0, 0(k1)
    lw      sp, 0(k0)

    lw      t0, 128(k0)
    lw      t1, 124(k0)
    mthi    t0
    mtlo    t1

    lw      ra, 12(k0)
    lw      s8, 16(k0)
    lw      gp, 20(k0)
    lw      t9, 24(k0)
    lw      t8, 28(k0)
    lw      s7, 32(k0)
    lw      s6, 36(k0)
    lw      s5, 40(k0)
    lw      s4, 44(k0)
    lw      s3, 48(k0)
    lw      s2, 52(k0)
    lw      s1, 56(k0)
    lw      s0, 60(k0)
    lw      t7, 64(k0)
    lw      t6, 68(k0)
    lw      t5, 72(k0)
    lw      t4, 76(k0)
    lw      t3, 80(k0)
    lw      t2, 84(k0)
    lw      t1, 88(k0)
    lw      t0, 92(k0)
    lw      a3, 96(k0)
    lw      a2, 100(k0)
    lw      a1, 104(k0)
    lw      a0, 108(k0)
    lw      v1, 112(k0)
    lw      v0, 116(k0)
    lw      $1, 120(k0)

    lw      k1, 4(k0)
    mtc0    k1, $14
    lw      k0, 8(k0)
    mtc0    k0, $12

    // Calculate how much time (in us) has elapsed in the ISR
    // This is tricky, as we only really have $k0 and $k1 available
//    la      k1, _ticks_per_us
//    lw      k1, 0(k1)
    mfc0    k0, $9  // Load the current tick value
//    divu    k0, k1 // Calculate the number of us the ISR took
//    mfhi    k0

    la      k1, ISRThread
    lw      k1, 0(k1)
    lw      k1, 132(k1)
    addu    k0, k1, k0
    la      k1, ISRThread
    lw      k1, 0(k1)
    sw      k0, 132(k1)

    // Clear the counter and reset the compare
    mtc0    zero, $9
    la      k0, _core_tick
    lw      k1, 0(k0)
    mtc0    k1, $11

    //ei      zero

    eret

    .end ISRWrapper

