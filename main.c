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
#include "off1.pio.h"
#include "off2.pio.h"
#include "spe1.pio.h"
#include "spe2.pio.h"
#include "src/pico_servo.h"
#define SERVO_PIN 5
#define estop 6


void estop_callback(void) {
    // Stop the motor
    servo_set_position(SERVO_PIN, 90);
    // Stop the PIO state machines
    pio_sm_set_enabled(pio0, 0, false);
    pio_sm_set_enabled(pio0, 1, false);
    pio_sm_set_enabled(pio0, 2, false);
    pio_sm_set_enabled(pio0, 3, false);
    // Stop the program
    while (true) {
        tight_loop_contents();
    }
}




int main() {
    // Initialize variables
    uint32_t off1, off2, spe1, spe2;
    uint32_t off1_last, off2_last, spe1_last, spe2_last;
    uint32_t offset, speeddiff;
    int curpwm = 90;
    // Initialize interrupts
    gpio_set_irq_enabled_with_callback (estop, GPIO_IRQ_EDGE_FALL, true, &estop_callback);
    // Initialize PWM
    servo_enable(SERVO_PIN);
    curpwm = 90;// put motor throttle to neutral position
    servo_set_position(SERVO_PIN, curpwm); 

    // Initialize PIO
    PIO pio = pio0;
    // Get first free state machine in PIO 0
    uint sm1 = pio_claim_unused_sm(pio, true);
    uint sm2 = pio_claim_unused_sm(pio, true);
    uint sm3 = pio_claim_unused_sm(pio, true);
    uint sm4 = pio_claim_unused_sm(pio, true);
    // Add PIO program to PIO instruction memory. SDK will find location and
    // return with the memory offset of the program.
    uint offset1 = pio_add_program(pio, &off1_program);
    uint offset2 = pio_add_program(pio, &off2_program);
    uint offset3 = pio_add_program(pio, &spe1_program);
    uint offset4 = pio_add_program(pio, &spe2_program);


    // Initialize the program using the helper function in our .pio file
    off1_program_init(pio, sm1, offset1, 1);
    off2_program_init(pio, sm2, offset2, 1);
    spe1_program_init(pio, sm3, offset3, 1);
    spe2_program_init(pio, sm4, offset4, 1);

    // Start running our PIO program in the state machine
    pio_sm_set_enabled(pio, sm1, true);
    pio_sm_set_enabled(pio, sm2, true);
    pio_sm_set_enabled(pio, sm3, true);
    pio_sm_set_enabled(pio, sm4, true);
    

    //match offset( min(off1,off2) smaller better) and speed (abs(spe1-spe2) smaller better)
    while (true) {
        //get the current value form the pio state machines
        off1 = pio_sm_get(pio0, sm1);
        off2 = pio_sm_get(pio0, sm2);
        spe1 = pio_sm_get(pio0, sm3);
        spe2 = pio_sm_get(pio0, sm4);
        //clear the fifo
        pio_sm_clear_fifos (pio, sm1);
        pio_sm_clear_fifos (pio, sm2);
        pio_sm_clear_fifos (pio, sm3);
        pio_sm_clear_fifos (pio, sm4);
        //callculate motor power adjustment
        speeddiff = abs(spe1-spe2);
        offset = MIN(off1,off2);
        //adjust the motor power
        if (abs(spe1_last-spe1) < 5 && abs(spe2_last-spe2) < 5 )
        {
            if (speeddiff > 10){
                if (spe1 > spe2){
                    curpwm = curpwm - 5;
                }else{  
                    curpwm = curpwm + 5;
                }
                servo_set_position(SERVO_PIN, curpwm);
            }else
            {
                if (offset > 10){
                    if (off1 > off2){
                        servo_set_position(SERVO_PIN, curpwm - 5);
                    }else{
                        servo_set_position(SERVO_PIN, curpwm + 5);
                    }
                    sleep_ms(10);
                    servo_set_position(SERVO_PIN, curpwm );

                }

            }
        
        }
    

        //store the last value
        off1_last = off1;
        off2_last = off2;
        spe1_last = spe1;
        spe2_last = spe2;

    }
}