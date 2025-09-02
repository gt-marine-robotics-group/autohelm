#include <Arduino.h>
#include <micro_ros_platformio.h>
#include <micro_ros_utilities/string_utilities.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/float64.h>

#include <MCP_POT.h>
#include <RCInput.h>

// #include <rmw_microros/rmw_microros.h>
#include <std_srvs/srv/set_bool.h>
#include <std_msgs/msg/string.h>
#include <std_msgs/msg/bool.h>

#include <pins.h>

rcl_publisher_t estop_pub;

std_msgs__msg__Bool estop_msg;

rcl_timer_t timer;

rcl_service_t service;

std_srvs__srv__SetBool_Response res;
std_srvs__srv__SetBool_Request req;

// Initialize two subscribers for port and starboard motors
rcl_subscription_t port_motor;
rcl_subscription_t stbd_motor;

std_msgs__msg__Float64 port_msg;
std_msgs__msg__Float64 stbd_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;

MCP_POT pot(MCP_SEL, MCP_RST, MCP_SHDN, MCP_DOUT, MCP_CLK);

// RC Stuff

RCInput<SERVO_1, SERVO_2, SERVO_3, SERVO_4, SERVO_5> rcInput;


int cmd_srg;  // Commanded Surge
int cmd_swy;  // Commanded Sway
int cmd_yaw;  // Commanded Yaw
int cmd_ctr;  // Commanded Control State
int cmd_kil;  // Commanded Kill State


// throttle_0_en = stbd
// throttle_1_en = port

uint16_t port_throttle = 128;
uint16_t stbd_throttle = 128;

ulong loop_time = 0;
ulong last_time = 0;

enum states {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,
  AGENT_DISCONNECTED
} state;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

#define EXECUTE_EVERY_N_MS(MS, X) \
  do { \
    static volatile int64_t init = -1; \
    if (init == -1) { init = uxr_millis(); } \
    if (uxr_millis() - init > MS) { \
      X; \
      init = uxr_millis(); \
    } \
  } while (0)


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

// void error_loop(){
//   while(1){
//     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//     delay(100);
//   }
// }




int16_t throttle_convert(float input){
  input *= .9f; // -100 -> -90, 100 -> 90
  input += 100.0f; // -100 -> 0
  int16_t throttle = static_cast<int16_t>(input *= 1.28f); // scale to 0-to-256
  throttle = max(min(255, throttle), 0); // ensure within 0 to 255 range
  return throttle;
}

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

void timer_callback(rcl_timer_t * timer, int64_t last_call_time) {
  RCLC_UNUSED(last_call_time);
  bool estop_status = bool(digitalRead(SERVO_6));
  if (timer != NULL) {
    estop_msg.data = estop_status;
    RCSOFTCHECK(rcl_publish(&estop_pub, &estop_msg, NULL));
  }
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
  
  bool arm = req_in->data;

  if(arm){
    pot.setValue(0, MCP_POT_MIDDLE_VALUE);
    delay(100);
    digitalWrite(WW_M_EN, HIGH);
    delay(1000);
    res_in->success = true; 
  } else {
    pot.setValue(0, MCP_POT_MIDDLE_VALUE);
    digitalWrite(WW_M_EN, LOW);
    digitalWrite(WW_THR0_EN, LOW);
    digitalWrite(WW_THR1_EN, LOW);
    res_in->success = true; 
  }  
}

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
      digitalWrite(YELLOW_LED, HIGH);  
      break;
    case AGENT_AVAILABLE:
      created = ros_create_entities();
      state = (true == created) ? AGENT_CONNECTED : WAITING_AGENT;
      delay(100);
      digitalWrite(YELLOW_LED, LOW);
      if (state == WAITING_AGENT) {
        ros_destroy_entities();
        digitalWrite(GREEN_LED, HIGH);         
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
      digitalWrite(GREEN_LED, LOW); 
      break;
    default:
      break;
  }
}

// void read_rc() {
//   cmd_srg = orxElev.mapDeadzone(-100, 101, 0.05);
//   cmd_swy = orxAile.mapDeadzone(-100, 101, 0.05);
//   cmd_yaw = orxRudd.mapDeadzone(-100, 101, 0.05);
//   cmd_ctr = orxAux1.map(0, 2);
//   cmd_kil = 0;  //orxGear.map(1, 0);
//   char buffer[100];
  // sprintf(buffer, "RC | SRG: %4i  SWY: %4i  YAW: %4i CTR: %1i KIL: %1i",
          // cmd_srg, cmd_swy, cmd_yaw, cmd_ctr, cmd_kil);
  //Serial.println(buffer);
// }


// Translate RC input to 2 motor system
// void set_motor_2x() {
//   int a = (cmd_srg - cmd_yaw);
//   int d = (cmd_srg + cmd_yaw);
//   float max_val = max(100, max(abs(a), abs(d))) / 100;
//   rc_cmd_a = a / max_val;
//   rc_cmd_b = 0;
//   rc_cmd_c = 0;
//   rc_cmd_d = d / max_val;
// }

void exec_mode(int mode, bool killed) {
  // Vehicle Logic
  if (killed) {
    delay(1);
  } else {
    // if (mode == 0) {  // AUTONOMOUS
    //   ros_handler();
    //   motor_a.writeMicroseconds(throttleToESC(ros_cmd_a));
    //   motor_b.writeMicroseconds(throttleToESC(ros_cmd_b));
    //   motor_c.writeMicroseconds(throttleToESC(ros_cmd_c));
    //   motor_d.writeMicroseconds(throttleToESC(ros_cmd_d));
    //   motor_e.writeMicroseconds(throttleToESC(ros_cmd_e));
    //   motor_f.writeMicroseconds(throttleToESC(ros_cmd_f));
    //   delay(20);
    // } else if (mode == 1) {  // CALIBRATION
    //   calibrate_rc();
    //   // cfg_lt(0, 2, 0, 0);
    // } else if (mode == 2) {  // REMOTE CONTROL
    //   // set_motor_6x();
    //   // cfg_lt(0, 1, 0, 0);
    //   Serial.println("Throttle set");
    //   motor_a.writeMicroseconds(throttleToESC(rc_cmd_a));
    //   motor_b.writeMicroseconds(throttleToESC(rc_cmd_b));
    //   motor_c.writeMicroseconds(throttleToESC(rc_cmd_c));
    //   motor_d.writeMicroseconds(throttleToESC(rc_cmd_d));
    //   motor_e.writeMicroseconds(throttleToESC(rc_cmd_e));
    //   motor_f.writeMicroseconds(throttleToESC(rc_cmd_f));
    // } else {
    //   Serial.println("Error, mode not supported");
    // }
  }
}


void setup() {
  Serial.begin(115200);
  set_microros_serial_transports(Serial);
  delay(2000);

  SPI.begin();
  pot.begin();

  pot.setValue(0, MCP_POT_MIDDLE_VALUE);
  pot.setValue(1, MCP_POT_MIDDLE_VALUE);

  pinMode(SERVO_6, INPUT_PULLUP);
  
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);  
  pinMode(YELLOW_LED, OUTPUT);
  digitalWrite(YELLOW_LED, HIGH);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  
  delay(500);

  digitalWrite(RED_LED, LOW);  
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  state = WAITING_AGENT;

  // ros_create_entities();
}

void loop() {
  loop_time = millis();
  // estop_msg.data = true;
  // RCSOFTCHECK(rcl_publish(&estop_pub, &estop_msg, NULL));
 
  rcInput.read();

  ros_handler();
  set_motor_throttles();
  if (state == AGENT_CONNECTED) {
    rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
  }
}