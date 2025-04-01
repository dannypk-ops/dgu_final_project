// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from ros2_orb_slam3:msg/Stereoimage.idl
// generated code does not contain a copyright notice

#ifndef ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__STRUCT_HPP_
#define ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'left'
// Member 'right'
#include "sensor_msgs/msg/detail/image__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__ros2_orb_slam3__msg__Stereoimage __attribute__((deprecated))
#else
# define DEPRECATED__ros2_orb_slam3__msg__Stereoimage __declspec(deprecated)
#endif

namespace ros2_orb_slam3
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct Stereoimage_
{
  using Type = Stereoimage_<ContainerAllocator>;

  explicit Stereoimage_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : left(_init),
    right(_init)
  {
    (void)_init;
  }

  explicit Stereoimage_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : left(_alloc, _init),
    right(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _left_type =
    sensor_msgs::msg::Image_<ContainerAllocator>;
  _left_type left;
  using _right_type =
    sensor_msgs::msg::Image_<ContainerAllocator>;
  _right_type right;

  // setters for named parameter idiom
  Type & set__left(
    const sensor_msgs::msg::Image_<ContainerAllocator> & _arg)
  {
    this->left = _arg;
    return *this;
  }
  Type & set__right(
    const sensor_msgs::msg::Image_<ContainerAllocator> & _arg)
  {
    this->right = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator> *;
  using ConstRawPtr =
    const ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__ros2_orb_slam3__msg__Stereoimage
    std::shared_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__ros2_orb_slam3__msg__Stereoimage
    std::shared_ptr<ros2_orb_slam3::msg::Stereoimage_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const Stereoimage_ & other) const
  {
    if (this->left != other.left) {
      return false;
    }
    if (this->right != other.right) {
      return false;
    }
    return true;
  }
  bool operator!=(const Stereoimage_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct Stereoimage_

// alias to use template instance with default allocator
using Stereoimage =
  ros2_orb_slam3::msg::Stereoimage_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace ros2_orb_slam3

#endif  // ROS2_ORB_SLAM3__MSG__DETAIL__STEREOIMAGE__STRUCT_HPP_
