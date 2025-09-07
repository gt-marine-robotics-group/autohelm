#pragma once

#include <MCP_POT.h>

#include "globals.h"

extern MCP_POT pot;

extern uint16_t port_throttle;
extern uint16_t stbd_throttle;

extern int16_t throttle_convert(float input);

extern void set_motor_throttles();
extern bool set_arm(bool arm);