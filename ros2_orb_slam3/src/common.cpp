/*

A bare-bones example node demonstrating the use of the Monocular mode in ORB-SLAM3

Author: Azmyin Md. Kamal
Date: 01/01/24

REQUIREMENTS
* Make sure to set path to your workspace in common.hpp file

*/

//* Includes
#include "ros2_orb_slam3/common.hpp"
#include "rcpputils/filesystem_helper.hpp"  // 반드시 상단에 추가!
#include <vector>            // ✅ std::vector
#include <opencv2/core.hpp>  // ✅ cv::Mat (KeyFrame에서 imRGB 등 사용 시 필요)


//* Constructor
MonocularMode::MonocularMode() :Node("mono_node_cpp")
{
    // Declare parameters to be passsed from command line
    // https://roboticsbackend.com/rclcpp-params-tutorial-get-set-ros2-params-with-cpp/
    
    //* Find path to home directory
    homeDir = getenv("HOME");
    // std::cout<<"Home: "<<homeDir<<std::endl;
    
    // std::cout<<"VLSAM NODE STARTED\n\n";
    RCLCPP_INFO(this->get_logger(), "\nORB-SLAM3-V1 NODE STARTED");

    this->declare_parameter("node_name_arg", "not_given"); // Name of this agent 
    this->declare_parameter("voc_file_arg", "file_not_set"); // Needs to be overriden with appropriate name  
    this->declare_parameter("settings_file_path_arg", "file_path_not_set"); // path to settings file  
    
    //* Watchdog, populate default values
    nodeName = "not_set";
    vocFilePath = "file_not_set";
    settingsFilePath = "file_not_set";

    //* Populate parameter values
    rclcpp::Parameter param1 = this->get_parameter("node_name_arg");
    nodeName = param1.as_string();
    
    rclcpp::Parameter param2 = this->get_parameter("voc_file_arg");
    vocFilePath = param2.as_string();

    rclcpp::Parameter param3 = this->get_parameter("settings_file_path_arg");
    settingsFilePath = param3.as_string();

    // rclcpp::Parameter param4 = this->get_parameter("settings_file_name_arg");
    
  
    //* HARDCODED, set paths
    if (vocFilePath == "file_not_set" || settingsFilePath == "file_not_set")
    {
        pass;
        vocFilePath = homeDir + "/" + packagePath + "orb_slam3/Vocabulary/ORBvoc.txt.bin";
        settingsFilePath = homeDir + "/" + packagePath + "orb_slam3/config/Monocular/";
    }

    // std::cout<<"vocFilePath: "<<vocFilePath<<std::endl;
    // std::cout<<"settingsFilePath: "<<settingsFilePath<<std::endl;
    
    
    //* DEBUG print
    RCLCPP_INFO(this->get_logger(), "nodeName %s", nodeName.c_str());
    RCLCPP_INFO(this->get_logger(), "voc_file %s", vocFilePath.c_str());
    // RCLCPP_INFO(this->get_logger(), "settings_file_path %s", settingsFilePath.c_str());
    
    subexperimentconfigName = "/mono_py_driver/experiment_settings"; // topic that sends out some configuration parameters to the cpp ndoe
    pubconfigackName = "/mono_py_driver/exp_settings_ack"; // send an acknowledgement to the python node
    subImgMsgName = "/mono_py_driver/img_msg"; // topic to receive RGB image messages
    subTimestepMsgName = "/mono_py_driver/timestep_msg"; // topic to receive RGB image messages

    //* subscribe to python node to receive settings
    expConfig_subscription_ = this->create_subscription<std_msgs::msg::String>(subexperimentconfigName, 1, std::bind(&MonocularMode::experimentSetting_callback, this, _1));

    //* publisher to send out acknowledgement
    configAck_publisher_ = this->create_publisher<std_msgs::msg::String>(pubconfigackName, 10);

    //* subscrbite to the image messages coming from the Python driver node
    subImgMsg_subscription_= this->create_subscription<sensor_msgs::msg::Image>(subImgMsgName, 1, std::bind(&MonocularMode::Img_callback, this, _1));

    //* subscribe to receive the timestep
    subTimestepMsg_subscription_= this->create_subscription<std_msgs::msg::Float64>(subTimestepMsgName, 1, std::bind(&MonocularMode::Timestep_callback, this, _1));

    finish_subscription_ = this->create_subscription<std_msgs::msg::String>("/mono_py_driver/finished",10,std::bind(&MonocularMode::finish_callback, this, _1));

    init_colmap_publisher_ = this->create_publisher<std_msgs::msg::Bool>("/mono_py_driver/SLAM_done",10);
    
    RCLCPP_INFO(this->get_logger(), "Waiting to finish handshake ......");
    
}

//* Destructor
MonocularMode::~MonocularMode()
{   

    pAgent->Shutdown();
    pass;

}

//* Callback which accepts experiment parameters from the Python node
void MonocularMode::experimentSetting_callback(const std_msgs::msg::String& msg){
    
    // std::cout<<"experimentSetting_callback"<<std::endl;
    bSettingsFromPython = true;
    experimentConfig = msg.data.c_str();
    // receivedConfig = experimentConfig; // Redundant
    
    RCLCPP_INFO(this->get_logger(), "Configuration YAML file name: %s", this->receivedConfig.c_str());

    //* Publish acknowledgement
    auto message = std_msgs::msg::String();
    message.data = "ACK";
    
    std::cout<<"Sent response: "<<message.data.c_str()<<std::endl;
    configAck_publisher_->publish(message);

    //* Wait to complete VSLAM initialization
    initializeVSLAM(experimentConfig);

}

