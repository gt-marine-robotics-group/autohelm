

## Quick Start

```
ros2 service call /autohelm/arm std_srvs/srv/SetBool "{data: true}"
```


```
# First you have to install micro_ros_agent and source ROS with agent
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 --baudrate 115200
```

```
# Publish ROS integer value for thrust
ros2 topic pub /autohelm/port_motor std_msgs/msg/Float32 "data: 0.8" --once
ros2 topic pub /autohelm/stbd_motor std_msgs/msg/Int32 "data: 200" --once
```





