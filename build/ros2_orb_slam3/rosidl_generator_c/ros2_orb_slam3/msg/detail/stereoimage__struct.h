// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from ros2_orb_slam3:msg/Stereoimage.idl
// generated code does not contain a copyright notice

#ifndef ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__STRUCT_H_
#define ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'left'
// Member 'right'
#include "sensor_msgs/msg/detail/image__struct.h"

/// Struct defined in msg/Stereoimage in the package ros2_orb_slam3.
typedef struct ros2_orb_slam3__msg__Stereoimage
{
  sensor_msgs__msg__Image left;
  sensor_msgs__msg__Image right;
} ros2_orb_slam3__msg__Stereoimage;

// Struct for a sequence of ros2_orb_slam3__msg__Stereoimage.
typedef struct ros2_orb_slam3__msg__Stereoimage__Sequence
{
  ros2_orb_slam3__msg__Stereoimage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} ros2_orb_slam3__msg__Stereoimage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__STRUCT_H_
