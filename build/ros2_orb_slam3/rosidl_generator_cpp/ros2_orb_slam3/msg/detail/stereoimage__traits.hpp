// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from ros2_orb_slam3:msg/Stereoimage.idl
// generated code does not contain a copyright notice

#ifndef ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__TRAITS_HPP_
#define ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "ros2_orb_slam3/msg/detail/stereoimage__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'left'
// Member 'right'
#include "sensor_msgs/msg/detail/image__traits.hpp"

namespace ros2_orb_slam3
{

namespace msg
{

inline void to_flow_style_yaml(
  const Stereoimage & msg,
  std::ostream & out)
{
  out << "{";
  // member: left
  {
    out << "left: ";
    to_flow_style_yaml(msg.left, out);
    out << ", ";
  }

  // member: right
  {
    out << "right: ";
    to_flow_style_yaml(msg.right, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Stereoimage & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: left
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "left:\n";
    to_block_style_yaml(msg.left, out, indentation + 2);
  }

  // member: right
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "right:\n";
    to_block_style_yaml(msg.right, out, indentation + 2);
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Stereoimage & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace ros2_orb_slam3

namespace rosidl_generator_traits
{

[[deprecated("use ros2_orb_slam3::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const ros2_orb_slam3::msg::Stereoimage & msg,
  std::ostream & out, size_t indentation = 0)
{
  ros2_orb_slam3::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use ros2_orb_slam3::msg::to_yaml() instead")]]
inline std::string to_yaml(const ros2_orb_slam3::msg::Stereoimage & msg)
{
  return ros2_orb_slam3::msg::to_yaml(msg);
}

template<>
inline const char * data_type<ros2_orb_slam3::msg::Stereoimage>()
{
  return "ros2_orb_slam3::msg::Stereoimage";
}

template<>
inline const char * name<ros2_orb_slam3::msg::Stereoimage>()
{
  return "ros2_orb_slam3/msg/Stereoimage";
}

template<>
struct has_fixed_size<ros2_orb_slam3::msg::Stereoimage>
  : std::integral_constant<bool, has_fixed_size<sensor_msgs::msg::Image>::value> {};

template<>
struct has_bounded_size<ros2_orb_slam3::msg::Stereoimage>
  : std::integral_constant<bool, has_bounded_size<sensor_msgs::msg::Image>::value> {};

template<>
struct is_message<ros2_orb_slam3::msg::Stereoimage>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__TRAITS_HPP_
