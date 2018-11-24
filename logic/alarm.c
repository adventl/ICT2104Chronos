// *************************************************************************************************
//
//      Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//        Redistribution and use in source and binary forms, with or without
//        modification, are permitted provided that the following conditions
//        are met:
//
//          Redistributions of source code must retain the above copyright
//          notice, this list of conditions and the following disclaimer.
//
//          Redistributions in binary form must reproduce the above copyright
//          notice, this list of conditions and the following disclaimer in the
//          documentation and/or other materials provided with the
//          distribution.
//
//          Neither the name of Texas Instruments Incorporated nor the names of
//          its contributors may be used to endorse or promote products derived
//          from this software without specific prior written permission.
//
//        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//        "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//        LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//        A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//        OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//        SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//        LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//        DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//        THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//        OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************
// Alarm routines.
// *************************************************************************************************

// *************************************************************************************************
// Include section

// system
#include "project.h"

// driver
#include "display.h"
#include "bmp_as.h"
#include "cma_as.h"
#include "as.h"

#include "buzzer.h"
#include "ports.h"

// logic
#include "alarm.h"
#include "clock.h"
#include "user.h"
#include "acceleration.h"

#include "temperature.h"
#include "adc12.h"
#include "timer.h"

// *************************************************************************************************
// Prototypes section

// *************************************************************************************************
// Defines section

// *************************************************************************************************
// Global Variable section
struct alarm sAlarm;
u8 last_stateX;
u8 last_stateY;
u8 last_stateZ;
u8 secondlast_stateX;
u8 secondlast_stateY;
u8 secondlast_stateZ;

// *************************************************************************************************
// Extern section

// *************************************************************************************************
// @fn          reset_alarm
// @brief       Resets alarmData to 06:30
// @param       none
// @return      none
// *************************************************************************************************
void reset_alarm(void)
{
    // Default alarm time 06:30
    sAlarm.hour = 06;
    sAlarm.minute = 30;

    // Alarm is initially off
    sAlarm.duration = ALARM_ON_DURATION;
    sAlarm.state = ALARM_DISABLED;
}

