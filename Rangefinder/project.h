extern volatile unsigned char a, b;
extern volatile unsigned char changed_rot;  // flag for rotary encoder change
extern volatile unsigned short far;
extern volatile unsigned short near;
extern volatile unsigned char new_state_rot, old_state_rot;
extern volatile unsigned char state_toggle; // 0 for near, 1 for far
volatile unsigned char start; // distance measurement started
volatile unsigned char complete; // distance measurement completed
volatile unsigned char abnormal; // >400cm case
volatile unsigned long pulse_count;
