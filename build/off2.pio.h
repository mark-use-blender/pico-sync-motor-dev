// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ---- //
// off2 //
// ---- //

#define off2_wrap_target 0
#define off2_wrap 8

static const uint16_t off2_program_instructions[] = {
            //     .wrap_target
    0xa02b, //  0: mov    x, !null                   
    0x2002, //  1: wait   0 gpio, 2                  
    0x2003, //  2: wait   0 gpio, 3                  
    0x2082, //  3: wait   1 gpio, 2                  
    0x00c6, //  4: jmp    pin, 6                     
    0x0044, //  5: jmp    x--, 4                     
    0xa029, //  6: mov    x, !x                      
    0x4020, //  7: in     x, 32                      
    0x8000, //  8: push   noblock                    
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program off2_program = {
    .instructions = off2_program_instructions,
    .length = 9,
    .origin = -1,
};

static inline pio_sm_config off2_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + off2_wrap_target, offset + off2_wrap);
    return c;
}

// Helper function (for use in C program) to initialize this PIO program
void off2_program_init(PIO pio, uint sm, uint offset, float div) {
    // Sets up state machine and wrap target. This function is automatically
    pio_sm_config c = off2_program_get_default_config(offset);
    sm_config_set_fifo_join (&c, 2);
    sm_config_set_jmp_pin(&c, 3);
    // Set the clock divider for the state machine
    sm_config_set_clkdiv(&c, div);
    // Load configuration and jump to start of the program
    pio_sm_init(pio, sm, offset, &c);
}

#endif

