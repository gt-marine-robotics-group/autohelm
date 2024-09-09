#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#include <MCP_POT.h>

#include <std_srvs/srv/trigger.h>

rcl_service_t service;

std_srvs__srv__Trigger_Response res;
std_srvs__srv__Trigger_Request req;

// Initialize two subscribers for port and starboard motors
rcl_subscription_t port_motor;
rcl_subscription_t stbd_motor;

std_msgs__msg__Int32 port_msg;
std_msgs__msg__Int32 stbd_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

MCP_POT pot(37, 9, 16, 11, 27);
uint32_t ww_m_en = 8;
uint32_t ww_thr_en = 7;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

void error_loop(){
  while(1){
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(100);
  }
}

void subscription_callback_port(const void * msgin)
{  
  // Loads the message for the port motor
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  uint16_t throttle = msg->data;

  // Simulate motor control (adjust this as per actual use case)
  pot.setValue(0, throttle);
  delay(2000);
}

void subscription_callback_stbd(const void * msgin)
{  
  // Loads the message for the starboard motor
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  uint16_t throttle = msg->data;

  // Simulate motor control (adjust this as per actual use case)
  pot.setValue(0, throttle);
  delay(2000);
}

void service_callback(const void * req, void * res){
  std_srvs__srv__Trigger_Request * req_in = (std_srvs__srv__Trigger_Request *) req;
  std_srvs__srv__Trigger_Response * res_in = (std_srvs__srv__Trigger_Response *) res;
  
  res_in->success = true; 
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
  RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

  // Create subscriber for port motor with topic /wamv/port_motor
  RCCHECK(rclc_subscription_init_default(
    &port_motor,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "/wamv/port_motor"));

  // Create subscriber for starboard motor with topic /wamv/stbd_motor
  RCCHECK(rclc_subscription_init_default(
    &stbd_motor,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "/wamv/stbd_motor"));

  RCCHECK(rclc_service_init_default(&service, &node, ROSIDL_GET_SRV_TYPE_SUPPORT(std_srvs, srv, Trigger), "srv_trigger"));

  // Create executor for handling both subscriptions
  RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));  // Allow 2 subscriptions
  RCCHECK(rclc_executor_add_service(&executor, &service, &req, &res, service_callback));
  RCCHECK(rclc_executor_add_subscription(&executor, &port_motor, &port_msg, &subscription_callback_port, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_subscription(&executor, &stbd_motor, &stbd_msg, &subscription_callback_stbd, ON_NEW_DATA));
}

void loop() {
  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
