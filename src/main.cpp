#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <micro_ros_utilities/string_utilities.h>

#include <rc_input.h>
#include <pins.h>

#include "ros.h"
#include "motors.h"
#include "globals.h"

RCInput rcInput(g_servo1, g_servo2, g_servo3, g_servo4, g_servo5);


enum states {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,
  AGENT_DISCONNECTED
} state;


bool ros_create_entities() {
  delay(1000);
  allocator = rcl_get_default_allocator();

  // Initialize ROS 2 support
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node
  RCCHECK(rclc_node_init_default(&node, "autohelm_node", "", &support));

  // create publisher
  RCCHECK(rclc_publisher_init_default(
    &estop_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Bool),
    "/autohelm/estop"));

  // Create subscriber for port motor with topic /wamv/port_motor
  RCCHECK(rclc_subscription_init_default(
    &port_motor,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float64),
    "/autohelm/port_motor"));

  // Create subscriber for starboard motor with topic /wamv/stbd_motor
  RCCHECK(rclc_subscription_init_default(
    &stbd_motor,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float64),
    "/autohelm/stbd_motor"));

  const unsigned int timer_timeout = 1000;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  RCCHECK(rclc_service_init_default(&service, &node, ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, SetBool), "/autohelm/arm"));
 
  // Create executor for handling both subscriptions
  RCCHECK(rclc_executor_init(&executor, &support.context, 4, &allocator));  // Allow 2 subscriptions
  RCCHECK(rclc_executor_add_service(&executor, &service, &req, &res, service_callback));
  RCCHECK(rclc_executor_add_subscription(&executor, &port_motor, &port_msg, &subscription_callback_port, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_subscription(&executor, &stbd_motor, &stbd_msg, &subscription_callback_stbd, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  return true;
}

void ros_destroy_entities() {
  rmw_context_t *rmw_context = rcl_context_get_rmw_context(&support.context);
  (void)rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

  rcl_subscription_fini(&stbd_motor, &node);
  rcl_subscription_fini(&port_motor, &node);
  rcl_service_fini(&service, &node);
  rcl_publisher_fini(&estop_pub, &node);
  rcl_timer_fini(&timer);
  rclc_executor_fini(&executor);
  rcl_node_fini(&node);
  rclc_support_fini(&support);
}

void ros_handler() {
  bool created = false;
  switch (state) {
    case WAITING_AGENT:
      EXECUTE_EVERY_N_MS(200, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 2)) ? AGENT_AVAILABLE : WAITING_AGENT;);
      // digitalWrite(YELLOW_LED, HIGH);  
      break;
    case AGENT_AVAILABLE:
      created = ros_create_entities();
      state = (true == created) ? AGENT_CONNECTED : WAITING_AGENT;
      delay(100);
      digitalWrite(YELLOW_LED, LOW);
      if (state == WAITING_AGENT) {
        ros_destroy_entities();
        // digitalWrite(GREEN_LED, HIGH);         
      };
      break;
    case AGENT_CONNECTED:
      EXECUTE_EVERY_N_MS(1000, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 4)) ? AGENT_CONNECTED : AGENT_DISCONNECTED;);
      digitalWrite(GREEN_LED, HIGH); 
      break;
    case AGENT_DISCONNECTED:
      ros_destroy_entities();
      delay(100);
      state = WAITING_AGENT;
      // digitalWrite(GREEN_LED, LOW); 
      break;
    default:
      break;
  }
}

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
      digitalWrite(RED_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      // digitalWrite(GREEN_LED, HIGH);
      ros_handler();
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
  set_microros_serial_transports(Serial);
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
  digitalWrite(YELLOW_LED, LOW);

  state = WAITING_AGENT;

  ros_create_entities();
  // Turn off green to indicate ROS entities created
  digitalWrite(GREEN_LED, LOW);
}

void loop() {
  loop_time = millis();
 
  rcInput.read();
  
  set_motor_throttles();
  if (state == AGENT_CONNECTED) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
  }
}