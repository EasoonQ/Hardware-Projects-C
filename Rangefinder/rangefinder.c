#include "project.h"
#include "rangefinder.h"

volatile unsigned char start = 0; // measurement started
volatile unsigned char complete = 0; // measurement completed
volatile unsigned char abnormal = 0; // >400cm case
volatile unsigned long pulse_count = 0;

ISR(TIMER1_COMPA_vect){ // interrupt for abnormal (timer1)
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // turn off timer
    abnormal = 1;
    complete = 1;
    start = 0;
}

ISR(PCINT2_vect){ // interrupt for triggering (PD3)
    if(start == 0){
        abnormal = 0;
        start = 1;
        TCNT1 = 0; //set the count to 0
        TCCR1B |= (1 << CS11);
        TCCR1B &= ~((1 << CS10) | (1 << CS12));// turn on timer
    }
    else{
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // turn off timer
        pulse_count = TCNT1;
        complete = 1;
        start = 0;
    }
}
