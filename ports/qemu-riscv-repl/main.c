/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Original code:
 * Copyright (c) 2023 Alessandro Gatti
 *
 * Modifications:
 * Copyright (c) 2024 Chuck Benedict
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/builtin.h"
#include "py/compile.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/stackctrl.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "uart.h"

// Allocate memory for the MicroPython GC heap.
static char heap[4096];

int main(int argc, char **argv) {
    // Initialise the MicroPython runtime.
    mp_stack_ctrl_init();
    gc_init(heap, heap + sizeof(heap));
    mp_init();

    // Start a normal REPL; will exit when ctrl-D is entered on a blank line.
    pyexec_friendly_repl();

    // Deinitialise the runtime.
    gc_sweep_all();
    mp_deinit();
    return 0;
}

void exit(int status) {
    uint32_t semihosting_arguments[2] = { 0 };

    // Exit via QEMU's RISC-V semihosting support.
    __asm volatile (
        ".option push            \n" // Transient options
        ".option norvc           \n" // Do not emit compressed instructions
        ".align 4                \n" // 16 bytes alignment
        "mv     a1, %0           \n" // Load buffer
        "li     t0, 0x20026      \n" // ADP_Stopped_ApplicationExit
        "sw     t0, 0(a1)        \n" // ADP_Stopped_ApplicationExit
        "sw     %1, 4(a1)        \n" // Exit code
        "addi   a0, zero, 0x20   \n" // TARGET_SYS_EXIT_EXTENDED
        "slli   zero, zero, 0x1F \n" // Entry NOP
        "ebreak                  \n" // Give control to the debugger
        "srai   zero, zero, 7    \n" // Semihosting call
        ".option pop             \n" // Restore previous options set
        :
        : "r" (semihosting_arguments), "r" (status)
        : "memory"
        );

    // Should never reach here.
    for (;;) {
    }
}

void _entry_point(void) {
    // Enable UART
    uart_init();
    // Now that we have a basic system up and running we can call main
    main(0, 0);
		// Ask host to exit
		exit(0);
}

// Handle uncaught exceptions (should never be reached in a correct C implementation).
void nlr_jump_fail(void *val) {
    for (;;) {
    }
}

// Do a garbage collection cycle.
void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}

// There is no filesystem so stat'ing returns nothing.
mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

// There is no filesystem so opening a file raises an exception.
mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_raise_OSError(MP_ENOENT);
}