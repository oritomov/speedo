/*
Test.
version: 0.1

This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.



This program should compile under SourceBoost.  Feel free to modify it
to compile with any program you wish, for any platform you wish.
*/

//#include "speed.h"
//#include "hodo.h"
#include "common.h"
#include "display.h"
#include "leds.h"
#include "button.h"

#pragma DATA _CONFIG, _LVP_OFF & _BOREN_ON & _MCLRE_OFF & _PWRTE_ON & _WDT_OFF & _INTOSC_OSC_NOCLKOUT
#define MENU_TIMEOUT            12

typedef enum
{ 
	MODE_MENU_TEST, MODE_TEST, MODE_TEST2, MODE_TEST3
} mainmode;

// ram
mainmode current_mode;              // tells our state machine which task the main loop is trying to execute

// writes "text" on the 7 segment display by setting BCD position bits
void sevenseg_text(unsigned char message)
{
    gie = 0;
    // block interrupts while we're writing sevenseg_bits

    switch (message)
    {
        case MODE_MENU_TEST:    // test mode (tSt)
            sevenseg_bits(2, SEVSEG_BITFIELD(0,1,1,1,1,0,0,0));
            sevenseg_bits(1, SEVSEG_BITFIELD(0,1,1,0,1,1,0,1));
            sevenseg_bits(0, SEVSEG_BITFIELD(0,1,1,1,1,0,0,0));
            break;
        default:                // error (EEE)
            sevenseg_bits(2, SEVSEG_BITFIELD(0,1,1,1,1,0,0,1));
            sevenseg_bits(1, SEVSEG_BITFIELD(0,1,1,1,1,0,0,1));
            sevenseg_bits(0, SEVSEG_BITFIELD(0,1,1,1,1,0,0,1));
            break;
    }
    gie = 1;
}

void main(void)
{
    // initialization of ports, timers, interrupts, variables, etc
    cmcon = 0b00000111;
    // 1 means high impedance (input)
    trisa = 0b00110000;     // buton & lpg
    trisb = 0b00001000;
    // 0 is low (gnd), 1 is high (5V)
    porta = 0b00000000;     
    portb = 0b00000000;

	init_led();
    //init_speed();
    //init_hodo();

    pir1 = 0b00000000;  // clear the rest of the interrupt flags
    pie1 = 0b00000011;  // enable interrupts: tmr1, tmr2
    intcon = 0b01100000;    // enable interrupts: tmr0 and PEIE
    clear_wdt();

    current_mode = MODE_MENU_TEST;    // PIC testing

    while (1)
    {
        unsigned char gen_timer;    // a general variable we can set to tmr1_upper + whatever, and break when they match. Simple software timer.

    	reset_button();
        //calculate_speed(0);
        //write_distance();
        switch (current_mode)
        {
            case MODE_MENU_TEST:    // show menu
                sevenseg_text(current_mode);
                gen_timer = tmr1_upper + MENU_TIMEOUT;
                while (tmr1_upper != gen_timer)
                {
                    if (is_buttonpressed() || is_buttonheld())
                    {
                        current_mode++;
                        break;
                    }
                }
                if (tmr1_upper == gen_timer)
                    current_mode = MODE_MENU_TEST;
                break;
            case MODE_TEST:         // display test mode
                // TEST: running numbers. display tmr1_upper on the LED until button is pressed
                while (!(is_buttonpressed() || is_buttonheld()))
                {
                    display_speed(tmr1_upper());
                    if (tmr1h.7)    // blink all dots on and off to show we are in test mode
                    {
                        sevenseg_bits(2,sevenseg_bits(2) | SEVSEG_DECIMAL_MASK);
                    }
                    sevenseg_bits(0, sevenseg_bits(0) | (BUTTON_INPUT ? SEVSEG_DECIMAL_MASK : 0));
                    sevenseg_bits(1, sevenseg_bits(1) | (SPEEDO_INPUT ? SEVSEG_DECIMAL_MASK : 0));
                }
                current_mode = MODE_TEST2;
                break;
            case MODE_TEST2:            // button input test mode for 15 seconds, button doesn't change modes
                // TEST2: Check our button
                gen_timer = tmr1_upper + 30;
                display_speed(0);
                while (gen_timer != tmr1_upper)
                {
                    if (button_heldcount() > 0)
                        display_speed(button_heldcount());
                    if (tmr1h.7)    // blink all dots on and off to show we are in test mode
                    {
                        sevenseg_bits(1, sevenseg_bits(1) | SEVSEG_DECIMAL_MASK);
                        sevenseg_bits(0, sevenseg_bits(0) | SEVSEG_DECIMAL_MASK);
                    }
                    sevenseg_bits(2, sevenseg_bits(2) | (BUTTON_INPUT ? SEVSEG_DECIMAL_MASK : 0));
                }
                current_mode = MODE_TEST3;
                break;
            case MODE_TEST3:            // speedo input test mode
                // TEST3: count pulses to test speedometer input
                pulsecount(0);
                while (!(is_buttonpressed() || is_buttonheld()))
                {
                    if (pulsecount() > 512)
                        pulsecount(0);     // wrap because we don't display numbers higher than 512
                    display_speed(pulsecount());
                    if (tmr1h.7)    // blink all dots on and off to show we are in test mode
                    {
                        sevenseg_bits(2, sevenseg_bits(2) | SEVSEG_DECIMAL_MASK);
                        sevenseg_bits(1, sevenseg_bits(1) | SEVSEG_DECIMAL_MASK);
                        sevenseg_bits(0, sevenseg_bits(0) | SEVSEG_DECIMAL_MASK);
                    }
                }
                current_mode = MODE_MENU_TEST;
                break;
		}
    }
}

// handles ALL interrupts
void interrupt(void)
{
    interrupt_led();
    //interrupt_speed();
    interrupt_button();
}