#include <Firmata.h>
#include <LedControl.h>
#include <pitches.h>

const int LED_PIN = 13;
const int BUTTON_PIN = 9;
const int BUZZER_PIN = 8;

const int MATRIX_NUM_ROWS = 8;
const int MATRIX_FRAME_MIN_TIME = 100;
const int MATRIX_EXPLOSION_FRAMES = 4;
const byte MATRIX_EXPLOSION_PATTERN[][MATRIX_NUM_ROWS] = {
    {B00100100,B01000010,B10000001,B00011000,B00011000,B10000001,B01000010,B00100100},
    {B01000010,B10000001,B00011000,B00100100,B00100100,B00011000,B10000001,B01000010},
    {B10000001,B00011000,B00100100,B01000010,B01000010,B00100100,B00011000,B10000001},
    {B00011000,B00100100,B01000010,B10000001,B10000001,B01000010,B00100100,B00011000}
};

const int TONE_LOOP_NUM_NOTES = 5;
const int TONE_LOOP_NOTES[TONE_LOOP_NUM_NOTES] = {NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5};

// Firmata commands
const byte FIRMATA_ALERT = 0x01;
const byte FIRMATA_CHECK_BUTTON = 0x02;

const int BUTTON_PRESS_TONE_DURATION_MS = 2000;

/*
 Now we need a LedControl to work with.
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl led_control = LedControl(12,11,10,1);

boolean led_state;

boolean alert = false;
boolean alert_cancelled = false;

boolean button_state = HIGH;

void display_init(){
    /*
     The MAX72XX is in power-saving mode on startup,
     we have to do a wakeup call
     */
    led_control.shutdown(0,false);
    /* Set the brightness to a medium values */
    led_control.setIntensity(0,8);
    /* and clear the display */
    led_control.clearDisplay(0);
}

void firmata_init(){
    Firmata.setFirmwareVersion(1, 0);
    Firmata.attach(START_SYSEX, firmata_sysex_callback);
    Firmata.begin(57600);
}

void matrix_clear(){
    for(int row = 0; row < MATRIX_NUM_ROWS; row++){
        led_control.setRow(0, row, B00000000);
    }
}

int current_matrix_frame = 0;
void render_matrix_explosion_frame(){
    for(int row = 0; row < MATRIX_NUM_ROWS; row++){
        led_control.setRow(0, row, MATRIX_EXPLOSION_PATTERN[current_matrix_frame][row]);
    }
    current_matrix_frame = ++current_matrix_frame % MATRIX_EXPLOSION_FRAMES;
    delay(MATRIX_FRAME_MIN_TIME);
}

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

int current_beep_count = 0;
int current_beep_tone = 0;
void occasionally_beep(int beep_modulo, int beep_duration){
    if(current_beep_count++ % beep_modulo == 0){
        play_tone(TONE_LOOP_NOTES[current_beep_tone++], beep_duration);
    }
    current_beep_count = current_beep_count % beep_modulo;
    current_beep_tone = current_beep_tone % TONE_LOOP_NUM_NOTES;
}

void test_run(){
    int test_loops = 40;
    int test_tone_duration_ms = 20;
    int test_tone = NOTE_C5;
    for(int i=0; i<test_loops; i++){
        render_matrix_explosion_frame();
        if(i % 10 == 0){
            play_tone(test_tone, test_tone_duration_ms);
            toggle_led();
        }
    }
    turn_led_off();
    matrix_clear();
}

void firmata_sysex_callback(byte command, byte argc, byte *argv)
{
    byte mode;
    byte stopTX;
    byte slaveAddress;
    byte data;
    int slaveRegister;
    unsigned int delayTime;
   
    String echoText;
   
    switch (command) {
        case FIRMATA_ALERT:
            // First and only byte should be the alert Boolean
            alert = argv[0] && !alert_cancelled;
        break;
        case FIRMATA_CHECK_BUTTON:
            if(alert_cancelled){
                Firmata.sendString("1");
            }
            else{
                Firmata.sendString("0");
            }
            alert_cancelled = false;
        break;
    }
}

void setup(){
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    display_init();
    test_run();
    firmata_init();
}

void loop(){
    while(Firmata.available()) {
        Firmata.processInput();
    }
    if(digitalRead(BUTTON_PIN) == LOW){
        if(button_state == HIGH){
            if(alert){
                alert_cancelled = true;
                play_tone(NOTE_C3, BUTTON_PRESS_TONE_DURATION_MS);
            }
            alert = false;
        }
        button_state = LOW;
    }
    else{
        button_state = HIGH;
    }
    if(alert){
        render_matrix_explosion_frame();
        occasionally_beep(2, 20);
    }
    else{
        matrix_clear();
    }
}
