// **********************************************************************
// *                    SEGGER Microcontroller GmbH                     *
// *                        The Embedded Experts                        *
// **********************************************************************
// *                                                                    *
// *            (c) 2014 - 2024 SEGGER Microcontroller GmbH             *
// *            (c) 2001 - 2024 Rowley Associates Limited               *
// *                                                                    *
// *           www.segger.com     Support: support@segger.com           *
// *                                                                    *
// **********************************************************************
// *                                                                    *
// * All rights reserved.                                               *
// *                                                                    *
// * Redistribution and use in source and binary forms, with or         *
// * without modification, are permitted provided that the following    *
// * condition is met:                                                  *
// *                                                                    *
// * - Redistributions of source code must retain the above copyright   *
// *   notice, this condition and the following disclaimer.             *
// *                                                                    *
// * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
// * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
// * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
// * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
// * DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
// * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
// * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
// * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
// * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
// * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
// * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
// * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
// * DAMAGE.                                                            *
// *                                                                    *
// **********************************************************************
//
//
//                           Preprocessor Definitions
//                           ------------------------
// APP_ENTRY_POINT
//
//   Defines the application entry point function, if undefined this setting
//   defaults to "main".
//
// INITIALIZE_STACK
//
//   If defined, the contents of the stack will be initialized to a the
//   value 0xCC.
//
// INITIALIZE_SECONDARY_SECTIONS
//
//   If defined, the .data2, .text2, .rodata2 and .bss2 sections will be initialized.
//
// INITIALIZE_TCM_SECTIONS
//
//   If defined, the .data_tcm, .text_tcm, .rodata_tcm and .bss_tcm sections 
//   will be initialized.
//
// INITIALIZE_USER_SECTIONS
//
//   If defined, the function InitializeUserMemorySections will be called prior
//   to entering main in order to allow the user to initialize any user defined
//   memory sections.
//
// FULL_LIBRARY
//
//   If defined then 
//     - argc, argv are setup by the SEGGER_SEMIHOST_GetArgs.
//     - the exit symbol is defined and executes on return from main.
//     - the exit symbol calls destructors, atexit functions and then SEGGER_SEMIHOST_Exit.
//  
//   If not defined then
//     - argc and argv are zero.
//     - the exit symbol is defined, executes on return from main and loops
//
// STACK_CHECK
//
//   If defined will set the v8m msplim/psplim registers to the start of the stacks
//
// RETURN_FROM_CRT0
//
//   If defined, carry out a return to link register on application exit.
//


#ifndef APP_ENTRY_POINT
#define APP_ENTRY_POINT main
#endif

#ifndef ARGSSPACE
#define ARGSSPACE 128
#endif
  .syntax unified

  .global _start
  .extern APP_ENTRY_POINT
  .weak exit
  .weak _Exit

#ifdef INITIALIZE_USER_SECTIONS
  .extern InitializeUserMemorySections
#endif

  .section .init, "ax"
  .code 16
  .balign 2
  .thumb_func

_start:
  /* Set up main stack if size > 0 */
  ldr r1, =__stack_end__
  ldr r0, =__stack_start__
  subs r2, r1, r0
  beq 1f
#ifdef __ARM_EABI__
  movs r2, #0x7
  bics r1, r2
#endif
  mov sp, r1
#ifdef RETURN_FROM_CRT0
  push {lr}
#endif
#ifdef INITIALIZE_STACK
  movs r2, #0xCC
  ldr r0, =__stack_start__
  bl memory_set
#endif
#ifdef STACK_CHECK
  ldr r0, =__stack_start__
  msr msplim, r0
#endif
1:
  /* Set up process stack if size > 0 */
  ldr r1, =__stack_process_end__
  ldr r0, =__stack_process_start__
  subs r2, r1, r0
  beq 1f
#ifdef __ARM_EABI__
  movs r2, #0x7
  bics r1, r2
#endif
  msr psp, r1
  movs r2, #2
  msr control, r2
