/*

A bare-bones example node demonstrating the use of the Monocular mode in ORB-SLAM3

Author: Azmyin Md. Kamal
Date: 01/01/24

REQUIREMENTS
* Make sure to set path to your workspace in common.hpp file

*/

//* Includes
#include "ros2_orb_slam3/stereo_common.hpp"
#include "rcpputils/filesystem_helper.hpp"  // 반드시 상단에 추가!
#include <vector>            // ✅ std::vector
#include <opencv2/core.hpp>  // ✅ cv::Mat (KeyFrame에서 imRGB 등 사용 시 필요)
using ORB_SLAM3::KeyFrame;

//* Constructor
StereoNode::StereoNode() :Node("stereo_node_cpp")
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
        settingsFilePath = homeDir + "/" + packagePath + "orb_slam3/config/Stereo/";
    }

    // std::cout<<"vocFilePath: "<<vocFilePath<<std::endl;
    // std::cout<<"settingsFilePath: "<<settingsFilePath<<std::endl;
    
    
    //* DEBUG print
    RCLCPP_INFO(this->get_logger(), "nodeName %s", nodeName.c_str());
    RCLCPP_INFO(this->get_logger(), "voc_file %s", vocFilePath.c_str());
    // RCLCPP_INFO(this->get_logger(), "settings_file_path %s", settingsFilePath.c_str());
    
    subexperimentconfigName = "/stereo_py_driver/experiment_settings"; // topic that sends out some configuration parameters to the cpp ndoe
    pubconfigackName = "/stereo_py_driver/exp_settings_ack"; // send an acknowledgement to the python node
    // subImgMsgName = "/stereo_py_driver/img_msg"; // topic to receive RGB image messages

    sub_left  = "/stereo_py_driver/left";
    sub_right = "/stereo_py_driver/right";

    subTimestepMsgName = "/stereo_py_driver/timestep_msg"; // topic to receive RGB image messages
    localizationMode = "/stereo_py_driver/localization_msg";

    //* subscribe to python node to receive settings
    expConfig_subscription_ = this->create_subscription<std_msgs::msg::String>(subexperimentconfigName, 1, std::bind(&StereoNode::experimentSetting_callback, this, _1));

    //* publisher to send out acknowledgement
    configAck_publisher_ = this->create_publisher<std_msgs::msg::String>(pubconfigackName, 10);

    //* subscrbite to the image messages coming from the Python driver node
    sub_left_ImgMsg = this->create_subscription<sensor_msgs::msg::Image>(sub_left, 1, std::bind(&StereoNode::left_cb, this, _1));
    sub_right_ImgMsg = this->create_subscription<sensor_msgs::msg::Image>(sub_right, 1, std::bind(&StereoNode::right_cb, this, _1));

    //* subscribe to receive the timestep
    subTimestepMsg_subscription_= this->create_subscription<std_msgs::msg::Float64>(subTimestepMsgName, 1, std::bind(&StereoNode::Timestep_callback, this, _1));

    finish_subscription_ = this->create_subscription<std_msgs::msg::String>("/stereo_py_driver/finished",10,std::bind(&StereoNode::finish_callback, this, _1));

    init_colmap_publisher_ = this->create_publisher<std_msgs::msg::String>("/stereo_py_driver/SLAM_done",10);

    localizationMode_subscription_ = this->create_subscription<std_msgs::msg::String>(localizationMode, 10, std::bind(&StereoNode::LocalizationMode_callback, this, _1));
    
    RCLCPP_INFO(this->get_logger(), "Waiting to finish handshake ......");
    
}

//* Destructor
StereoNode::~StereoNode()
{   

    pAgent->Shutdown();
    pass;

}

