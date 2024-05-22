/*
openocd -f interface/raspberrypi-swd.cfg -f target/rp2040.cfg -c "program pico-sync-motor-dev.elf verify reset exit"
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "in1.pio.h"





int main() {

    static const uint input1 = 25;
    static const uint input2 = 26;


    // Choose PIO instance (0 or 1)
    PIO pio = pio0;

    // Get first free state machine in PIO 0
    uint sm = pio_claim_unused_sm(pio, true);

    // Add PIO program to PIO instruction memory. SDK will find location and
    // return with the memory offset of the program.
    uint offset = pio_add_program(pio, &in1_program);

    // Initialize the program using the helper function in our .pio file
    in1_program_init(pio, sm, offset, input1, 1);

    // Start running our PIO program in the state machine
    pio_sm_set_enabled(pio, sm, true);

    // Do nothing
    while (true) {
        sleep_ms(1000);
    }
}