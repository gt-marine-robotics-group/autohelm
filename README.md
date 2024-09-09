

## Quick Start

```
ros2 service call /srv_trigger std_srvs/srv/Trigger
```


```
# First you have to install micro_ros_agent and source ROS with agent
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyACM0 --baudrate 115200
```

```
# Publish ROS integer value for thrust
ros2 topic pub /wamv/port_motor std_msgs/msg/Int32 "data: 200" --once
ros2 topic pub /wamv/stbd_motor std_msgs/msg/Int32 "data: 200" --once
```