//* Callback which accepts experiment parameters from the Python node
void StereoNode::experimentSetting_callback(const std_msgs::msg::String& msg){
    
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
void StereoNode::initializeVSLAM(std::string& configString) {
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
    sensorType = ORB_SLAM3::System::STEREO;
    enablePangolinWindow = true;
    enableOpenCVWindow = true;

    pAgent = new ORB_SLAM3::System(vocFilePath, settingsFilePath, sensorType, enablePangolinWindow);

    RCLCPP_INFO(this->get_logger(), "StereoNode node initialized successfully.");
}

//* Callback that processes timestep sent over ROS
void StereoNode::Timestep_callback(const std_msgs::msg::Float64& time_msg){
    // timeStep = 0; // Initialize
    timeStep = time_msg.data;
}

void StereoNode::left_cb(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
    std::lock_guard<std::mutex> lk(mtx_);
    last_left_  = cv_bridge::toCvCopy(msg, "rgb8")->image;
    last_stamp_ = msg->header.stamp;
    try_process();
}

void StereoNode::right_cb(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
{
    std::lock_guard<std::mutex> lk(mtx_);
    last_right_ = cv_bridge::toCvCopy(msg, "rgb8")->image;
    try_process();
}

void StereoNode::try_process(){
    if(!last_left_.empty() && !last_right_.empty()){
        pAgent->TrackStereo(last_left_, last_right_, rclcpp::Time(last_stamp_).seconds());
        last_left_.release();
        last_right_.release();
    }
}

void StereoNode::finish_callback(const std_msgs::msg::String& msg)
{
    /* ------------ 0. 경로 설정 ------------ */
    const std::string iter = "iteration" + std::to_string(this->image_processing_time);
    std::string base_dir = kRoot + iter,
                traj_dir = base_dir + "/SLAM_pose",
                traj_file = traj_dir + "/trajectory.txt",
                save_dir  = base_dir + "/SLAM_images";

    if (!rcpputils::fs::exists(base_dir)) {
        rcpputils::fs::create_directories(traj_dir);
        rcpputils::fs::create_directories(save_dir);
    }

    /* ------------ 1. Trajectory 저장 ------------ */
    if (msg.data == "done") {
        if (image_processing_time == 0)
            pAgent->SaveTrajectoryEuRoC(traj_file);
        else
            pAgent->SaveTrajectoryEuRoC(traj_file, keyframe_index);
    }

    /* ------------ 2. 이미지 저장 ------------ */
    std::vector<ORB_SLAM3::KeyFrame*> vpKFs = pAgent->GetKeyFrames();
    std::unordered_set<std::size_t> seen_ids(keyframe_index.begin(),
                                             keyframe_index.end());
    std::unordered_set<std::size_t> remove_ids;
    std::unordered_set<std::size_t> saved_ids;      // 이번 함수에서 이미 저장한 ID

    /* 2‑A) remove_ids 수집 : 새 KF ↔ 기존(seen) KF */
    for (ORB_SLAM3::KeyFrame* pKF : vpKFs) {
        if (!pKF || pKF->isBad() || pKF->imRGB.empty()) continue;
        if (seen_ids.count(pKF->mnId))                  continue;   // 새 KF

        /* (1) weight 내림차순 리스트 */
        const auto& vCov = pKF->GetCovisiblesByWeight(0);
        if (vCov.size() < 2) continue;                              // 이웃이 0·1개면 컷 불필요

        /* (2) weight 배열 추출 */
        std::vector<int> w;
        w.reserve(vCov.size());
        for (auto* n : vCov) w.push_back(pKF->GetWeight(n));

        /* (3) 최대 갭 위치 찾기 */
        int cut_idx = 0;                // keep vCov[0 .. cut_idx] inclusive
        int max_gap = -1;
        for (std::size_t i = 0; i + 1 < w.size(); ++i) {
            int gap = w[i] - w[i + 1];
            if (gap > max_gap) { max_gap = gap; cut_idx = static_cast<int>(i); }
        }

        /* (4) cut_idx 이전 이웃만 remove_ids 후보 */
        for (int i = 0; i <= cut_idx; ++i) {
            ORB_SLAM3::KeyFrame* pNeigh = vCov[i];
            if (!pNeigh || pNeigh->isBad() || pNeigh->imRGB.empty()) continue;
            if (seen_ids.count(pNeigh->mnId)) {
                remove_ids.insert(pNeigh->mnId);
                // std::cout << "neigh="   << pNeigh->mnId
                //         << " weight=" << w[i]
                //         << " gap="    << max_gap
                //         << '\n';
            }
        }
    }

    /* 2‑B) 이미지 저장 */
    auto kf_data = pAgent->GetAllKeyFrameData();
    int idx = 0;

    for (ORB_SLAM3::KeyFrame* pKF : vpKFs) {
        if (!pKF || pKF->isBad() || pKF->imRGB.empty()) continue;
        std::size_t id = pKF->mnId;

        if (remove_ids.count(id) || saved_ids.count(id)) continue;  // 중복·제외
        auto dit = kf_data.find(id);
        if (dit == kf_data.end() || dit->second.image.empty()) continue;

        std::ostringstream oss;
        oss << save_dir << "/keyframe_"
            << std::setw(4) << std::setfill('0') << idx++
            << "_id" << id << ".png";

        if (cv::imwrite(oss.str(), dit->second.image)) {
            std::cout << "✅ Saved: " << oss.str() << '\n';
            keyframe_index.push_back(id);   // 다음 회차부터 seen
            saved_ids.insert(id);
        }
    }

    // 다음 image stream을 위하여 삭제된 keyframe index 유지.
    // keyframe_index 최신화
    this->removed_index = remove_ids;
    this->keyframe_index.erase(
        std::remove_if(
            this->keyframe_index.begin(), this->keyframe_index.end(),
            [&](int id){
                return remove_ids.count(static_cast<std::size_t>(id)) > 0;
            }),
        this->keyframe_index.end());


    /* ------------ 3. COLMAP 트리거 ------------ */
    std_msgs::msg::String slam_done_msg;
    slam_done_msg.data = base_dir;
    init_colmap_publisher_->publish(slam_done_msg);

    ++image_processing_time;
}



void StereoNode::LocalizationMode_callback(const std_msgs::msg::String& msg) {
    std::string requestedMode = msg.data;

    if (requestedMode == currentMode) {
        RCLCPP_INFO(this->get_logger(), "🔄 Already in %s Mode. No action taken.", currentMode.c_str());
        return;
    }

    if (requestedMode == "Localization") {
        RCLCPP_INFO(this->get_logger(), "🟢 New environment input start...");
        pAgent->ActivateLocalizationMode();
        currentMode = "Localization";
    } 
    else if (requestedMode == "SLAM") {
        RCLCPP_INFO(this->get_logger(), "🟢 Switching to SLAM Mode");
        pAgent->DeactivateLocalizationMode();
        currentMode = "SLAM";
    } 
    else {
        RCLCPP_WARN(this->get_logger(), "⚠️ Unknown mode received: %s", requestedMode.c_str());
    }
}
