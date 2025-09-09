#include "globals.h"

ulong loop_time = 0;
ulong last_time = 0;

ServoInputPin<SERVO_1> g_servo1;
ServoInputPin<SERVO_2> g_servo2;
ServoInputPin<SERVO_3> g_servo3;
ServoInputPin<SERVO_4> g_servo4;
ServoInputPin<SERVO_5> g_servo5;

int g_rc_port = 0;
int g_rc_stbd = 0;

int g_rc_srg = 0;
int g_rc_swy = 0; 
int g_rc_yaw = 0;
int g_rc_ctr = 0;
int g_rc_kil = 0;

int g_rc_peff = 0;
int g_rc_seff = 0;

bool g_armed = false;