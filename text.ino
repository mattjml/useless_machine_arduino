#include <pitches.h>
#include <LedControl.h>

int LED_PIN = 13;
int BUTTON_PIN = 9;
int BUZZER_PIN = 8;

int MATRIX_NUM_ROWS = 8;
int MATRIX_FRAME_MIN_TIME = 100;
int MATRIX_EXPLOSION_FRAMES = 4;
byte MATRIX_EXPLOSION_PATTERN[][8] = {
    {B00100100,B01000010,B10000001,B00011000,B00011000,B10000001,B01000010,B00100100},
    {B01000010,B10000001,B00011000,B00100100,B00100100,B00011000,B10000001,B01000010},
    {B10000001,B00011000,B00100100,B01000010,B01000010,B00100100,B00011000,B10000001},
    {B00011000,B00100100,B01000010,B10000001,B10000001,B01000010,B00100100,B00011000}
};

/*
 Now we need a LedControl to work with.
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(12,11,10,1);

int led_state;
void turn_led_on(){
    digitalWrite(LED_PIN, HIGH);
    led_state = HIGH;
}

void turn_led_off(){
    digitalWrite(LED_PIN, LOW);
    led_state = LOW;
}

void toggle_led(){
    if(led_state == HIGH){
        turn_led_off();
    }
    else{
        turn_led_on();
    }

}

void play_tone(int note, int milliseconds){
    tone(BUZZER_PIN, note, milliseconds);
}

void display_init(){
    /*
     The MAX72XX is in power-saving mode on startup,
     we have to do a wakeup call
     */
    lc.shutdown(0,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(0,8);
    /* and clear the display */
    lc.clearDisplay(0);
}

void matrix_clear(){
    for(int row = 0; row < MATRIX_NUM_ROWS; row++){
        lc.setRow(0, row, B00000000);
    }
}

int current_matrix_frame = 0;
void render_matrix_explosion_frame() {
    for(int row = 0; row < MATRIX_NUM_ROWS; row++){
        lc.setRow(0, row, MATRIX_EXPLOSION_PATTERN[current_matrix_frame][row]);
    }
    current_matrix_frame = ++current_matrix_frame % MATRIX_EXPLOSION_FRAMES;
    delay(MATRIX_FRAME_MIN_TIME);
}

void test_run(){
    int test_tone_duration = 20;
    int test_tone = NOTE_C5;
    for(int i=0; i<100; i++){
        render_matrix_explosion_frame();
        if(i % 10 == 0){
            play_tone(test_tone, test_tone_duration);
            toggle_led();
        }
    }
    turn_led_off();
    matrix_clear();
}

void setup(){
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    display_init();
    test_run();
}

int alerted = 1;
int button_state = HIGH;
void loop(){
    if(digitalRead(BUTTON_PIN) == LOW)
    {
        if(button_state == HIGH){
            alerted = !alerted;
            play_tone(NOTE_C5, 500);
        }
        button_state = LOW;
    }
    else{
        button_state = HIGH;
    }
    if(alerted)
    {
        render_matrix_explosion_frame();
    }
    else{
        matrix_clear();
    }
}
