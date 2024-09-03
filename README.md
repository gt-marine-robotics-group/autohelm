

## Quick Start

```
# First you have to install micro_ros_agent and source ROS with agent
ros2 run micro_ros_agent micro_ros_agent serial --serial_port /dev/ttyACM0 --baudrate 115200
```

```
# Publish ROS integer value for thrust
ros2 topic pub /thrust_values std_msgs/msg/Int32 "data: 200" --once
```





