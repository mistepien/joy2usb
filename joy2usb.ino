/*  joy2usb
    Author: mistepien@wp.pl

    Copyright (c) 2022, 2023 Michał Stępień

    GNU GENERAL PUBLIC LICENSE
    Version 3, 29 June 2007

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/


#define UP_BTN 1      //PIND
#define DOWN_BTN 2    //PIND
#define LEFT_BTN 3    //PIND
#define RIGHT_BTN 4   //PIND
#define C64MODE_PIN 7 //PIND
#define FIRE1 0       //PIND
#define FIRE2 5       //PINB
#define FIRE3 6       //PINB

#include <Joystick.h>  //https://github.com/MHeironimus/ArduinoJoystickLibrary


byte minimal_axis_time;
byte minimal_button_time;



Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK,
                   3, 0,                  // Button Count, Hat Switch Count
                   true, true, false,     // X and Y, but no Z Axis
                   false, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering


void debouncing(bool mode = 1) {
  if (mode) {
    minimal_axis_time = 8;
    minimal_button_time = 10;
  } else {
    minimal_axis_time = 0;
    minimal_button_time = 0;
  }
}

void setup() {
  bitSet(DDRE, 6); bitClear(PORTE, 6); //STATUSLED(PE6) as OUTPUT + set to LOW


  bitClear(DDRF, 4); bitSet(PORTF, 4); //INPUT_PULLUP FOR DEBOUNCING SWITCH

  DDRD &= ~( bit(UP_BTN) | bit(DOWN_BTN) | bit(LEFT_BTN) | bit(RIGHT_BTN) | bit(FIRE1) | bit(C64MODE_PIN)) ;  //PD0-PD5 as INPUT
  PORTD |= ( bit(UP_BTN) | bit(DOWN_BTN) | bit(LEFT_BTN) | bit(RIGHT_BTN) | bit(FIRE1)); //FIRE1,UP,DOWN,LEFT,RIGT as INPUT_PULLUP
  PORTD &= ~(bit(C64MODE_PIN)); //C64MODE_PIN as INPUT_NO_PULL_UP

  DDRB &= ~(bit(FIRE2) | bit(FIRE3) | bit(PB1) | bit(PB2) | bit(PB3) | bit(PB4));  //FIRE2 and FIRE3 as INPUT
  PORTB &= ~(bit(FIRE2) | bit(FIRE3));  //FIRE2 and FIRE3 as INPUT_NO_PULL_UP

  //UNUSED PINS
  DDRB &= ~(bit(PB1) | bit(PB2) | bit(PB3) | bit(PB4));  //PB1-PB4 as INPUT_PULLUP -- unused_pins
  PORTB  |= bit(PB1) | bit(PB2) | bit(PB3) | bit(PB4);
  bitClear(DDRC, 6); bitSet(PORTC, 6); //INPUT_PULLUP PC6 -- unused pin

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

  Joystick.sendState();

  /*turn off RX and TX LEDS
    on permanent basis

    turning off these LEDS here at the end of setup()
    is an indicator that joy2usb is ready
  */
  bitClear(DDRD, 5);
  bitClear(DDRB, 0);

  bitSet(PORTE, 6); //STATUSLED(PE6) set to HIGH
}

byte joystate_update() {
  //UP,DOWN,LEFT,RIGHT,F1,F2,F3
  //0, 1,   2,   3,   ,4, 5, 6
  byte pind = ~PIND;
  byte pinb = ~PINB;

  bool C64MODE = bitRead(pind, C64MODE_PIN);
  pind &= (bit(UP_BTN) | bit(DOWN_BTN) | bit(LEFT_BTN) | bit(RIGHT_BTN) | bit(FIRE1));
  pinb &= (bit(FIRE2) | bit(FIRE3));
  bitWrite(pinb, FIRE2, (C64MODE ^ bitRead(pinb, FIRE2)));
  bitWrite(pinb, FIRE3, (C64MODE ^ bitRead(pinb, FIRE3)));
  byte joystate_current = ((pind >> 1) | ((pind & bit(FIRE1)) << 4) | (pinb));
  return joystate_current;
}

//NO-OPPOSITE-DIRECTIONS
bool nod(bool OUT_DIR, bool SECOND_DIR) {
  //bool value = ((!( OUT_DIR  || SECOND_DIR )) || OUT_DIR); //logic where LOW=pressed, HIGH=released
  bool value = ((!( OUT_DIR  && SECOND_DIR )) && OUT_DIR); //logic where HIGH=pressed, LOW=released
  return value;
}

unsigned long prev_button_time[3] = { 0, 0, 0 };
unsigned long prev_axis_time[4] = { 0, 0, 0, 0 };
byte prev_axises;
byte prev_buttons;

void joystate_to_usb(byte joystate) {

  byte current_axises = joystate & B00001111;
  byte current_buttons = (joystate >> 4);

  //Serial.println(current_axises, BIN);

  byte changedAxises = current_axises ^ prev_axises;
  byte changedButtons = current_buttons ^ prev_buttons;

  if (changedAxises || changedButtons) {
    debouncing(bitRead(PINF, 4));
    unsigned long current_time = millis();  //ONE common millis() for AXISES AND BUTTONS SECTIONS

    //####################################################
    if (changedAxises) {  //AXISES SECTION
      byte JOY_D = 0;
      byte JOY_U = 0;
      byte JOY_R = 0;
      byte JOY_L = 0;
      byte axisXchanged = 0;
      byte axisYchanged = 0;
      for (byte index = 0; index < 4; index++) {
        if (bitRead(changedAxises, index)) {
          if (current_time - prev_axis_time[index] > minimal_axis_time) {
            prev_axis_time[index] = current_time;
            prev_axises = current_axises;

            bool btn_state = bitRead(current_axises, index);
            switch (index) {
              case 0:  //UP
                JOY_U = btn_state;
                axisYchanged = 1;
                break;
              case 1:  //DOWN
                JOY_D = btn_state;
                axisYchanged = 1;
                break;
              case 2:  //LEFT
                JOY_L = btn_state;
                axisXchanged = 1;
                break;
              case 3:  //RIGHT
                JOY_R = btn_state;
                axisXchanged = 1;
                break;
            }
          }
        }
      }
      if (axisYchanged) {
        Joystick.setYAxis(nod(JOY_D, JOY_U) - nod(JOY_U, JOY_D));
      }
      if (axisXchanged) {
        Joystick.setXAxis(nod(JOY_R, JOY_L) - nod(JOY_L, JOY_R));
      }
    }

    //##############################################
    if (changedButtons) {  //BUTTONS SECTION
      for (byte index = 0; index < 3; index++) {
        if (bitRead(changedButtons, index)) {
          if (current_time - prev_button_time[index] > minimal_button_time) {
            prev_button_time[index] = current_time;
            prev_buttons = current_buttons;
            Joystick.setButton(index, bitRead(current_buttons, index));
          }
        }
      }
    }
  }
  Joystick.sendState();  //ONE common send.State for AXISES AND BUTTONS SECTIONS

}

void loop() {
  while (1) {
    joystate_to_usb(joystate_update());
  }
}
