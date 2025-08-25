# Autohelm Control Firmware


## Development

1. Install PlatformIO


## Quick Start Commands

```
ros2 service call /autohelm/arm std_srvs/srv/SetBool "{data: true}"
```

```
# First you have to install micro_ros_agent and source ROS with agent
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 --baudrate 115200
```

```
# Publish ROS integer value for thrust
ros2 topic pub /autohelm/port_motor std_msgs/msg/Float64 "data: 0.8" --once
```





