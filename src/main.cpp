#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <micro_ros_utilities/string_utilities.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/float32.h>

#include <MCP_POT.h>

#include <std_srvs/srv/set_bool.h>
#include <std_msgs/msg/string.h>

rcl_service_t service;

std_srvs__srv__SetBool_Response res;
std_srvs__srv__SetBool_Request req;

// Initialize two subscribers for port and starboard motors
rcl_subscription_t port_motor;
rcl_subscription_t stbd_motor;

std_msgs__msg__Float32 port_msg;
std_msgs__msg__Float32 stbd_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

MCP_POT pot(37, 9, 16, 11, 27);
uint32_t ww_m_en = 8;
uint32_t ww_thr_en = 7;

uint16_t port_throttle = 128;
uint16_t stbd_throttle = 128;

ulong loop_time = 0;
ulong last_time = 0;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void set_motor_throttles(){
  if (loop_time - last_time > 2000){
    port_throttle = 128;
    stbd_throttle = 128;
  }
  pot.setValue(0, port_throttle);
  pot.setValue(1, stbd_throttle);
}

void error_loop(){
  while(1){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}

int16_t throttle_convert(float input){
  input += 1.0f;
  return (static_cast<int16_t>(input *= 128));
}

void subscription_callback_port(const void * msgin)
{  
  // Loads the message for the port motor
  const std_msgs__msg__Float32 * msg = (const std_msgs__msg__Float32 *)msgin;
  float effort = msg->data;
  uint16_t throttle = throttle_convert(effort);

  // Simulate motor control (adjust this as per actual use case)
  port_throttle = throttle;
  last_time = millis();
}

void subscription_callback_stbd(const void * msgin)
{  
  // Loads the message for the starboard motor
  const std_msgs__msg__Float32 * msg = (const std_msgs__msg__Float32 *)msgin;
  float effort = msg->data;
  uint16_t throttle = throttle_convert(effort);

  // Simulate motor control (adjust this as per actual use case)
  stbd_throttle = throttle;
  last_time = millis();
} 

void service_callback(const void * req, void * res){
  std_srvs__srv__SetBool_Request * req_in = (std_srvs__srv__SetBool_Request *) req;
  std_srvs__srv__SetBool_Response * res_in = (std_srvs__srv__SetBool_Response *) res;
  
  bool arm = req_in->data;

  if(arm){
    pot.setValue(0, MCP_POT_MIDDLE_VALUE);
    delay(4000);
    digitalWrite(ww_m_en, HIGH);
    delay(3000);
    digitalWrite(ww_thr_en, HIGH);
    delay(1000);
    res_in->success = true; 
  } else {
    pot.setValue(0, MCP_POT_MIDDLE_VALUE);
    digitalWrite(ww_m_en, LOW);
    digitalWrite(ww_thr_en, LOW);
    res_in->success = true; 
  }  
}

void setup() {
  Serial.begin(115200);
  set_microros_serial_transports(Serial);
  delay(2000);

  SPI.begin();
  pot.begin();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  // Initialize ROS 2 support
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // Create node
  RCCHECK(rclc_node_init_default(&node, "autohelm_node", "", &support));

  // Create subscriber for port motor with topic /wamv/port_motor
  RCCHECK(rclc_subscription_init_default(
    &port_motor,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "/autohelm/port_motor"));

  // Create subscriber for starboard motor with topic /wamv/stbd_motor
  RCCHECK(rclc_subscription_init_default(
    &stbd_motor,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "/autohelm/stbd_motor"));

  RCCHECK(rclc_service_init_default(&service, &node, ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, SetBool), "/autohelm/arm"));

  // Create executor for handling both subscriptions
  RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));  // Allow 2 subscriptions
  RCCHECK(rclc_executor_add_service(&executor, &service, &req, &res, service_callback));
  RCCHECK(rclc_executor_add_subscription(&executor, &port_motor, &port_msg, &subscription_callback_port, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_subscription(&executor, &stbd_motor, &stbd_msg, &subscription_callback_stbd, ON_NEW_DATA));
}

void loop() {
  loop_time = millis();
  delay(100);
  set_motor_throttles();
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}