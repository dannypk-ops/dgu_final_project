// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from ros2_orb_slam3:msg/Stereoimage.idl
// generated code does not contain a copyright notice

#ifndef ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__BUILDER_HPP_
#define ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "ros2_orb_slam3/msg/detail/stereoimage__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace ros2_orb_slam3
{

namespace msg
{

namespace builder
{

class Init_Stereoimage_right
{
public:
  explicit Init_Stereoimage_right(::ros2_orb_slam3::msg::Stereoimage & msg)
  : msg_(msg)
  {}
  ::ros2_orb_slam3::msg::Stereoimage right(::ros2_orb_slam3::msg::Stereoimage::_right_type arg)
  {
    msg_.right = std::move(arg);
    return std::move(msg_);
  }

private:
  ::ros2_orb_slam3::msg::Stereoimage msg_;
};

class Init_Stereoimage_left
{
public:
  Init_Stereoimage_left()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Stereoimage_right left(::ros2_orb_slam3::msg::Stereoimage::_left_type arg)
  {
    msg_.left = std::move(arg);
    return Init_Stereoimage_right(msg_);
  }

private:
  ::ros2_orb_slam3::msg::Stereoimage msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::ros2_orb_slam3::msg::Stereoimage>()
{
  return ros2_orb_slam3::msg::builder::Init_Stereoimage_left();
}

}  // namespace ros2_orb_slam3

#endif  // ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__BUILDER_HPP_