// *************************************************************************************************
// @fn          check_alarm
// @brief       Check if current time matches alarm time
// @param       none
// @return      none
// *************************************************************************************************
//ALARM FUNCTION BY DANIEL, Modifies default alarm to instead check accelerometer values for change
void check_alarm(void)
{
    u8 *str;
    u8 raw_x;
    u8 raw_y;
    u8 raw_z;
    u16 accel_data;
    u16 adc_result;

    volatile s32 temperature;

    //start_buzzer(1, CONV_MS_TO_TICKS(5000), CONV_MS_TO_TICKS(150));

//    //Only check accel every x seconds
//    if ((sTime.second%3) != 0 )
//        return;
//    // Return if alarm is not enabled
//    if (sAlarm.state != ALARM_ENABLED)
//        return;

    //=================== Get Temperature Value (return if too low <26 celsius i.e. watch not being worn) ========================
    // Convert internal temperature diode voltage
    adc_result = adc12_single_conversion(REFVSEL_0, ADC12SHT0_8, ADC12INCH_10);

    // Convert ADC value to "xx.x °C"
    // Temperature in Celsius
    // ((A10/4096*1500mV) - 680mV)*(1/2.25mV) = (A10/4096*667) - 302
    // = (A10 - 1855) * (667 / 4096)
    temperature = (((s32) ((s32) adc_result - 1855)) * 667 * 10) / 4096;

    // Add temperature offset
    temperature += sTemp.offset;

   // if(temperature<250){

//    if(temperature<700){
//        display_chars(LCD_SEG_L1_3_0, (u8 *) "COLD", SEG_ON);
//        return;
//    }

//    str = int_to_array(temperature, 3, 1);
//    display_chars(LCD_SEG_L1_3_1, str, SEG_ON);


    //=================== Get Acceleration Value ========================
    // Start acceleration sensor
    // Clear previous acceleration value
    sAccel.data = 0;

    // Start sensor
        if (bmp_used) {
            bmp_as_start();
        } else {
            cma_as_start();
        }

    // Set timeout counter
    sAccel.timeout = ACCEL_MEASUREMENT_TIMEOUT;

    // Set mode
    sAccel.mode = ACCEL_MODE_ON;

    // Start with Y-axis values
    sAccel.view_style = DISPLAY_ACCEL_Y;


    // Get data from sensor
    if (bmp_used) {
        bmp_as_get_data(sAccel.xyz);
    } else {
        cma_as_get_data(sAccel.xyz);
    }


    raw_x = sAccel.xyz[0]; // X axis
    raw_y = sAccel.xyz[1]; // y axis
    raw_z = sAccel.xyz[2]; // z axis

    s8 diff_x = raw_x-last_stateX;
    s8 diff_y = raw_y-last_stateY;


    //Include +/- 255 to catch overflow
    if ((diff_x == 0 || diff_x==1 || diff_x==-1 || diff_x==255 || diff_x==-255) && (diff_y == 0 || diff_y==1 || diff_y==-1 || diff_y==255 || diff_y==-255) ) {
        sAlarm.state = ALARM_ON;
        //display_chars(LCD_SEG_L1_2_0, (u8 *) "ASS", SEG_ON);

    }
    else {
        //display_chars(LCD_SEG_L1_2_0, (u8 *) "OUT", SEG_ON);
    }



    //Save raw_data to the global
    //secondlast_stateX = last_stateX;
    //secondlast_stateY = last_stateY;
    //secondlast_stateZ = last_stateZ;
    last_stateX = raw_x;
    last_stateY = raw_y;
    //last_stateZ = raw_z;

    // Stop acceleration sensor
    if (bmp_used) {
        bmp_as_stop();
    } else {
        cma_as_stop();
    }
    // Clear mode
    sAccel.mode = ACCEL_MODE_OFF;
}

// *************************************************************************************************
// @fn          stop_alarm
// @brief       Stop active alarm
// @param       none
// @return      none
// *************************************************************************************************
void stop_alarm(void)
{
    // Indicate that alarm is enabled, but not active
    sAlarm.state = ALARM_ENABLED;

    // Stop buzzer
    stop_buzzer();
}

// *************************************************************************************************
// @fn          sx_alarm
// @brief       Sx button turns alarm on/off.
// @param       u8 line         LINE1
// @return      none
// *************************************************************************************************
void sx_alarm(u8 line)
{
    // UP: Alarm on, off
    if (button.flag.up)
    {
        // Toggle alarm state
        if (sAlarm.state == ALARM_DISABLED)
        {
            sAlarm.state = ALARM_ENABLED;

            // Show "  on" message
            message.flag.prepare = 1;
            message.flag.type_alarm_on = 1;

            display_chars(LCD_SEG_L2_5_0, "  ON", SEG_ON);
        }
        else if (sAlarm.state == ALARM_ENABLED)
        {
            sAlarm.state = ALARM_DISABLED;

            // Show "  off" message
            message.flag.prepare = 1;
            message.flag.type_alarm_off = 1;
            display_chars(LCD_SEG_L2_5_0, " OFF", SEG_ON);
        }
    }
}

