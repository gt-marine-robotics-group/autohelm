#include "motors.h"

uint16_t port_throttle = 128;
uint16_t stbd_throttle = 128;

MCP_POT pot(MCP_SEL, MCP_RST, MCP_SHDN, MCP_DOUT, MCP_CLK);

int16_t throttle_convert(float input){
  input *= .9f; // -100 -> -90, 100 -> 90
  input += 100.0f; // -100 -> 0
  int16_t throttle = static_cast<int16_t>(input *= 1.28f); // scale to 0-to-256
  throttle = max(min(255, throttle), 0); // ensure within 0 to 255 range
  return throttle;
}

void set_motor_throttles(){
  if (loop_time - last_time > 2000){
    port_throttle = 128;
    stbd_throttle = 128;
  }
  if (port_throttle != 128) {
    digitalWrite(WW_THR0_EN, HIGH);
  } else {
    digitalWrite(WW_THR0_EN, LOW);
  }
  if (stbd_throttle != 128) {
    digitalWrite(WW_THR1_EN, HIGH);
  } else {
    digitalWrite(WW_THR1_EN, LOW);
  }
  pot.setValue(0, port_throttle);
  pot.setValue(1, stbd_throttle);
}

bool set_arm(bool arm){
    if(arm){
    pot.setValue(0, MCP_POT_MIDDLE_VALUE);
    delay(100);
    digitalWrite(WW_M_EN, HIGH);
    delay(1000);
    g_armed = true;
    return true; 
  } else {
    pot.setValue(0, MCP_POT_MIDDLE_VALUE);
    digitalWrite(WW_M_EN, LOW);
    digitalWrite(WW_THR0_EN, LOW);
    digitalWrite(WW_THR1_EN, LOW);
    g_armed = false;
    return true; 
  }  
}