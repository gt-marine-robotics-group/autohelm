#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>

#include <MCP_POT.h>

rcl_subscription_t subscriber;
std_msgs__msg__Int32 msg;
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

void subscription_callback(const void * msgin)
{  
  // Loads in message and sets throttle to data
  const std_msgs__msg__Int32 * msg = (const std_msgs__msg__Int32 *)msgin;
  //digitalWrite(LED_BUILTIN, (msg->data == 0) ? LOW : HIGH);  
  uint16_t throttle = msg->data;

  // Setting potentiometer 0 to middle value
  pot.setValue(0, MCP_POT_MIDDLE_VALUE);
  delay(4000);

  // Setting wigwag motor enable (key) to on
  digitalWrite(ww_m_en, HIGH);
  delay(4000);

  // Setting throttle enable to on
  digitalWrite(ww_thr_en, HIGH);
  delay(1000);

  // Incrementing from neutral to throttle value
  // for (int i = 128; i < throttle; i++) {
  //   pot.setValue(0, i);
  //   delay(20);
  // }
  pot.setValue(0, throttle);
  delay(2000);
  // Decrementing from throttle to neutral value
  // for (int i = throttle; i > 128; i--) {
  //   pot.setValue(0, i);
  //   delay(20);
  // }
  delay(200);
  // Set throttle enable to off
  digitalWrite(ww_thr_en, LOW);

  // Set potentiometer 0 to middle value
  pot.setValue(0, MCP_POT_MIDDLE_VALUE);
  delay(2000);
  // Set motor enable to off
  digitalWrite(ww_m_en, LOW);
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

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "micro_ros_platformio_node", "", &support));

  // create subscriber
  RCCHECK(rclc_subscription_init_default(
    &subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "thrust_values"));

  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator)); // increment number for no. of subs/pubs
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriber, &msg, &subscription_callback, ON_NEW_DATA));
}

void loop() {
  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}