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
#include "hardware/irq.h"
#include "off1.pio.h"
#include "off2.pio.h"
#include "spe1.pio.h"
#include "spe2.pio.h"
#include "src/pico_servo.h"
#define SERVO_PIN 5
#define motor_pwr 4
#define estop_pin 6
#define ARM_PIN 7
#define LED_PIN 25
#define reset_pos 90
#define min_tror_off 10
#define sync_stat_red_led 13
#define sync_stat_green_led 14






void estop_pin_callback(uint gpio, uint32_t events) {
    // Stop the motor
    gpio_acknowledge_irq(gpio, events);
    gpio_put(motor_pwr, 0);
    servo_set_position(SERVO_PIN, reset_pos);
    while (true)
    {
        for (int i = 0; i < 3; i++) {
            for (int i = 0; i < 3000000; i++) {
                gpio_put(LED_PIN, 1);
            }
            for (int i = 0; i < 1500000; i++) {
                gpio_put(LED_PIN, 0);
            }
        }
        for (int i = 0; i < 3; i++) {
            for (int i = 0; i < 12000000; i++) {
                gpio_put(LED_PIN, 1);
            }
            for (int i = 0; i < 1500000; i++) {
                gpio_put(LED_PIN, 0);
            }
        }
        for (int i = 0; i < 3; i++) {
            for (int i = 0; i < 3000000; i++) {
                gpio_put(LED_PIN, 1);
            }
            for (int i = 0; i < 1500000; i++) {
                gpio_put(LED_PIN, 0);
            }
        }
        for (int i = 0; i < 9000000; i++) 
        {
            gpio_put(LED_PIN, 0);
        }
    }
}




