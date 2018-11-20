/*
 *  inactivityMonitor.c
 *  Created on: Nov 21, 2018
 *  Author: Nicholas
 */

#ifndef INACTIVITY_H_
#define INACTIVITY_H_

// *************************************************************************************************
// Include section
extern void sx_iamonitor(u8 line);
extern void mx_iamonitor(u8 line);
extern void display_iamonitor(u8 line, u8 update);
extern void do_iamonitor(void);
extern void check_iamonitor(void);
extern void reset_iamonitor(void);
// *************************************************************************************************
// Prototypes section


// *************************************************************************************************
// Defines section
#define ACTIVE     (1u)
#define INACTIVE   (0u)
#define THRESHOLD  (1u)
#define ALARMLIMIT (1u)


// *************************************************************************************************
// Global Variable section

struct iamonitor {
	u8  state;
	u8  inactiveCount;
	u8  alarmLimit;
	u8  threshold;
	u8  rawReading[3];
	u16 converted[3];
};

// *************************************************************************************************
// Extern section
extern struct iamonitor sIamonitor;

#endif