//* Method to bind an initialized VSLAM framework to this node
void MonocularMode::initializeVSLAM(std::string& configString) {
    // 필수 경로가 비어있는지 확인
    if (vocFilePath == "file_not_set" || settingsFilePath == "file_not_set") {
        RCLCPP_ERROR(this->get_logger(), "Please provide valid voc_file and settings_file paths");
        rclcpp::shutdown();
    }

    // .yaml 중복 방지 처리
    if (configString.size() < 5 || configString.substr(configString.size() - 5) != ".yaml") {
        settingsFilePath += configString + ".yaml";
    } else {
        settingsFilePath += configString;
    }

    RCLCPP_INFO(this->get_logger(), "Final settings file path: %s", settingsFilePath.c_str());

    // 파일 존재 여부 확인 (ROS 2 방식)
    if (!rcpputils::fs::exists(settingsFilePath)) {
        RCLCPP_ERROR(this->get_logger(), "Settings file does NOT exist: %s", settingsFilePath.c_str());
        rclcpp::shutdown();
    }

    // ORB-SLAM3 시스템 초기화
    sensorType = ORB_SLAM3::System::MONOCULAR;
    enablePangolinWindow = true;
    enableOpenCVWindow = true;

    pAgent = new ORB_SLAM3::System(vocFilePath, settingsFilePath, sensorType, enablePangolinWindow);

    RCLCPP_INFO(this->get_logger(), "MonocularMode node initialized successfully.");
}

//* Callback that processes timestep sent over ROS
void MonocularMode::Timestep_callback(const std_msgs::msg::Float64& time_msg){
    // timeStep = 0; // Initialize
    timeStep = time_msg.data;
}

//* Callback to process image message and run SLAM node
void MonocularMode::Img_callback(const sensor_msgs::msg::Image& msg)
{
    // Initialize
    cv_bridge::CvImagePtr cv_ptr; //* Does not create a copy, memory efficient
    
    //* Convert ROS image to openCV image
    try
    {
        //cv::Mat im =  cv_bridge::toCvShare(msg.img, msg)->image;
        cv_ptr = cv_bridge::toCvCopy(msg); // Local scope
        
        // DEBUGGING, Show image
        // Update GUI Window
        // cv::imshow("test_window", cv_ptr->image);
        // cv::waitKey(3);
    }
    catch (cv_bridge::Exception& e)
    {
        RCLCPP_ERROR(this->get_logger(),"Error reading image");
        return;
    }
    
    // std::cout<<std::fixed<<"Timestep: "<<timeStep<<std::endl; // Debug
    
    //* Perform all ORB-SLAM3 operations in Monocular mode
    //! Pose with respect to the camera coordinate frame not the world coordinate frame
    Sophus::SE3f Tcw = pAgent->TrackMonocular(cv_ptr->image, timeStep); 

    //* An example of what can be done after the pose w.r.t camera coordinate frame is computed by ORB SLAM3
    //Sophus::SE3f Twc = Tcw.inverse(); //* Pose with respect to global image coordinate, reserved for future use
    
}

void MonocularMode::finish_callback(const std_msgs::msg::String& msg) {
    if (msg.data == "done" && !hasPublished) {
        RCLCPP_INFO(this->get_logger(), "📩 Received 'done' signal from Python. Publishing trajectory...");

        // std::string traj_file = homeDir + "/trajectory_output/trajectory.txt";
        std::string traj_file = "/home/jk/ros2_test/src/ros2_orb_slam3/colmap_output/SLAM_pose/trajectory.txt";
        std::string traj_dir = "/home/jk/ros2_test/src/ros2_orb_slam3/colmap_output/SLAM_pose/";
        rcpputils::fs::path dir_path(traj_dir);

        if (!rcpputils::fs::exists(dir_path)) {
            rcpputils::fs::create_directories(dir_path);
        }

        pAgent->SaveTrajectoryEuRoC(traj_file);
        hasPublished = true;

        RCLCPP_INFO(this->get_logger(), "✅ Final trajectory published.");
    }
    
    std::vector<cv::Mat> images = pAgent->GetAllKeyFrameImages();

    std::string save_dir = "/home/jk/ros2_test/src/ros2_orb_slam3/colmap_output/SLAM_images";
    rcpputils::fs::path dir_path(save_dir);

    if (!rcpputils::fs::exists(dir_path)) {
        rcpputils::fs::create_directories(dir_path);
    }

    for (size_t i = 0; i < images.size(); ++i)
    {
        const cv::Mat& image = images[i];
        if (image.empty()) continue;

        std::ostringstream oss;
        oss << save_dir << "/keyframe_" << std::setw(4) << std::setfill('0') << i << ".png";
        std::string filename = oss.str();

        if (cv::imwrite(filename, image)) {
            std::cout << "✅ Saved: " << filename << std::endl;
        } else {
            std::cerr << "❌ Failed to save: " << filename << std::endl;
        }
    }

    // 콜맵 실행 신호 전달
    auto slam_done_msg = std_msgs::msg::Bool();
    slam_done_msg.data = true;
    init_colmap_publisher_->publish(slam_done_msg);
}