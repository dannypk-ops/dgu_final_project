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

void printSE3f(const Sophus::SE3f& Twc) {
    // rotation matrix (3x3)
    Eigen::Matrix3f R = Twc.rotationMatrix();
    // translation vector (3x1)
    Eigen::Vector3f t = Twc.translation();

    // rotation 출력
    printf("Rotation matrix:\n");
    for (int i = 0; i < 3; ++i) {
        printf("[%.6f %.6f %.6f]\n", R(i,0), R(i,1), R(i,2));
    }

    // translation 출력
    printf("Translation vector:\n");
    printf("[%.6f %.6f %.6f]\n", t(0), t(1), t(2));
}

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
    currentMode = "SLAM";  // 기본값은 SLAM
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
    localizationMode = "/mono_py_driver/localization_msg";

    //* subscribe to python node to receive settings
    expConfig_subscription_ = this->create_subscription<std_msgs::msg::String>(subexperimentconfigName, 1, std::bind(&MonocularMode::experimentSetting_callback, this, _1));

    //* publisher to send out acknowledgement
    configAck_publisher_ = this->create_publisher<std_msgs::msg::String>(pubconfigackName, 10);

    //* subscrbite to the image messages coming from the Python driver node
    subImgMsg_subscription_= this->create_subscription<sensor_msgs::msg::Image>(subImgMsgName, 1, std::bind(&MonocularMode::Img_callback, this, _1));

    //* subscribe to receive the timestep
    subTimestepMsg_subscription_= this->create_subscription<std_msgs::msg::Float64>(subTimestepMsgName, 1, std::bind(&MonocularMode::Timestep_callback, this, _1));

    finish_subscription_ = this->create_subscription<std_msgs::msg::String>("/mono_py_driver/finished",10,std::bind(&MonocularMode::finish_callback, this, _1));

    init_colmap_publisher_ = this->create_publisher<std_msgs::msg::String>("/mono_py_driver/SLAM_done",10);

    localizationMode_subscription_ = this->create_subscription<std_msgs::msg::String>(localizationMode, 10, std::bind(&MonocularMode::LocalizationMode_callback, this, _1));
    
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
    // printf("state : %d\n",pAgent->GetTrackingState());

    //* An example of what can be done after the pose w.r.t camera coordinate frame is computed by ORB SLAM3
    // if (this->currentMode == "Localization"){
    //     Sophus::SE3f Twc = Tcw.inverse(); //* Pose with respect to global image coordinate, reserved for future use
    //     printSE3f(Twc);
    // }
}

// void MonocularMode::saving_image_path(const std_msgs::msg::String& msg){

// }

void MonocularMode::finish_callback(const std_msgs::msg::String& msg) {

    const std::string iter = "iteration" + std::to_string(this->image_processing_time);

    // 2) 필요한 경로들
    std::string base_dir = this->kRoot + iter;                               // .../iterationN
    std::string traj_dir = base_dir + "/SLAM_pose";                    // .../iterationN/SLAM_pose
    std::string traj_file = traj_dir + "/trajectory.txt";              // .../SLAM_pose/trajectory.txt
    std::string save_dir  = base_dir + "/SLAM_images";                 // .../iterationN/SLAM_images

    if (!rcpputils::fs::exists(base_dir)) {
        rcpputils::fs::create_directories(base_dir);
        rcpputils::fs::create_directories(traj_dir);
        rcpputils::fs::create_directories(save_dir);
    }

    if (msg.data == "done") {
        RCLCPP_INFO(this->get_logger(), "📩 Received 'done' signal from Python. Publishing trajectory...");

        // SaveTrajectory의 함수로 현재 Kerframe ID가 없는, 새로운 이미지에 대한 pose만을 저장하도록 수정.
        if (this->image_processing_time == 0)
            pAgent->SaveTrajectoryEuRoC(traj_file);
        else
            pAgent->SaveTrajectoryEuRoC(traj_file, this->keyframe_index);

        RCLCPP_INFO(this->get_logger(), "✅ Final trajectory published.");
    }
    
    auto kf_data = pAgent->GetAllKeyFrameData();

    // keyframe_index는 클래스 멤버: std::vector<std::size_t> keyframe_index;
    std::unordered_set<std::size_t> seen_ids(this->keyframe_index.begin(),
                                            this->keyframe_index.end());

    int idx = 0;
    for (const auto& [kf_id, data] : kf_data)           // 모든 KeyFrame 순회
    {
        // ── 1) 중복 검사 ──────────────────────────────────────────────
        if (seen_ids.count(kf_id)) {
            continue;                                   // 이미 처리한 ID면 건너뜀
        }

        const cv::Mat& img = data.image;
        if (img.empty()) continue;

        // ── 2) 파일명 생성 ────────────────────────────────────────────
        std::ostringstream oss;
        oss << save_dir << "/keyframe_"
            << std::setw(4) << std::setfill('0') << idx++   // 0000, 0001, …
            << "_id" << kf_id << ".png";

        const std::string filename = oss.str();

        // ── 3) 저장 & 인덱스 갱신 ─────────────────────────────────────
        if (cv::imwrite(filename, img)) {
            std::cout << "✅ Saved: " << filename << '\n';
            this->keyframe_index.push_back(kf_id);  // 벡터에도 기록
        } else {
            std::cerr << "❌ Failed to save: " << filename << '\n';
        }
    }

    // 콜맵 실행 신호 전달
    std_msgs::msg::String slam_done_msg;
    slam_done_msg.data = base_dir;
    init_colmap_publisher_->publish(slam_done_msg);

    this->image_processing_time += 1;
}

void MonocularMode::LocalizationMode_callback(const std_msgs::msg::String& msg) {
    std::string requestedMode = msg.data;

    if (requestedMode == currentMode) {
        RCLCPP_INFO(this->get_logger(), "🔄 Already in %s Mode. No action taken.", currentMode.c_str());
        return;
    }

    if (requestedMode == "Localization") {
        RCLCPP_INFO(this->get_logger(), "🟢 New environment input start...");
        // pAgent->ActivateLocalizationMode();
        currentMode = "Localization";
    } 
    else if (requestedMode == "SLAM") {
        RCLCPP_INFO(this->get_logger(), "🟢 Switching to SLAM Mode");
        // pAgent->DeactivateLocalizationMode();
        currentMode = "SLAM";
    } 
    else {
        RCLCPP_WARN(this->get_logger(), "⚠️ Unknown mode received: %s", requestedMode.c_str());
    }
}
