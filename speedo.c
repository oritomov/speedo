/*
Main routine of speedometer for PIC16F648A
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

#include "common.h"
#include "speed.h"
#include "hodo.h"
#include "display.h"
#include "leds.h"
#include "button.h"

#pragma DATA _CONFIG, _LVP_OFF & _BOREN_ON & _MCLRE_OFF & _PWRTE_ON & _WDT_OFF & _INTOSC_OSC_NOCLKOUT

// (14" * 2.54cm * 0.01 m/cm + 2 * 175mm * 0.001 m/mm * 70%) * pi => 1.89m winter
// (14" * 2.54cm * 0.01 m/cm + 2 * 195mm * 0.001 m/mm * 60%) * pi => 1.85m summer
// 100m / 1.85m * 8 pulses/turn = 54 turns/100m * 8 pulses/turn = 432 pulses / 100m
//#define DEFAULT_PULSES_IN_100M			445   // 432 + 3% = 13
// 3600 sec/hr * 125000 bigticks/sec * 0.1 km => 45000000 smallticks * 100m / hr
// 45000000 / 445 pulses_in_100m * 4 bigticks/smallticks = 25281
//#define DEFAULT_CALIB_FACTOR				25281 // 26042 - 3% = 781

#define MENU_TIMEOUT			12

void sevenseg_text(unsigned char message);

// The display unit's current task is defined by a state machine.  Normally on boot it defaults to MODE_SPD
// which displays the current speed as a normal speedometer.
// MSG_EEE is not a real mode, but it's here to prevent an overflow bug in sevenseg_text
typedef enum { MODE_MENU_CLR, MSG_EEE,
	MSG_UNL, MSG_LPG,
	MODE_SPD, MODE_CLR
				} mainmode;

#define MODE_DEFAULT	MODE_SPD

// ram 
mainmode current_mode;				// tells our state machine which task the main loop is trying to execute

// writes "text" on the 7 segment display by setting BCD position bits
void sevenseg_text(unsigned char message)
{
	gie = 0;
	// block interrupts while we're writing sevenseg_bits

	switch (message)
	{
		case MODE_MENU_CLR:		// calibration (CLr)
			sevenseg_bits[2] = _C; // C
			sevenseg_bits[1] = _L; // L
			sevenseg_bits[0] = _r; // r
			break;
		case MSG_EEE:			// error (EEE)
			sevenseg_bits[2] = _E; // E
			sevenseg_bits[1] = _E;
			sevenseg_bits[0] = _E;
			break;
		case MSG_UNL:    // unleaded (unL)
			sevenseg_bits[2] = _u;
			sevenseg_bits[1] = _n;
			sevenseg_bits[0] = _L;
			break;
		case MSG_LPG:    // lpg (LPG)
			sevenseg_bits[2] = _L;
			sevenseg_bits[1] = _P;
			sevenseg_bits[0] = _G;
			break;
	}
	gie = 1;
}

void main(void)
{
	// initialization of ports, timers, interrupts, variables, etc
	cmcon = 0b00000111;
	trisa = 0b00110000;		// 1 means high impedance (input)
	trisb = 0b00001000;
	porta = 0b00010000;		// 0 is low (gnd), 1 is high (5V)
	portb = 0b00000000;

	clear_wdt();	// do this to avoid a reset
	option_reg = 0b11010110;	// tmr0 triggers off internal clock, tmr0 prescaler to 1:128
	tmr0 = 0x00;			// in case we want to use it later, it's reset.

	init_speed();
	init_hodo();
	init_button();
	init_leds();

	current_mode = MODE_DEFAULT;	// can set MODE_DEFAULT to MODE_TEST for PIC testing

	pir1 = 0b00000000;		// clear the rest of the interrupt flags
	pie1 = 0b00000011;		// enable interrupts: tmr1, tmr2
	intcon = 0b01100000;	// enable interrupts: tmr0 and PEIE
	clear_wdt();

	gie = 1;				// enable global interrupts, now the display can start updating
	check_display();

	// by now we should have collected enough pulses (or not collected any) to get an idea of how fast we're going
	while (1)
	{
		uns24 start_time, end_time;
		unsigned char gen_timer;	// a general variable we can set to tmr1_upper + whatever, and break when they match. Simple software timer.

		reset_button();

		// When control enters a new state, the button flags are cleared, blink is turned off.
		// Display bits are however the last state left them.  Handy for blinking our selection when exiting a menu.

		switch (current_mode)
        {
			case MODE_SPD:	// calculate and show current speed until a button is held
				while (!flag_buttonheld)
				{
					calculate_speed(0);
					write_distance();
					display_speed(speed);
					display_trip(trip);
					display_distance(distance);
					if (flag_lpg_reed != flag_lpg_mode)
					{
						if (flag_lpg_reed)
							current_mode = MSG_LPG - 1;
						else
							current_mode = MSG_UNL - 1;
						flag_lpg_mode = flag_lpg_reed;
						break;
					}
				}
				// switch to first menu state
				current_mode++;
				break;
			case MSG_UNL:			// show mode
			case MSG_LPG:
				sevenseg_text(current_mode);
				gen_timer = tmr1_upper + MENU_TIMEOUT;
				while (tmr1_upper != gen_timer)
				{
					calculate_speed(0);
					write_distance();
					display_distance(distance);
				}
				current_mode = MODE_SPD;
				break;
			case MODE_CLR:			// clear trip
				trip = 0;
				trip_pulses = 0;
				write_trip(0);
				display_trip(trip);
				current_mode = MODE_SPD;
				break;
			case MODE_MENU_CLR:		// show menu
				sevenseg_text(current_mode);
				gen_timer = tmr1_upper + MENU_TIMEOUT;
				while (tmr1_upper != gen_timer)
				{
					if (flag_buttonpressed)
					{
						current_mode++;
						break;
					}
					else if (flag_buttonheld)
					{
						current_mode = current_mode + (MODE_SPD + 1);
						break;
					}
				}
				if (current_mode == MODE_MENU_CLR + 1)
					current_mode = MODE_SPD;		// let's wrap around.
				if (tmr1_upper == gen_timer)
					current_mode = MODE_SPD;
				break;
			default:
				// We should never arrive here.
				// if we're debugging, we want to know about this; print MSG_EEE and lockup.
				sevenseg_text(MSG_EEE);
				while (!flag_buttonpressed)
					;
				// if we're not debugging, just get us back into MODE_SPD.
				current_mode = MODE_SPD;
		}

		clear_wdt();
	}
}

void interrupt(void)
{
	interrupt_leds();
	interrupt_speed();
	interrupt_button();
}