int main() {
    // Initialize variables
    uint32_t off1, off2, spe1, spe2;
    uint32_t off1_last, off2_last, spe1_last, spe2_last;
    uint32_t offset, speeddiff;
    uint32_t offset_last , speeddiff_last;
    int curpwm = 0;
    // Initialize estop pin
    gpio_init(estop_pin);
    gpio_set_dir(estop_pin, GPIO_IN);
    gpio_pull_down(estop_pin);
    // Initialize motor power pin
    gpio_init(motor_pwr);
    gpio_set_dir(motor_pwr, GPIO_OUT);
    gpio_put(motor_pwr, 0);
    
    // Initialize arm pin
    gpio_init(ARM_PIN);
    gpio_set_dir(ARM_PIN, GPIO_IN);
    gpio_pull_down(ARM_PIN);

    // Initialize LED pin
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    // Initialize sync_stat_red_led pin
    gpio_init(sync_stat_red_led);
    gpio_set_dir(sync_stat_red_led, GPIO_OUT);
    gpio_put(sync_stat_red_led, 0);
    // Initialize sync_stat_green_led pin
    gpio_init(sync_stat_green_led);
    gpio_set_dir(sync_stat_green_led, GPIO_OUT);
    gpio_put(sync_stat_green_led, 0);
    // Initialize PWM
    servo_enable(SERVO_PIN);
    curpwm = reset_pos;// put motor throttle to neutral position
    servo_set_position(SERVO_PIN, curpwm); 
    sleep_ms(500);
    // Initialize PIO
    PIO pio00 = pio0;
    PIO pio01 = pio1;
    // Get first free state machine in PIO 
    uint sm1 = pio_claim_unused_sm(pio00, true);
    uint sm2 = pio_claim_unused_sm(pio00, true);
    uint sm3 = pio_claim_unused_sm(pio01, true);
    uint sm4 = pio_claim_unused_sm(pio01, true);
    // Add PIO program to PIO instruction memory. SDK will find location and
    // return with the memory offset of the program.
    uint offset1 = pio_add_program(pio00, &off1_program);
    uint offset2 = pio_add_program(pio00, &off2_program);
    uint offset3 = pio_add_program(pio01, &spe1_program);
    uint offset4 = pio_add_program(pio01, &spe2_program);
    // Initialize the program using the helper function in our .pio file
    off1_program_init(pio00, sm1, offset1, 1);
    off2_program_init(pio00, sm2, offset2, 1);
    spe1_program_init(pio01, sm3, offset3, 1);
    spe2_program_init(pio01, sm4, offset4, 1);
    // Start running our PIO program in the state machine
    pio_sm_set_enabled(pio00, sm1, true);
    pio_sm_set_enabled(pio00, sm2, true);
    pio_sm_set_enabled(pio01, sm3, true);
    pio_sm_set_enabled(pio01, sm4, true);
    gpio_put(LED_PIN, 0);
    //wait for arm cammand
    while (gpio_get(estop_pin) == 0) {
        gpio_put(LED_PIN, 1);
        sleep_ms(200);
        gpio_put(LED_PIN, 0);
        sleep_ms(50);
    }
    gpio_set_irq_enabled_with_callback (estop_pin,GPIO_IRQ_EDGE_FALL , true, &estop_pin_callback);
    gpio_put(LED_PIN, 1);
    while (true)
    {
        gpio_put(motor_pwr, 0);
        off1_last = 0;
        off2_last = 0;
        spe1_last = 0;
        spe2_last = 0;
        offset_last = 0;
        speeddiff_last = 0;
        servo_set_position(SERVO_PIN, reset_pos);
        // Stop the PIO state machines
        while (gpio_get(ARM_PIN) == 0)
        {
            gpio_put(LED_PIN, 1);
            sleep_ms(50);
            gpio_put(LED_PIN, 0);
            sleep_ms(50);
        }
        gpio_put(LED_PIN, 1);
        gpio_put(motor_pwr, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        curpwm = reset_pos+min_tror_off;// put motor throttle to start position
        servo_set_position(SERVO_PIN, curpwm); 
        //match offset( min(off1,off2) smaller better) and speed (abs(spe1-spe2) smaller better)
        while (gpio_get(ARM_PIN) == 1) {
            gpio_put(motor_pwr, 1);
            gpio_put(LED_PIN, 1);
            //get the current value form the pio state machines
            off1 = pio_sm_get(pio00, sm1);
            off2 = pio_sm_get(pio00, sm2);
            spe1 = pio_sm_get(pio01, sm3);
            spe2 = pio_sm_get(pio01, sm4);
            //clear the fifo
            pio_sm_clear_fifos (pio00, sm1);
            pio_sm_clear_fifos (pio00, sm2);
            pio_sm_clear_fifos (pio01, sm3);
            pio_sm_clear_fifos (pio01, sm4);
            //callculate motor power adjustment
            speeddiff = abs(spe1-spe2);
            offset = MIN(off1,off2);
            gpio_put(LED_PIN, 0);
            gpio_put(sync_stat_red_led, 0);
            gpio_put(sync_stat_green_led, 0);
            //adjust the motor power
            if (abs(speeddiff-speeddiff_last) < 50)
            {
                if (speeddiff > 10){
                    gpio_put(sync_stat_red_led, 1);
                    
                    //speed adjustment
                    if (spe1 > spe2){
                        curpwm = curpwm - 1;
                    }else{  
                        curpwm = curpwm + 1;
                    }
                    curpwm = MIN(180,curpwm);
                    curpwm = MAX(reset_pos+min_tror_off,curpwm);
                    servo_set_position(SERVO_PIN, curpwm);
                    
                }
                else if (abs(offset-offset_last) < 50)
                {
                    //offset adjustment
                    if (offset > 10){
                        gpio_put(sync_stat_red_led, 1);
                        gpio_put(sync_stat_green_led, 1);
                        if (off1 > off2){
                            servo_set_position(SERVO_PIN, MAX(reset_pos+min_tror_off ,(curpwm - 1)));
                        }else{
                            servo_set_position(SERVO_PIN, MIN(180,(curpwm + 1)));
                        }
                        sleep_ms(5);
                        servo_set_position(SERVO_PIN, curpwm );
                    }
                    else
                    {
                        gpio_put(sync_stat_green_led, 1);
                    }
                    

                }
            
            }
            //store the last value
            off1_last = off1;
            off2_last = off2;
            spe1_last = spe1;
            spe2_last = spe2;
            offset_last = offset;
            speeddiff_last = speeddiff;
        }
    }
}