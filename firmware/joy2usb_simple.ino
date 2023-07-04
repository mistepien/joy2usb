/*  joy2usb (simple version)
 *  Author: mistepien@wp.pl
 *
 *  Copyright (c) 2022 Michał Stępień
 *
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#include <Joystick.h> //https://github.com/MHeironimus/ArduinoJoystickLibrary

#define minimal_btime 2

//#define DEBUG

byte minimal_button_time[5];
void debouncing(bool mode = 1) {
  if ( mode ) {
  minimal_button_time[0] = minimal_btime * 5;
  minimal_button_time[1] = minimal_btime * 4;
  minimal_button_time[2] = minimal_btime * 4;
  minimal_button_time[3] = minimal_btime * 5;
  minimal_button_time[4] = minimal_btime * 5;
  } else {
  minimal_button_time[0] = 0;
  minimal_button_time[1] = 0;
  minimal_button_time[2] = 0;
  minimal_button_time[3] = 0;
  minimal_button_time[4] = 0;
  }
}


#ifdef DEBUG
//for performance monitor
unsigned long timer = 0;
unsigned long loops = 0;
#define SEKUNDA 1000000
#endif



Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
                   3, 0,            // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering


byte C64Mode = 0;

#define SAMPLES 10

void setup() {
DDRF &= ~bit(4); PORTF |= bit(4); //INPUT_PULLUP FOR DEBOUNCING SWITCH
  
DDRE &= ~bit(6); PORTE |= bit(6); //INPUT_PULLUP FOR C64/AMIGA MODE SWITCH
//READ C64/AMIGA MODE SWITCH STATE
  byte C64ModeTEMP = 0;  
  for (byte index = 0; index  < SAMPLES; index++) { 
  //READ C64/AMIGA SWITCH MODE
  delayMicroseconds(30);
  C64ModeTEMP = (1 ^ bitRead(PINE,6)) + C64ModeTEMP; 
  }
  if ( C64ModeTEMP / SAMPLES > 0.5 ) {
    C64Mode = 1;
  } else {
    C64Mode = 0;
  }

DDRC |= bit(6); //F2F3MODE as OUTPUT
PORTC &= ~bit(6); //F2F3MODE as LOW 

DDRD &= ~B00011111; PORTD |=  B00011111; //PD0-PD4 as INPUT_PULLUP


  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
  Joystick.begin(false);
  Joystick.setXAxis(0);
  Joystick.setYAxis(0);
 
  delay(1500); /*very ugly and dirty hack
                without that delay() joystick will not
                be centered at the beginning (that is an
                issue with Joystick.sendState();
                */

DDRB &= ~B01100000; //INPUT for F2 and F3
if (C64Mode){ //FOR C64 -- INPUT + PULL_DOWN (via IC4066) -- INPUT IS SET ONE LINE ABOVE
  PORTC |= bit(6); //F2F3MODE as HIGH
} else {      //FOR AMIGA -- INPUT_PULLUP + NO PULL_DOWN (switches in IC4066 are off)
  PORTB |= B01100000; //INPUT_PULLUP for F2 and F3
  PORTC &= ~bit(6); //F2F3MODE as LOW 
}


                
  debouncing(bitRead(PINF,4));
                
  Joystick.sendState();

/* turn off RX and TX LEDS
 * on permanent basis
 * 
 * turning off these LEDS here at the end of setup()
 * is an indicator that joy2usb is ready
 */
bitClear(DDRD,5);
bitClear(DDRB,0);
}


int8_t prev_button_state[5] = { 0, 0, 0, 0, 0 };
unsigned long prev_button_time[5] = { 0, 0, 0, 0, 0 };
byte JOY_D;
byte JOY_U;
byte JOY_R;
byte JOY_L;
byte prev_joy;

void loop() {
byte pinb = 0;
byte pind = 0;
  
pind = ~PIND & B00011111; //clear PD5,PD6 and PD7
if (C64Mode){
  pinb =  PINB & B01100000; //INPUT LOGIC + clear all except PB5 and PB6
} else {
  pinb = ~PINB & B01100000; //INPUT_PULLUP LOGIC -- inverted + clear all except PB5 and PB6
}
byte current_joy=pind | pinb; //PUT TOGETHER pinb and pind, bit7(out of 0-7) is always cleared


if ( current_joy ^ prev_joy ) {
//  Serial.println(current_joy, BIN);
unsigned long current_button_time = millis();
int8_t current_button_state;
  
    for (byte index = 0; index < 5; index++) {
      switch (index) {
        case 0:        // F1
          current_button_state = bitRead(current_joy,0);
          break;
        case 1:        // Y Axis Up && DOWN
          JOY_D = bitRead(current_joy,2);
          JOY_U = bitRead(current_joy,1);
          if ( ( JOY_U ) && ( JOY_D ) ) {
            JOY_D = 1 ^ JOY_D;
          }
          current_button_state = JOY_D - JOY_U;
          break;
        case 2:  // X Axis RIGHT && LEFT
          JOY_R = bitRead(current_joy,4);
          JOY_L = bitRead(current_joy,3);
          if ( ( JOY_R ) && ( JOY_L ) ) {
            JOY_R = 1 ^ JOY_R;
            JOY_L = 1 ^ JOY_L;
          }
          current_button_state = JOY_R - JOY_L;
          break;
        case 3:  // FIRE2
           current_button_state = bitRead(current_joy,5);
           break;
        case 4:  // FIRE3
          current_button_state = bitRead(current_joy,6);
          break;
      }
      //HERE current_button_state is validated for current index value
      if ((current_button_state != prev_button_state[index]) && (current_button_time - prev_button_time[index] > minimal_button_time[index]))
      {
          prev_button_state[index] = current_button_state;
          prev_button_time[index] = current_button_time;
          prev_joy = current_joy;
          pushbutton(index,current_button_state);       
      }
    }
   Joystick.sendState(); 
}


#ifdef DEBUG
    // PERFORMANCE MONITOR
    // it shows how many loops are executed in 1s
    if (micros() - timer >= SEKUNDA) {
      Serial.print("loop() cycles in last second: ");
      Serial.println(loops);
      loops = 0;
      timer = micros();
    }
    loops++;  //increassing a counter
  
#endif
  
}


void pushbutton(byte btn, int8_t btn_pressed) {
    switch (btn) {
        case 0:        // F1
          Joystick.setButton(0, btn_pressed);
          break;
        case 1:        // Y Axis Up && DOWN
          Joystick.setYAxis(btn_pressed);
          break;
        case 2:      // X Axis RIGHT && LEFT
          Joystick.setXAxis(btn_pressed);
          break;
        case 3:        // F2
          Joystick.setButton(1, btn_pressed);
          break;
        case 4:        // F3
          Joystick.setButton(2, btn_pressed);
          break;                       
    }
}
