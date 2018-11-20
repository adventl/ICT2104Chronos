/*
 * inactivityMonitor.c
 * Created on: Nov 21, 2018
 * Author: Nicholas
 */

// System/ Default include stuff
#include "project.h"
#include "display.h"
#include "bmp_as.h"
#include "cma_as.h"
#include "as.h"
#include "ports.h"
#include "buzzer.h"

// useful
#include "inactivitymonitor.h"
#include "user.h"
#include "clock.h"

// *************************************************************************************************
// Global Variable section
struct iamonitor sIamonitor;

// Functions

void reset_iamonitor(void) {
	sIamonitor.state = INACTIVE;
	sIamonitor.inactiveCount = 0;
}

void sx_iamonitor(u8 line) {
	clear_line(LINE1);
	display_time(LINE1, DISPLAY_LINE_UPDATE_FULL);
}

// Temporarily empty
void mx_iamonitor(u8 line) {
}

void display_iamonitor(u8 line, u8 update) {

	display_chars(LCD_SEG_L1_3_0, (u8 *) "IA00", SEG_ON);

	if (update == DISPLAY_LINE_UPDATE_FULL) {
		display_chars(LCD_SEG_L1_3_0, (u8 *) "IA00", SEG_ON);
	} else {
		display_chars(LCD_SEG_L1_3_0, (u8 *) "IDK", SEG_ON);
	}

	u8* str;
	str = int_to_array(sIamonitor.inactiveCount,2,0);
	display_chars(LCD_SEG_L2_3_0, (u8 *) str, SEG_ON);
}

void do_iamonitor(void) {
	sIamonitor.inactiveCount++;
	display.flag.update_iamonitor = 1;
}

// chceking accelerometer goes here
void check_iamonitor(void) {

}
