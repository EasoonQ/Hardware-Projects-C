#include "project.h"
#include "encoder.h"

volatile unsigned char changed_rot = 0;
volatile unsigned short far = 50;
volatile unsigned short near = 10;
volatile unsigned char new_state_rot, old_state_rot;
volatile unsigned char state_toggle = 0; // 0 for near, 1 for far
volatile unsigned char a, b;
    
ISR(PCINT1_vect){ // interrupt for rotary encoder (PC1,2)
    // Read the input bits and determine A and B.
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
    // For each state, examine the two input bits to see if state
    // has changed, and if so set "new_state" to the new state,
    // and adjust the count value.
    if (state_toggle == 0) {
        if (old_state_rot == 0) {
            // Handle A and B inputs for state 0
            if(a){
                new_state_rot = 1;
                if (near + 1 + 5 <= far) {
                    near++;
                }
            }
            else if(b){
                new_state_rot = 2;
                if (near - 1 >= 1) {
                    near--;
                }
            }
        }
        else if (old_state_rot == 1) {
            // Handle A and B inputs for state 1
            if(b){
                new_state_rot = 3;
                if (near + 1 + 5 <= far) {
                    near++;
                }
            }
            else if(!a){
                new_state_rot = 0;
                if (near - 1 >= 1 ) {
                    near--;
                }
            }
        }
        else if (old_state_rot == 2) {
            // Handle A and B inputs for state 2
            if(!b){
                new_state_rot = 0;
                if (near + 1 + 5 <= far) {
                    near++;
                }
            }
            else if(a){
                new_state_rot = 3;
                if (near - 1 >= 1) {
                    near--;
                }
            }
        }
        else {   // old_state_rot = 3
            // Handle A and B inputs for state 3
            if(!a){
                new_state_rot = 2;
                if (near + 1 + 5 <= far) {
                    near++;
                }
            }
            else if(!b){
                new_state_rot = 1;
                if (near - 1 >= 1) {
                    near--;
                }
            }
        }
    }
    
    if (state_toggle == 1) {
        if (old_state_rot == 0) {
            // Handle A and B inputs for state 0
            if(a){
                new_state_rot = 1;
                if (far + 1 <= 400) {
                    far++;
                }
            }
            else if(b){
                new_state_rot = 2;
                if (far - 1 - 5 >= near) {
                    far--;
                }
            }
        }
        else if (old_state_rot == 1) {
            // Handle A and B inputs for state 1
            if(b){
                new_state_rot = 3;
                if (far + 1 <= 400) {
                    far++;
                }
            }
            else if(!a){
                new_state_rot = 0;
                if (far - 1 - 5 >= near) {
                    far--;
                }
            }
        }
        else if (old_state_rot == 2) {
            // Handle A and B inputs for state 2
            if(!b){
                new_state_rot = 0;
                if (far + 1 <= 400) {
                    far++;
                }
            }
            else if(a){
                new_state_rot = 3;
                if (far - 1 - 5 >= near) {
                    far--;
                }
            }
        }
        else {   // old_state_rot = 3
            // Handle A and B inputs for state 3
            if(!a){
                new_state_rot = 2;
                if (far + 1 <= 400) {
                    far++;
                }
            }
            else if(!b){
                new_state_rot = 1;
                if (far - 1 - 5 >= near) {
                    far--;
                }
            }
        }
    }
    
    if (new_state_rot != old_state_rot) {
        changed_rot = 1;
        old_state_rot = new_state_rot;
    }
}
    