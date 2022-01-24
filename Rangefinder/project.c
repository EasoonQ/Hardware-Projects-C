#include "project.h"
#include "encoder.h"
#include "rangefinder.h"
#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include <avr/eeprom.h>

volatile unsigned char buzzer_count = 0;

void init_timer0(){
    //set to CTC mode
    TCCR0B |= (1 << WGM01);
    //enable interrupt
    TIMSK0 |= (1 << OCIE0A);
    //will set prescalar to 256
    OCR0A = 250; // interrupt will be generated every 1/125 s
}

void init_timer1(){
    //set to CTC mode
    TCCR1B |= (1 << WGM12);
    //enbale interrupt
    TIMSK1 |= (1 << OCIE1A);
    //when turn on, will set prescalar to 8
    //max time interval = 400 * 58 = 23200 us = 0.0232s
    //16M * 0.0232 / 8 = 46400
    OCR1A = 46400;
}

void init_timer2(){     //set to Fast PWM mode
    TCCR2B &= ~(1 << WGM22);
    TCCR2A |= ((1 << WGM21) | (1 << WGM20));
    TCCR2A |= (1 << COM2A1);
    TCCR2A &= ~(1 << COM2A0);
    //will set prescalar to 256
    OCR2A = 128;
    DDRB |= (1 << PB3);
}

int main(void){
    //make trigger port PD2 as output
    DDRD |= (1 << PD2);
    
    //make the LED two color segments as output
    DDRB |= (1 << PB3);
    DDRC |= (1 << PC4);
    
    //make buzzer as output
    DDRC |= (1 << PC5);

    //enable pull-up for pressMeasure
    PORTB |= (1 << PB4);
    
    init_timer0();
    init_timer1();
    init_timer2();
    lcd_init();

    //enable interrupt for triggering (PD3)
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT19);
    PORTD |= (1 << PD3); // pull-up
    
    
    //enable interrupt for rotary encoder (PC1,2) and near/far toggle (PC3)
    PCICR |= (1 << PCIE1);
    PCMSK1 |= ((1 << PCINT9) | (1 << PCINT10));
    PORTC |= ((1 << PC1) | (1 << PC2) | (1 << PC3)); // pull-up

    sei();
    
    // check if eeprom stores valid values and load them in
    if (eeprom_read_word((void *) 100) >= 1 && eeprom_read_word((void *) 100) <= 400
        && eeprom_read_word((void *) 200) >= 1 && eeprom_read_word((void *) 200) <= 400){
        near = eeprom_read_word((void *) 100);
        far = eeprom_read_word((void *) 200);
    }
    
    // get the initial state of the rotary encoder
    unsigned char temp = PINC;
    if(temp & (1 << PC1)){
        b = 1;
    }
    else{
        b = 0;
    }
    if(temp & (1 << PC2)){
        a = 1;
    }
    else{
        a = 0;
    }

    if (!b && !a)
    old_state_rot = 0;
    else if (!b && a)
    old_state_rot = 1;
    else if (b && !a)
    old_state_rot = 2;
    else
    old_state_rot = 3;
    
    new_state_rot = old_state_rot;
    
    // Write a spash screen to the LCD
    lcd_writecommand(1);
    lcd_moveto(0,0);
    lcd_stringout("EE109 Project");
    lcd_moveto(1,0);
    lcd_stringout("Eason Qin"); 
    _delay_ms(1000);
    lcd_writecommand(1);

    unsigned long dist = 0;
    char buff1[17];
    char buff2[17];
    
    while (1){
        unsigned char pressMeasure = !(PINB & (1 << PB4));
        unsigned char pressToggle = !(PINC & (1 << PC3));
        if (pressMeasure){
            _delay_ms(200); //debounce
            PORTD |= (1 << PD2);
            _delay_us(10);
            PORTD &= ~(1 << PD2);
        }
        if (pressToggle) {
            _delay_ms(200); //debounce
            if (state_toggle == 0) {
                state_toggle = 1;
            }
            else{
                state_toggle = 0;
            }
        }
        if (changed_rot == 1) { // update eeprom when near/far value change
            changed_rot = 0;
            eeprom_update_word((void *) 100, near);
            eeprom_update_word((void *) 200, far);
        }
        if (abnormal == 1) {
            // blue on, show > 400
            DDRB &= ~(1 << PB3);
            if (state_toggle == 0) {
                snprintf(buff1, 17, "Near=%03d  >400.0", near);
                snprintf(buff2, 17, "Far  %03d", far);
            }
            else if (state_toggle == 1){
                snprintf(buff1, 17, "Near %03d  >400.0", near);
                snprintf(buff2, 17, "Far =%03d", far);
            }
            PORTC |= (1 << PC4);
        }
        else if (complete == 1) {
            DDRB |= (1 << PB3);
            complete = 0;
            dist = (pulse_count * 10) / 116;
            
            if(dist < near * 10){
                // red on, others off, buzzer on
                TCCR2B |= (1 << CS22);
                TCCR2B &= ~((1 << CS21) | (1 << CS20)); //turn on PWM
                OCR2A = 0; // make LED red all the time
                PORTC &= ~(1 << PC4);
                TCCR0B |= (1 << CS22);
                TCCR0B &= ~((1 << CS21) | (1 << CS20));
                buzzer_count = 125;
            }
            else if(dist > far * 10){
                // green on, others off
                TCCR2B |= (1 << CS22);
                TCCR2B &= ~((1 << CS21) | (1 << CS20)); //turn on PWM
                OCR2A = 255; // make LED green all the time
                PORTC &= ~(1 << PC4);
            }
            else{
                // somewhere between red and green
                TCCR2B |= (1 << CS22);
                TCCR2B &= ~((1 << CS21) | (1 << CS20)); //turn on PWM
                OCR2A = (dist - near * 10) * 255 / (far * 10 - near * 10); // calculate value for OCR
                PORTC &= ~(1 << PC4);
            }
        }
        if (abnormal == 0) {
            if (state_toggle == 0) {
                snprintf(buff1, 17, "Near=%03d  %4d.%d", near, (int)(dist/10), (int)(dist%10));
                snprintf(buff2, 17, "Far  %03d", far);
            }
            else if (state_toggle == 1) {
                snprintf(buff1, 17, "Near %03d  %4d.%d", near, (int)(dist/10), (int)(dist%10));
                snprintf(buff2, 17, "Far =%03d", far);
            }
        }
        lcd_moveto(0,0);
        lcd_stringout(buff1);
        lcd_moveto(1,0);
        lcd_stringout(buff2);
    }

    return 0;
}



ISR(TIMER0_COMPA_vect){ // interrupt for buzzer (timer 0)
    if (buzzer_count == 0) {
        TCCR0B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));; // turn off the timer when 1s is completed
    }
    else if (buzzer_count % 2 == 1) {
        PORTC |= (1 << PC5); // turn on the buzzer
        buzzer_count--;
    }
    else{
        PORTC &= ~(1 << PC5); // turn off the buzzer
        buzzer_count--;
    }
}
