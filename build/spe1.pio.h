// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ---- //
// spe1 //
// ---- //

#define spe1_wrap_target 3
#define spe1_wrap 10

static const uint16_t spe1_program_instructions[] = {
    0xa02b, //  0: mov    x, !null                   
    0x2002, //  1: wait   0 gpio, 2                  
    0x2082, //  2: wait   1 gpio, 2                  
            //     .wrap_target
    0xa02b, //  3: mov    x, !null                   
    0x00c7, //  4: jmp    pin, 7                     
    0x00c8, //  5: jmp    pin, 8                     
    0x0045, //  6: jmp    x--, 5                     
    0x0044, //  7: jmp    x--, 4                     
    0xa029, //  8: mov    x, !x                      
    0x4020, //  9: in     x, 32                      
    0x8000, // 10: push   noblock                    
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program spe1_program = {
    .instructions = spe1_program_instructions,
    .length = 11,
    .origin = -1,
};

static inline pio_sm_config spe1_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + spe1_wrap_target, offset + spe1_wrap);
    return c;
}

// Helper function (for use in C program) to initialize this PIO program
void spe1_program_init(PIO pio, uint sm, uint offset, float div) {
    // Sets up state machine and wrap target. This function is automatically
    pio_sm_config c = spe1_program_get_default_config(offset);
    sm_config_set_fifo_join (&c, 2);
    sm_config_set_jmp_pin(&c, 2);
    // Set the clock divider for the state machine
    sm_config_set_clkdiv(&c, div);
    // Load configuration and jump to start of the program
    pio_sm_init(pio, sm, offset, &c);
}

#endif

