#pragma once

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/bool.h>
#include <std_msgs/msg/float64.h>
#include <std_msgs/msg/string.h>
#include <std_srvs/srv/set_bool.h>

#include "globals.h"
#include "motors.h"

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

extern rcl_publisher_t estop_pub;

extern std_msgs__msg__Bool estop_msg;

extern rcl_timer_t timer;

extern rcl_service_t service;

extern std_srvs__srv__SetBool_Response res;
extern std_srvs__srv__SetBool_Request req;

extern rcl_subscription_t port_motor;
extern rcl_subscription_t stbd_motor;

extern std_msgs__msg__Float64 port_msg;
extern std_msgs__msg__Float64 stbd_msg;

extern rclc_executor_t executor;
extern rclc_support_t support;
extern rcl_allocator_t allocator;
extern rcl_node_t node;

extern void subscription_callback_port(const void * msgin);
extern void subscription_callback_stbd(const void * msgin);
extern void service_callback(const void * req, void * res);
extern void timer_callback(rcl_timer_t * timer, int64_t last_call_time);