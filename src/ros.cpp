#include "ros.h"

rcl_publisher_t estop_pub;

std_msgs__msg__Bool estop_msg;

rcl_timer_t timer;

rcl_service_t service;

std_srvs__srv__SetBool_Response res;
std_srvs__srv__SetBool_Request req;

rcl_subscription_t port_motor;
rcl_subscription_t stbd_motor;

std_msgs__msg__Float64 port_msg;
std_msgs__msg__Float64 stbd_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;


// Functions

void subscription_callback_port(const void * msgin)
{  
  // Loads the message for the port motor
  const std_msgs__msg__Float64 * msg = (const std_msgs__msg__Float64 *)msgin;
  float effort = msg->data;
  uint16_t throttle = throttle_convert(effort);

  // Simulate motor control (adjust this as per actual use case)
  port_throttle = throttle;
  last_time = millis();
}

void subscription_callback_stbd(const void * msgin)
{  
  // Loads the message for the starboard motor
  const std_msgs__msg__Float64 * msg = (const std_msgs__msg__Float64 *)msgin;
  float effort = msg->data;
  uint16_t throttle = throttle_convert(effort);

  // Simulate motor control (adjust this as per actual use case)
  stbd_throttle = throttle;
  last_time = millis();
} 

void service_callback(const void * req, void * res){
  std_srvs__srv__SetBool_Request * req_in = (std_srvs__srv__SetBool_Request *) req;
  std_srvs__srv__SetBool_Response * res_in = (std_srvs__srv__SetBool_Response *) res;
  
  res_in->success = set_arm(req_in->data);
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  bool estop_status = bool(digitalRead(SERVO_6));
  if (timer != NULL) {
    estop_msg.data = estop_status;
    RCSOFTCHECK(rcl_publish(&estop_pub, &estop_msg, NULL));
  }
}
