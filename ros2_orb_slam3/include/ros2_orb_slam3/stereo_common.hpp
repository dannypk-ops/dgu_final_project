// Include file 
#ifndef COMMON_HPP  // Header guard to prevent multiple inclusions
#define COMMON_HPP

// C++ includes
#include <iostream> // The iostream library is an object-oriented library that provides input and output functionality using streams
#include <algorithm> // The header <algorithm> defines a collection of functions especially designed to be used on ranges of elements.
#include <fstream> // Input/output stream class to operate on files.
#include <chrono> // c++ timekeeper library
#include <vector> // vectors are sequence containers representing arrays that can change in size.
#include <queue>
#include <thread> // class to represent individual threads of execution.
#include <mutex> // A mutex is a lockable object that is designed to signal when critical sections of code need exclusive access, preventing other threads with the same protection from executing concurrently and access the same memory locations.
#include <cstdlib> // to find home directory

#include <cstring>
#include <sstream> // String stream processing functionalities

//* ROS2 includes
//* std_msgs in ROS 2 https://docs.ros2.org/foxy/api/std_msgs/index-msg.html
#include "rclcpp/rclcpp.hpp"

// #include "your_custom_msg_interface/msg/custom_msg_field.hpp" // Example of adding in a custom message
#include <std_msgs/msg/header.hpp>
#include "std_msgs/msg/float64.hpp"
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/bool.hpp>
#include "sensor_msgs/msg/image.hpp"
using std::placeholders::_1; //* TODO why this is suggested in official tutorial

#include <std_msgs/msg/float64_multi_array.hpp>      // ✅ Float64MultiArray
#include <geometry_msgs/msg/pose_stamped.hpp>        // ✅ PoseStamped


// Include Eigen
// Quick reference: https://eigen.tuxfamily.org/dox/group__QuickRefPage.html
#include <Eigen/Dense> // Includes Core, Geometry, LU, Cholesky, SVD, QR, and Eigenvalues header file

// Include cv-bridge
#include <cv_bridge/cv_bridge.h>

// Include OpenCV computer vision library
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp> // Image processing tools
#include <opencv2/highgui/highgui.hpp> // GUI tools
#include <opencv2/core/eigen.hpp>
#include <image_transport/image_transport.h>

//* ORB SLAM 3 includes
#include "System.h" //* Also imports the ORB_SLAM3 namespace

//* Gobal defs
#define pass (void)0 // Python's equivalent of "pass" i.e. no operation


//* Node specific definitions
class StereoNode : public rclcpp::Node
{   
    //* This slam node inherits from both rclcpp and ORB_SLAM3::System classes
    //* public keyword needs to come before the class constructor and anything else
    public:
    std::string experimentConfig = ""; // String to receive settings sent by the python driver
    double timeStep; // Timestep data received from the python node
    std::string receivedConfig = "";
    int image_processing_time = 0;


    int cnt = 0;

    //* Class constructor
    StereoNode(); // Constructor 

    ~StereoNode(); // Destructor
        
    private:

        cv::Mat             last_left_;
        cv::Mat             last_right_;
        rclcpp::Time        last_stamp_;
        std::mutex          mtx_;

        bool hasPublished = false; 
        
        // Class internal variables
        std::string homeDir = "";
        std::string packagePath = "ros2_test/src/ros2_orb_slam3/"; //! Change to match path to your workspace
        std::string OPENCV_WINDOW = ""; // Set during initialization
        std::string nodeName = ""; // Name of this node
        std::string vocFilePath = ""; // Path to ORB vocabulary provided by DBoW2 package
        std::string settingsFilePath = ""; // Path to settings file provided by ORB_SLAM3 package
        bool bSettingsFromPython = false; // Flag set once when experiment setting from python node is received
        
        std::string subexperimentconfigName = ""; // Subscription topic name
        std::string pubconfigackName = ""; // Publisher topic name
        // std::string subImgMsgName = ""; // Topic to subscribe to receive RGB images from a python node

        std::string sub_left = "";
        std::string sub_right = "";

        std::string subTimestepMsgName = ""; // Topic to subscribe to receive the timestep related to the 
        std::string localizationMode = "";   // Localization 모드를 시작하기 위한 msg
        
        // 인덱스 관리 
        std::vector<int> keyframe_index;  
        std::unordered_set<std::size_t> removed_index;

        // source path
        std::string kRoot = "/home/jk/ros2_test/src/ros2_orb_slam3/colmap_output/";

        //* Definitions of publisher and subscribers
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr configAck_publisher_;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr expConfig_subscription_;
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subImgMsg_subscription_;
        rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr subTimestepMsg_subscription_;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr finish_subscription_;
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr init_colmap_publisher_;
        rclcpp::Subscription<std_msgs::msg::String>::SharedPtr localizationMode_subscription_;

        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_left_ImgMsg;
        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr sub_right_ImgMsg;

        //* ORB_SLAM3 related variables
        ORB_SLAM3::System* pAgent; // pointer to a ORB SLAM3 object
        ORB_SLAM3::System::eSensor sensorType;
        bool enablePangolinWindow = false; // Shows Pangolin window output
        bool enableOpenCVWindow = false; // Shows OpenCV window output
        std::string currentMode;

        //* ROS callbacks
        void experimentSetting_callback(const std_msgs::msg::String& msg); // Callback to process settings sent over by Python node
        void Timestep_callback(const std_msgs::msg::Float64& time_msg); // Callback to process the timestep for this image
        void Img_callback(const sensor_msgs::msg::Image& msg); // Callback to process RGB image and semantic matrix sent by Python node
        
        //* Helper functions
        // ORB_SLAM3::eigenMatXf convertToEigenMat(const std_msgs::msg::Float32MultiArray& msg); // Helper method, converts semantic matrix eigenMatXf, a Eigen 4x4 float matrix
        void initializeVSLAM(std::string& configString); //* Method to bind an initialized VSLAM framework to this node
        void LocalizationMode_callback(const std_msgs::msg::String& msg);
        void finish_callback(const std_msgs::msg::String& msg);

        void left_cb (const sensor_msgs::msg::Image::ConstSharedPtr & msg);
        void right_cb(const sensor_msgs::msg::Image::ConstSharedPtr & msg);
        void try_process();

};

#endif