// *************************************************************************************************
// @fn          mx_alarm
// @brief       Set alarm time.
// @param       u8 line LINE1
// @return      none
// *************************************************************************************************
void mx_alarm(u8 line)
{
    u8 select;
    s32 hours;
    s32 minutes;
    u8 *str;

    // Clear display
    clear_display_all();

    // Keep global values in case new values are discarded
    hours = sAlarm.hour;
    minutes = sAlarm.minute;

    // Display HH:MM (LINE1)
    str = int_to_array(hours, 2, 0);
    display_chars(LCD_SEG_L1_3_2, str, SEG_ON);
    display_symbol(LCD_SEG_L1_COL, SEG_ON);

    str = int_to_array(minutes, 2, 0);
    display_chars(LCD_SEG_L1_1_0, str, SEG_ON);

    // Init value index
    select = 0;

    // Loop values until all are set or user breaks set
    while (1)
    {
        // Idle timeout: exit without saving
        if (sys.flag.idle_timeout)
            break;

        // STAR (short): save, then exit
        if (button.flag.star)
        {
            // Store local variables in global alarm time
            sAlarm.hour = hours;
            sAlarm.minute = minutes;
            // Set display update flag
            display.flag.line1_full_update = 1;
            break;
        }

        switch (select)
        {
            case 0:            // Set hour
                set_value(&hours, 2, 0, 0, 23, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE +
                          SETVALUE_NEXT_VALUE, LCD_SEG_L1_3_2,
                          display_hours);
                select = 1;
                break;

            case 1:            // Set minutes
                set_value(&minutes, 2, 0, 0, 59, SETVALUE_ROLLOVER_VALUE + SETVALUE_DISPLAY_VALUE +
                          SETVALUE_NEXT_VALUE, LCD_SEG_L1_1_0,
                          display_value);
                select = 0;
                break;
        }
    }

    // Clear button flag
    button.all_flags = 0;

    // Indicate to display function that new value is available
    display.flag.update_alarm = 1;
}

// *************************************************************************************************
// @fn          display_alarm
// @brief       Display alarm time. 24H / 12H time format.
// @param       u8 line         LINE1, LINE2
//                              u8 update       DISPLAY_LINE_UPDATE_FULL, DISPLAY_LINE_CLEAR
// @return      none
// *************************************************************************************************
void display_alarm(u8 line, u8 update)
{
    u8 hour12;

    if (update == DISPLAY_LINE_UPDATE_FULL)
    {
        display_chars(switch_seg(line, LCD_SEG_L1_3_2,LCD_SEG_L2_3_2), (u8 *) "FA", SEG_ON);
        display_chars(switch_seg(line, LCD_SEG_L1_1_0,LCD_SEG_L2_1_0), (u8 *) "LL", SEG_ON);
        /*
        if (sys.flag.use_metric_units)
        {
            // Display 24H alarm time "HH:MM"
            display_chars(switch_seg(line, LCD_SEG_L1_3_2,
                                     LCD_SEG_L2_3_2), int_to_array(sAlarm.hour, 2, 0), SEG_ON);
        }
        else
        {
            // Display 12H alarm time "HH:MM" + AM/PM
            hour12 = convert_hour_to_12H_format(sAlarm.hour);
            display_chars(switch_seg(line, LCD_SEG_L1_3_2,
                                     LCD_SEG_L2_3_2), int_to_array(hour12, 2, 0), SEG_ON);

            // Display AM/PM symbol
            display_am_pm_symbol(sAlarm.hour);
        }
        display_chars(switch_seg(line, LCD_SEG_L1_1_0,
                                 LCD_SEG_L2_1_0), int_to_array(sAlarm.minute, 2, 0), SEG_ON);
        display_symbol(switch_seg(line, LCD_SEG_L1_COL, LCD_SEG_L2_COL0), SEG_ON);
        */
        // Show blinking alarm icon
        display_symbol(LCD_ICON_ALARM, SEG_ON_BLINK_ON);
    }
    else if (update == DISPLAY_LINE_CLEAR)
    {
        // Clean up function-specific segments before leaving function
        display_symbol(LCD_SYMB_AM, SEG_OFF);

        // Clear / set alarm icon
        if (sAlarm.state == ALARM_DISABLED)
        {
            display_symbol(LCD_ICON_ALARM, SEG_OFF_BLINK_OFF);
        }
        else
        {
            display_symbol(LCD_ICON_ALARM, SEG_ON_BLINK_OFF);
        }
    }
}

