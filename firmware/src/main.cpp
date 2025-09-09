#include <Arduino.h>

#include <rc_input.h>
#include <pins.h>

#include "motors.h"
#include "globals.h"

RCInput rcInput(g_servo5, g_servo2, g_servo3, g_servo4, g_servo1);


enum states {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,
  AGENT_DISCONNECTED
} state;


// Translate RC input to 2 motor system
void set_motor_2x() {
  int port = (g_rc_srg - g_rc_yaw);
  int stbd = (g_rc_srg + g_rc_yaw);
  float max_val = max(100, max(abs(port), abs(stbd))) / 100;
  g_rc_peff = port;
  g_rc_seff = stbd;
}

void exec_mode(int mode, bool killed) {
  // Vehicle Logic
  if (killed) {
    delay(1);
  } else {
    if (mode + 1 == RCInput::ControlState::autonomous) {  // AUTONOMOUS
      if (!g_armed) {
        set_arm(true);
      }
      digitalWrite(RED_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    } else if (mode + 1 == RCInput::ControlState::calibration) {  // CALIBRATION
      if (g_armed) {
        set_arm(false);
      }
      rcInput.check_calibration_ready();
      digitalWrite(RED_LED, HIGH);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else if (mode + 1 == RCInput::ControlState::remote_control) {  // REMOTE CONTROL
      if (!g_armed) {
        set_arm(true);
      }
      set_motor_2x();
      port_throttle = throttle_convert((float)g_rc_peff);
      stbd_throttle = throttle_convert((float)g_rc_seff);
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    }
  }
}


void setup() {
  // Set all LEDs to be output and on
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);  
  pinMode(YELLOW_LED, OUTPUT);
  digitalWrite(YELLOW_LED, HIGH);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);

  Serial.begin(115200);

  g_servo1.attach();
  g_servo2.attach();
  g_servo3.attach();
  g_servo4.attach();
  g_servo5.attach();

  delay(2000);



  // Turn off red to indicate microros transports
  digitalWrite(RED_LED, LOW);  

  SPI.begin();
  pot.begin();

  pot.setValue(0, MCP_POT_MIDDLE_VALUE);
  pot.setValue(1, MCP_POT_MIDDLE_VALUE);
  
  rcInput.calibrate();

  delay(500);
  // Turn off yellow to indicate SPI, Pot, RC ready
  Serial.println("======= CALIBRATION COMPLETE - RC READY =======");
  digitalWrite(YELLOW_LED, LOW);

  // Turn off green to indicate ROS entities created
  digitalWrite(GREEN_LED, LOW);
}

void loop() {
  loop_time = millis();
 
  rcInput.read();
  exec_mode(rcInput.get_ctr_state(), false);
  
  set_motor_throttles();
  delay(100);
}