#ifdef INITIALIZE_STACK
  movs r2, #0xCC
  bl memory_set
#endif
#ifdef STACK_CHECK
  ldr r0, =__stack_process_start__
  msr psplim, r0
#endif
1:

  /* Copy initialized memory sections into RAM (if necessary). */
  ldr r0, =__data_load_start__
  ldr r1, =__data_start__
  ldr r2, =__data_end__
  bl memory_copy
  ldr r0, =__text_load_start__
  ldr r1, =__text_start__
  ldr r2, =__text_end__
  bl memory_copy
  ldr r0, =__fast_load_start__
  ldr r1, =__fast_start__
  ldr r2, =__fast_end__
  bl memory_copy
  ldr r0, =__ctors_load_start__
  ldr r1, =__ctors_start__
  ldr r2, =__ctors_end__
  bl memory_copy
  ldr r0, =__dtors_load_start__
  ldr r1, =__dtors_start__
  ldr r2, =__dtors_end__
  bl memory_copy
  ldr r0, =__rodata_load_start__
  ldr r1, =__rodata_start__
  ldr r2, =__rodata_end__
  bl memory_copy
  ldr r0, =__tdata_load_start__
  ldr r1, =__tdata_start__
  ldr r2, =__tdata_end__
  bl memory_copy
#ifdef INITIALIZE_SECONDARY_SECTIONS
  ldr r0, =__data2_load_start__
  ldr r1, =__data2_start__
  ldr r2, =__data2_end__
  bl memory_copy
  ldr r0, =__text2_load_start__
  ldr r1, =__text2_start__
  ldr r2, =__text2_end__
  bl memory_copy
  ldr r0, =__rodata2_load_start__
  ldr r1, =__rodata2_start__
  ldr r2, =__rodata2_end__
  bl memory_copy
#endif /* #ifdef INITIALIZE_SECONDARY_SECTIONS */
#ifdef INITIALIZE_TCM_SECTIONS
  ldr r0, =__data_tcm_load_start__
  ldr r1, =__data_tcm_start__
  ldr r2, =__data_tcm_end__
  bl memory_copy
  ldr r0, =__text_tcm_load_start__
  ldr r1, =__text_tcm_start__
  ldr r2, =__text_tcm_end__
  bl memory_copy
  ldr r0, =__rodata_tcm_load_start__
  ldr r1, =__rodata_tcm_start__
  ldr r2, =__rodata_tcm_end__
  bl memory_copy
#endif /* #ifdef INITIALIZE_TCM_SECTIONS */

  /* Zero the bss. */
  ldr r0, =__bss_start__
  ldr r1, =__bss_end__
  movs r2, #0
  bl memory_set
  ldr r0, =__tbss_start__
  ldr r1, =__tbss_end__
  movs r2, #0
  bl memory_set
#ifdef INITIALIZE_SECONDARY_SECTIONS
  ldr r0, =__bss2_start__
  ldr r1, =__bss2_end__
  mov r2, #0
  bl memory_set
#endif /* #ifdef INITIALIZE_SECONDARY_SECTIONS */
#ifdef INITIALIZE_TCM_SECTIONS
  ldr r0, =__bss_tcm_start__
  ldr r1, =__bss_tcm_end__
  mov r2, #0
  bl memory_set
#endif /* #ifdef INITIALIZE_TCM_SECTIONS */

#if !defined(__HEAP_SIZE__) || (__HEAP_SIZE__)
  /* Initialize the heap */
  ldr r0, = __heap_start__
  ldr r1, = __heap_end__
  subs r1, r1, r0
#if defined(__SES_ARM)
  bl __SEGGER_RTL_init_heap
