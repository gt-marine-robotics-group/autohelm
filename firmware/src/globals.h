#pragma once

#include <Arduino.h>
#include <pins.h>
#include <ServoInput.h>

// Everything in this file should be truly global
// Ex: Serial, Wire, SPI

extern ulong loop_time;
extern ulong last_time;

extern ServoInputPin<SERVO_1> g_servo1;
extern ServoInputPin<SERVO_2> g_servo2;
extern ServoInputPin<SERVO_3> g_servo3;
extern ServoInputPin<SERVO_4> g_servo4;
extern ServoInputPin<SERVO_5> g_servo5;

extern int g_rc_peff; // RC Port Effort
extern int g_rc_seff; // RC Starboard Effort

extern int g_rc_srg;  // Commanded Surge
extern int g_rc_swy;  // Commanded Sway
extern int g_rc_yaw;  // Commanded Yaw
extern int g_rc_ctr;  // Commanded Control State
extern int g_rc_kil;  // Commanded Kill State

extern bool g_armed; 