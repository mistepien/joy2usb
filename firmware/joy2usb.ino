/*  joy2usb
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



#include <Joystick.h>  //https://github.com/MHeironimus/ArduinoJoystickLibrary



//#define DEBUG

const byte minimal_axis_time = 8;
const byte minimal_button_time = 10;
                                    

#ifdef DEBUG
// for performance monitor
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

void setup() {
DDRE &= ~bit(6); PORTE |= bit(6); //INPUT_PULLUP FOR C64/AMIGA MODE SWITCH
C64Mode = (1 ^ bitRead(PINE,6)); //READ C64/AMIGA MODE SWITCH STATE

DDRC |= bit(6); //F2F3MODE as OUTPUT
DDRD &= ~B00011111; PORTD |=  B00011111; //PD0-PD4 as INPUT_PULLUP
DDRB &= ~B01100000; //INPUT for F2 and F3
if (C64Mode){ //FOR C64 -- INPUT + PULL_DOWN (via IC4066) -- INPUT IS SET ONE LINE ABOVE
  PORTC |= bit(6); //F2F3MODE as HIGH
} else {      //FOR AMIGA -- INPUT_PULLUP + NO PULL_DOWN (switches in IC4066 are off)
  PORTB |= B01100000; //INPUT_PULLUP for F2 and F3
  PORTC &= ~bit(6); //F2F3MODE as LOW 
}


  Joystick.begin(false);
  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
  Joystick.setXAxis(0);
  Joystick.setYAxis(0);
  
  delay(3000); /*very ugly and dirty hack
                without that delay() joystick will not
                be centered at the beginning
                */
  Joystick.sendState();

/*turn off RX and TX LEDS
 * on permanent basis
 * 
 * turning off these LEDS here at the end of setup()
 * is an indicator that joy2usb is ready
 */
bitClear(DDRD,5);
bitClear(DDRB,0);
}



byte prev_button_state[3] = { 0, 0, 0};
byte prev_axis_state[4] = { 0, 0, 0, 0};
unsigned long prev_button_time[3] = { 0, 0, 0 };
unsigned long prev_axis_time[4] = { 0, 0, 0, 0 };
byte JOY_D = 0;
byte JOY_U = 0;
byte JOY_R = 0;
byte JOY_L = 0;
byte prev_axises;
byte prev_buttons;

void loop() {
byte pinb;
byte pind;
byte current_state;

pind = ~PIND & B00011111; //clear PD5,PD6 and PD7
if (C64Mode){
pinb =  PINB & B01100000; //INPUT LOGIC + clear all except PB5 and PB6
} else {
pinb = ~PINB & B01100000; //INPUT_PULLUP LOGIC -- inverted + clear all except PB5 and PB6
}
byte current_axises = pind >> 1; //0,1 -- Y axis, 2,3 -X axis
byte current_buttons = (pind & B00000001) | (pinb >> 4); /* eg. B00000111 (when F1, F2 and F3
                                                          are pressed the same time)
                                                          -- bit number is number of button */
//Serial.println(current_axises, BIN);

byte changedAxises = current_axises ^ prev_axises;
byte changedButtons = current_buttons ^ prev_buttons;

if ( changedAxises || changedButtons ) {
unsigned long current_time = millis(); //ONE common millis() for AXISES AND BUTTONS SECTIONS

if ( changedAxises ) { //AXISES SECTION


byte axisXchanged = 0;
byte axisYchanged = 0;
    for (byte index = 0; index < 4; index++) {
        current_state = bitRead(current_axises,index);
        if ((current_state != prev_axis_state[index]) && (current_time - prev_axis_time[index] > minimal_axis_time)) {
          prev_axis_state[index] = current_state;
          prev_axis_time[index] = current_time;
          prev_axises = current_axises;
          switch (index) {
            case 0: //UP
              JOY_U = current_state; axisYchanged = 1;
              break;
            case 1: //DOWN
              JOY_D = current_state; axisYchanged = 1;
              break;
            case 2: //LEFT
              JOY_L = current_state; axisXchanged = 1;
              break;
            case 3: //RIGHT
              JOY_R = current_state; axisXchanged = 1;
              break;
          }
        }
    }
    if ( axisYchanged ) {
      if ( ( JOY_U ) && ( JOY_D ) ) {
            JOY_D = 1 ^ JOY_D;
          }
      Joystick.setYAxis(JOY_D - JOY_U);
      }
    if ( axisXchanged ) {
      if ( ( JOY_L ) && ( JOY_R ) ) {
            JOY_R = 1 ^ JOY_R;
            JOY_L = 1 ^ JOY_L;
          }
      Joystick.setXAxis(JOY_R - JOY_L);
    }  
} 

  if ( changedButtons ) { //BUTTONS SECTION    
    for (byte index = 0; index < 3; index++) {
        current_state = bitRead(current_buttons,index);
        if ((current_state != prev_button_state[index]) && (current_time - prev_button_time[index] > minimal_button_time)) {
          prev_button_state[index] = current_state;
          prev_button_time[index] = current_time;
          prev_buttons = current_buttons;
          Joystick.setButton(index, current_state);
      }
    }

  } 
   Joystick.sendState(); //ONE common send.State for AXISES AND BUTTONS SECTIONS
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