#else
  cmp r1, #8
  blt 1f
  movs r2, #0
  str r2, [r0]
  str r1, [r0, #4] 
1:
#endif
#endif

#ifdef INITIALIZE_USER_SECTIONS
  ldr r2, =InitializeUserMemorySections
  blx r2
#endif

  .type start, function
start:

  /* Call constructors */
  ldr r0, =__ctors_start__
  ldr r1, =__ctors_end__
ctor_loop:
  cmp r0, r1
  beq ctor_end
  ldr r2, [r0]
  adds r0, #4
  push {r0-r1}  
  blx r2
  pop {r0-r1}
  b ctor_loop
ctor_end:

  /* Setup initial call frame */
  movs r0, #0
  mov lr, r0
  mov r12, sp

  .type __startup_complete, function
__startup_complete:

  /* Jump to application entry point */
#ifdef FULL_LIBRARY
  movs r0, #ARGSSPACE
  ldr r1, =args
  ldr r2, =SEGGER_SEMIHOST_GetArgs
  blx r2
  ldr r1, =args
#else
  movs r0, #0
  movs r1, #0
#endif
  ldr r2, =APP_ENTRY_POINT
  blx r2

  .thumb_func
exit:
#ifdef FULL_LIBRARY  
  mov r5, r0 // save the exit parameter/return result

  /* Call destructors */
  ldr r0, =__dtors_start__
  ldr r1, =__dtors_end__
dtor_loop:
  cmp r0, r1
  beq dtor_end
  ldr r2, [r0]
  adds r0, #4
  push {r0-r1}
  blx r2
  pop {r0-r1}
  b dtor_loop
dtor_end:

  /* Call atexit functions */
  ldr r2, =__SEGGER_RTL_execute_at_exit_fns
  blx r2

  .type _Exit, function
_Exit:
  /* Call SEGGER_SEMIHOST_Exit with return result/exit parameter */
  mov r0, r5
  ldr r2, =SEGGER_SEMIHOST_Exit
  blx r2
#elif defined(RETURN_FROM_CRT0)
  /* Returned from application entry point */
  pop {r2}
  bx r2
#else
  .type _Exit, function
_Exit:
#endif
  /* Loop forever */
.type exit_loop, function
exit_loop:
  b exit_loop

  .thumb_func
memory_copy:
  cmp r0, r1
  beq 3f
  subs r2, r2, r1
  beq 3f

  /* if either pointer or length is not word aligned then byte copy */
  mov r3, r0
  orrs r3, r1
  orrs r3, r2
  movs r4, #0x3
  tst r3, r4
  bne 2f
  /* word copy */
1:
  ldr r3, [r0]
  adds r0, r0, #4
  str r3, [r1]
  adds r1, r1, #4
  subs r2, r2, #4
  bne 1b
  bx lr
  /* byte copy */
2:
  ldrb r3, [r0]
  adds r0, r0, #1
  strb r3, [r1]
  adds r1, r1, #1
  subs r2, r2, #1
  bne 2b
3:
  bx lr

  .thumb_func
memory_set:
  cmp r0, r1
  beq 1f
  strb r2, [r0]
  adds r0, r0, #1
  b memory_set
1:
  bx lr

  // default C/C++ library helpers

.macro HELPER helper_name
  .section .text.\helper_name, "ax", %progbits
  .balign 2 
  .weak \helper_name
  .thumb_func
\helper_name:
.endm

HELPER __aeabi_read_tp
  ldr r0, =__tbss_start__-8
  bx lr
HELPER abort
#ifdef FULL_LIBRARY
  movs r0, #1
  ldr r2, =SEGGER_SEMIHOST_Exit
  blx r2
#endif
  b .
HELPER __assert
  b .
HELPER __assert_func
  b .
HELPER __aeabi_assert
  b .
HELPER __sync_synchronize
  bx lr
HELPER __semihost
  bkpt 0xab
  bx lr

#ifdef FULL_LIBRARY
  .bss
args:
  .space ARGSSPACE
#endif

  /* Setup attibutes of stack and heap sections so they don't take up room in the elf file */
  .section .stack, "wa", %nobits
  .section .stack_process, "wa", %nobits
  .section .heap, "wa", %nobits

