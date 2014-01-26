#include <p32xxxx.h>
#include <Tuning.h>

    .set mips32r2
    .set nomips16
    .text
    .align 4

//    .extern currentTask, 4

    .equ context_size, 128
    .equ tick_speed, CORE_TICK_RATE

    .set noreorder
    .set noat

    .global ThreadScheduler
    .global restoreThreadContext
    .ent ThreadScheduler
    
ThreadScheduler:
    rdpgpr  sp, sp

    // If currentThread is null then don't do any storing of registers.
    la      t0, currentThread
    lw      t1, 0(t0)
    beq     t1, zero, noThread
    nop

    // Store EPC and Status into stack
    mfc0    k1, $14
    addiu   sp, sp, -context_size
    sw      k1, 124(sp)
    mfc0    k1, $12
    mfc0    k0, $13

    sw      k1, 120(sp)

    /* enable nested interrupts */
    ins     k1, zero, 1, 15 /* clear IPL, UM, ERL, EXL */
    ext     k0, k0, 10, 6   /* extract RIPL */
    ins     k1, k0, 10, 6   /* set current IPL */
    mtc0    k1, $12         /* c0_status */

    // Store all other registers
    rdpgpr  ra, ra
    sw      ra, 116(sp)
    rdpgpr  s8, s8
    sw      s8, 112(sp)
    rdpgpr  gp, gp
    sw      gp, 108(sp)
    rdpgpr  t9, t9
    sw      t9, 104(sp)
    rdpgpr  t8, t8
    sw      t8, 100(sp)
    rdpgpr  s7, s7
    sw      s7, 96(sp)
    rdpgpr  s6, s6
    sw      s6, 92(sp)
    rdpgpr  s5, s5
    sw      s5, 88(sp)
    rdpgpr  s4, s4
    sw      s4, 84(sp)
    rdpgpr  s3, s3
    sw      s3, 80(sp)
    rdpgpr  s2, s2
    sw      s2, 76(sp)
    rdpgpr  s1, s1
    sw      s1, 72(sp)
    rdpgpr  s0, s0
    sw      s0, 68(sp)
    rdpgpr  t7, t7
    sw      t7, 64(sp)
    rdpgpr  t6, t6
    sw      t6, 60(sp)
    rdpgpr  t5, t5
    sw      t5, 56(sp)
    rdpgpr  t4, t4
    sw      t4, 52(sp)
    rdpgpr  t3, t3
    sw      t3, 48(sp)
    rdpgpr  t2, t2
    sw      t2, 44(sp)
    rdpgpr  t1, t1
    sw      t1, 40(sp)
    rdpgpr  t0, t0
    sw      t0, 36(sp)
    rdpgpr  a3, a3
    sw      a3, 32(sp)
    rdpgpr  a2, a2
    sw      a2, 28(sp)
    rdpgpr  a1, a1
    sw      a1, 24(sp)
    rdpgpr  a0, a0
    sw      a0, 20(sp)
    rdpgpr  v1, v1
    sw      v1, 16(sp)
    rdpgpr  v0, v0
    sw      v0, 12(sp)
    rdpgpr  $1, $1
    sw      $1, 8(sp)
    mfhi    t0
    mflo    t1
    sw      t0, 4(sp)
    sw      t1, 0(sp)

    la      t0, currentThread
    lw      t1, 0(t0)
    sw      sp, 0(t1)

noThread:

    di      t2
    la      k0, intStatus
    sw      t2, 0(k0)


    // Select the next task to work with
    jal      selectNextThread
    nop;

    mfc0    k0, $13
    ins     k0, zero, 8, 1
    mtc0    k0, $13

    lui     k0, %hi(IFS0CLR)
    ori     k1, zero, 1
    sw      k1, %lo(IFS0CLR)(k0)

    la      k0, intStatus
    lw      t2, 0(k0)
    mtc0    t2, $12

    // Clear the counter and reset the compare
    mtc0    zero, $9
    lui     k1, %hi(tick_speed)
    addiu   k1, k1, %lo(tick_speed)
    mtc0    k1, $11


    // Restore all the data for this thread

restoreThreadContext:

    la t0, currentThread
    lw t1, 0(t0)
    lw sp, 0(t1)

    lw      t0, 4(sp)
    lw      t1, 0(sp)
    mthi    t0
    mtlo    t1

    lw      ra, 116(sp)
    wrpgpr  ra, ra
    lw      s8, 112(sp)
    wrpgpr  s8, s8
    lw      gp, 108(sp)
    wrpgpr  gp, gp
    lw      t9, 104(sp)
    wrpgpr  t9, t9
    lw      t8, 100(sp)
    wrpgpr  t8, t8
    lw      s7, 96(sp)
    wrpgpr  s7, s7
    lw      s6, 92(sp)
    wrpgpr  s6, s6
    lw      s5, 88(sp)
    wrpgpr  s5, s5
    lw      s4, 84(sp)
    wrpgpr  s4, s4
    lw      s3, 80(sp)
    wrpgpr  s3, s3
    lw      s2, 76(sp)
    wrpgpr  s2, s2
    lw      s1, 72(sp)
    wrpgpr  s1, s1
    lw      s0, 68(sp)
    wrpgpr  s0, s0
    lw      t7, 64(sp)
    wrpgpr  t7, t7
    lw      t6, 60(sp)
    wrpgpr  t6, t6
    lw      t5, 56(sp)
    wrpgpr  t5, t5
    lw      t4, 52(sp)
    wrpgpr  t4, t4
    lw      t3, 48(sp)
    wrpgpr  t3, t3
    lw      t2, 44(sp)
    wrpgpr  t2, t2
    lw      t1, 40(sp)
    wrpgpr  t1, t1
    lw      t0, 36(sp)
    wrpgpr  t0, t0
    lw      a3, 32(sp)
    wrpgpr  a3, a3
    lw      a2, 28(sp)
    wrpgpr  a2, a2
    lw      a1, 24(sp)
    wrpgpr  a1, a1
    lw      a0, 20(sp)
    wrpgpr  a0, a0
    lw      v1, 16(sp)
    wrpgpr  v1, v1
    lw      v0, 12(sp)
    wrpgpr  v0, v0
    lw      $1, 8(sp)
    wrpgpr  $1, $1

    di zero
    ehb

    lw k0, 120(sp)
    lw k1, 124(sp)
    mtc0 k1, $14
    addiu sp, sp, context_size
    mtc0 k0, $12
    la t0, spOfCurrentThread
    sw sp, 0(t0)

    wrpgpr  sp, sp

    eret

    .end ThreadScheduler

    .bss
intStatus: .space 4
