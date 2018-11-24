/*
 * inactivityMonitor.c
 * Created on: Nov 21, 2018
 * Author: Nicholas
 */
#include <string.h>
#include <stdio.h>
#include <msp430.h>

// System/ Default include stuff
#include "project.h"
#include "display.h"
#include "bmp_as.h"
#include "cma_as.h"
#include "as.h"
#include "ports.h"
#include "buzzer.h"

extern u16 convert_acceleration_value_to_mgrav(u8 value);
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
	display_chars(LCD_SEG_L2_4_0, (u8 *) "resia", SEG_ON);
	printf("hi");
}

void sx_iamonitor(u8 line) {
	display_chars(LCD_SEG_L2_4_0, (u8 *) "sx", SEG_ON);
}

// Toggle iaMonitor state on star_long
void mx_iamonitor(u8 line) {
	if(!sIamonitor.state) {
		display_chars(LCD_SEG_L2_3_0, (u8 *) "CALI", SEG_ON);
		sIamonitor.state = CALIBRATING;

	} else {
		display_chars(LCD_SEG_L2_3_0, (u8 *) "OFF-", SEG_ON);
		reset_iamonitor();
	}
}

void calibrate_iamonitor(void) {

	int i;
	extern u16 convert_acceleration_value_to_mgrav(u8 value);

	if (bmp_used) {
		bmp_as_start();
		bmp_as_get_data(sIamonitor.rawReading);
		bmp_as_stop();
	} else {
		cma_as_start();
		cma_as_get_data(sIamonitor.rawReading);
		cma_as_stop();
	}

	for (i = 0; i < 3; i++) {
		sIamonitor.resting[i] = convert_acceleration_value_to_mgrav(sIamonitor.rawReading[i]);
	}

	buzzer_note(1, CONV_MS_TO_TICKS(1000),CONV_MS_TO_TICKS(0), 2);
}

void display_iamonitor(u8 line, u8 update) {

}

void do_iamonitor(void) {

	u8 *str;
    str = int_to_array(sIamonitor.inactiveCount, 5, 4);
	display_chars(LCD_SEG_L2_4_0, (u8 *) "do", SEG_ON);

	if (sIamonitor.state == CALIBRATING) {
		display_chars(LCD_SEG_L2_4_0, (u8 *) "s act", SEG_ON);
		if (sIamonitor.inactiveCount == 5) {
			calibrate_iamonitor();
			sIamonitor.inactiveCount = 0;
			sIamonitor.state = ACTIVE;
		    str = int_to_array(sIamonitor.inactiveCount, 5, 0);
			display_chars(LCD_SEG_L2_4_0, (u8 *) "C is5", SEG_ON);
		}
		sIamonitor.inactiveCount++;
	} else if (sIamonitor.state == ACTIVE) {
		display_chars(LCD_SEG_L2_4_0, (u8 *) "s act", SEG_ON);
		check_iamonitor();
	} else if (sIamonitor.state == RING) {
		buzzer_note(3, CONV_MS_TO_TICKS(333),CONV_MS_TO_TICKS(50), 2);
	}
	display.flag.update_iamonitor = 1;
}

// chceking accelerometer goes here
void check_iamonitor(void) {

	int i;
	extern u16 convert_acceleration_value_to_mgrav(u8 value);
	if (bmp_used) {
		bmp_as_start();
		bmp_as_get_data(sIamonitor.resting);
		bmp_as_stop();
	} else {
		cma_as_start();
		cma_as_get_data(sIamonitor.resting);
		cma_as_stop();
	}

	for (i = 0; i < 3; i++) {
		sIamonitor.converted[i] = convert_acceleration_value_to_mgrav(sIamonitor.rawReading[i]);
	}

	if ((sIamonitor.resting[0] - THRESHOLD <= sIamonitor.converted[0] &&
		sIamonitor.converted[0] <= sIamonitor.resting[0] + THRESHOLD) ||
		(sIamonitor.resting[1] - THRESHOLD <= sIamonitor.converted[1] &&
		sIamonitor.converted[1] <= sIamonitor.resting[1] + THRESHOLD) ||
		(sIamonitor.resting[2] - THRESHOLD <= sIamonitor.converted[2] &&
		sIamonitor.converted[2] <= sIamonitor.resting[2] + THRESHOLD)) {
		buzzer_note(3, CONV_MS_TO_TICKS(200),CONV_MS_TO_TICKS(100), 2);
		sIamonitor.inactiveCount++;
	} else {
		buzzer_note(3, CONV_MS_TO_TICKS(200),CONV_MS_TO_TICKS(100), 20);
		sIamonitor.inactiveCount = 0;
	}

	if (sIamonitor.inactiveCount == ALARMLIMIT) {
		sIamonitor.state = RING;
	}
	buzzer_note(2, CONV_MS_TO_TICKS(500),CONV_MS_TO_TICKS(100), 2);
}

u8 is_iamonitor(void) {
	return sIamonitor.state;
}
