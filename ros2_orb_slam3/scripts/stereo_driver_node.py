#!/usr/bin/env python3


"""
Python node for the MonocularMode cpp node.

Author: Azmyin Md. Kamal
Date: 01/01/2024

Requirements
* Dataset must be configured in EuRoC MAV format
* Paths to dataset must be set before bulding (or running) this node
* Make sure to set path to your workspace in common.hpp

Command line arguments
-- settings_name: EuRoC, TUM2, KITTI etc; the name of the .yaml file containing camera intrinsics and other configurations
-- image_seq: MH01, V102, etc; the name of the image sequence you want to run

"""

# Imports
#* Import Python modules
import sys # System specific modules
import os # Operating specific functions
import glob
import time # Python timing module
import copy # For deepcopying arrays
import shutil # High level folder operation tool
from pathlib import Path # To find the "home" directory location
import argparse # To accept user arguments from commandline
import natsort # To ensure all images are chosen loaded in the correct order
import yaml # To manipulate YAML files for reading configuration files
import copy # For making deepcopies of openCV matrices, python lists, numpy arrays etc.
import numpy as np # Python Linear Algebra module
import cv2 # OpenCV

#* ROS2 imports
import ament_index_python.packages
import rclpy
from rclpy.node import Node
from rclpy.parameter import Parameter

# If you have more files in the submodules folder
# from .submodules.py_utils import fn1 # Import helper functions from files in your submodules folder

# Import a custom message interface
# from your_custom_msg_interface.msg import CustomMsg #* Note the camel caps convention

# Import ROS2 message templates
from sensor_msgs.msg import Image # http://wiki.ros.org/sensor_msgs
from std_msgs.msg import String, Float64 # ROS2 string message template
from cv_bridge import CvBridge, CvBridgeError # Library to convert image messages to numpy array

#* Class definition
class StereoDriver(Node):
    def __init__(self, node_name = "stereo_py_node"):
        super().__init__(node_name) # Initializes the rclpy.Node class. It expects the name of the node

        # Initialize parameters to be passed from the command line (or launch file)
        self.declare_parameter("settings_name","EuRoC")
        self.declare_parameter("image_seq","NULL")

        #* Parse values sent by command line
        self.settings_name = str(self.get_parameter('settings_name').value) 
        self.image_seq = str(self.get_parameter('image_seq').value)
        self.image_seq_localization = "sample_localization"

        # DEBUG
        print(f"-------------- Received parameters --------------------------\n")
        print(f"self.settings_name: {self.settings_name}")
        print(f"self.image_seq: {self.image_seq}")
        print()

        # Global path definitions
        self.home_dir = str(Path.home()) + "/ros2_test/src/ros2_orb_slam3" #! Change this to match path to your workspace
        self.parent_dir = "TEST_DATASET" #! Change or provide path to the parent directory where data for all image sequences are stored
        self.image_sequence_dir = self.home_dir + "/" + self.parent_dir + "/" + self.image_seq # Full path to the image sequence folder
        self.image_sequence_dir_localization = self.home_dir + "/" + self.parent_dir + "/" + self.image_seq_localization
        print(f"self.image_sequence_dir: {self.image_sequence_dir}\n")

        # Global variables
        self.node_name = "stereo_py_driver"
        self.image_seq_dir = ""
        self.imgz_seqz = []
        self.time_seqz = [] # Maybe redundant

        # Define a CvBridge object
        self.br = CvBridge()

        # Read images from the chosen dataset, order them in ascending order and prepare timestep data as well
        # self.imgz_seqz_dir, self.imgz_seqz, self.time_seqz = self.get_image_dataset_asl(self.image_sequence_dir, "mav0") 
        # self.imgz_seqz_dir_local, self.imgz_seqz_local, self.time_seqz_local = self.get_image_dataset_asl(self.image_sequence_dir_localization, "mav0")
        # self.imgz_seqz_dir_local2, self.imgz_seqz_local2, self.time_seqz_local2 = self.get_image_dataset_asl(self.image_sequence_dir_localization, "mav1")

        # Stereo 이미지에 대해 클래스 변수들 생성
        self.l_imgz_seqz_dir, self.r_imgz_seqz_dir, self.l_imgz_seqz, self.r_imgz_seqz, self.time_seqz = self.get_image_dataset_asl(self.image_sequence_dir, "mav0")
        self.l_imgz_seqz_dir_local, self.r_imgz_seqz_dir_local, self.l_imgz_seqz_local, self.r_imgz_seqz_local, self.time_seqz_local = self.get_image_dataset_asl(self.image_sequence_dir_localization, "mav0")
        # self.l_imgz_seqz_dir_local2, self.r_imgz_seqz_dir_local2, self.l_imgz_seqz_local2, self.r_imgz_seqz_local2, self.time_seqz_local2 = self.get_image_dataset_asl(self.image_sequence_dir_localization, "mav1")



        print(self.image_seq_dir)
        print(len(self.imgz_seqz))

        #* ROS2 publisher/subscriber variables [HARDCODED]
        self.pub_exp_config_name = "/stereo_py_driver/experiment_settings" 
        self.sub_exp_ack_name = "/stereo_py_driver/exp_settings_ack"
        # self.pub_img_to_agent_name = "/stereo_py_driver/img_msg"
        self.pub_left_img_to_agent_name = "/stereo_py_driver/left"
        self.pub_right_img_to_agent_name = "/stereo_py_driver/right"
        self.pub_timestep_to_agent_name = "/stereo_py_driver/timestep_msg"
        self.localization_mode = "/stereo_py_driver/localization_msg"
        self.send_config = True # Set False once handshake is completed with the cpp node
        
        
        #* Setup ROS2 publishers and subscribers
        self.publish_exp_config_ = self.create_publisher(String, self.pub_exp_config_name, 1) # Publish configs to the ORB-SLAM3 C++ node
        self.publish_finish_ = self.create_publisher(String, "/stereo_py_driver/finished", 1)

        #* Build the configuration string to be sent out
        #self.exp_config_msg = self.settings_name + "/" + self.image_seq # Example EuRoC/sample_euroc_MH05
        self.exp_config_msg = self.settings_name # Example EuRoC
        print(f"Configuration to be sent: {self.exp_config_msg}")


        #* Subscriber to get acknowledgement from CPP node that it received experimetn settings
        self.subscribe_exp_ack_ = self.create_subscription(String, 
                                                           self.sub_exp_ack_name, 
                                                           self.ack_callback ,10)
        self.subscribe_exp_ack_

        # Publisher to send RGB image
        # self.publish_img_msg_ = self.create_publisher(Image, self.pub_img_to_agent_name, 1)
        
        self.pub_left  = self.create_publisher(Image, self.pub_left_img_to_agent_name,  1)
        self.pub_right = self.create_publisher(Image, self.pub_right_img_to_agent_name, 1)

        self.publish_timestep_msg_ = self.create_publisher(Float64, self.pub_timestep_to_agent_name, 1)

        self.publish_localization_mode_ = self.create_publisher(String, self.localization_mode, 1)



        # Initialize work variables for main logic
        self.start_frame = 0 # Default 0
        self.end_frame = -1 # Default -1
        self.frame_stop = -1 # Set -1 to use the whole sequence, some positive integer to force sequence to stop, 350 test2, 736 test3
        self.show_imgz = False # Default, False, set True to see the output directly from this node
        self.frame_id = 0 # Integer id of an image frame
        self.frame_count = 0 # Ensure we are consistent with the count number of the frame
        self.inference_time = [] # List to compute average time

        print()
        print(f"MonoDriver initialized, attempting handshake with CPP node")
    # ****************************************************************************************

    # ****************************************************************************************
    def get_image_dataset_asl(self, exp_dir, agent_name = "mav0"):
        """
            Returns images and list of timesteps in ascending order from a ASL formatted dataset
        """
        
        # print("exp_dir : ", exp_dir)
        # Define work variables
        time_list = []

        #* Only works for EuRoC MAV format
        left_image_dir = exp_dir + "/" + agent_name + "/" + "cam0"
        right_image_dir = exp_dir + "/" + agent_name + "/" + "cam1"

        left_imgz_file_dir = left_image_dir + "/" + "data" + "/"
        right_imgz_file_dir = right_image_dir + "/" + "data" + "/"

        left_imgz_file_list = natsort.natsorted(os.listdir(left_imgz_file_dir),reverse=False)
        right_imgz_file_list = natsort.natsorted(os.listdir(right_imgz_file_dir),reverse=False)
        # print(len(img_file_list)) # Debug, checks the number of rgb images

        # Extract timesteps from image names
        for iox in left_imgz_file_list:
            time_step = iox.split(".")[0]
            time_list.append(time_step)
            #print(time_step)

        return left_imgz_file_dir, right_imgz_file_dir, left_imgz_file_list, right_imgz_file_list, time_list
    # ****************************************************************************************

    # ****************************************************************************************
    def ack_callback(self, msg):
        """
            Callback function
        """
        print(f"Got ack: {msg.data}")
        
        if(msg.data == "ACK"):
            self.send_config = False
            # self.subscribe_exp_ack_.destory() # TODO doesn't work 
    # ****************************************************************************************
    
    # ****************************************************************************************
    def handshake_with_cpp_node(self):
        """
            Send and receive acknowledge of sent configuration settings
        """
        if (self.send_config == True):
            # print(f"Sent mesasge: {self.exp_config_msg}")
            msg = String()
            msg.data = self.exp_config_msg
            self.publish_exp_config_.publish(msg)
            time.sleep(0.01)
    # ****************************************************************************************
    
    # ****************************************************************************************
    def run_stereo_node(self, idx, left_imgz_name, right_imgz_name, mode = "SLAM"):
        """
            Master function that sends the RGB image message to the CPP node
        """

        # Initialize work variables
        left_img_msg = None # sensor_msgs image object
        right_img_msg = None

        # Path to this image
        if mode == "SLAM":
            left_img_path = self.l_imgz_seqz_dir + left_imgz_name
            right_img_path = self.r_imgz_seqz_dir + right_imgz_name
        elif mode == "Localization":
            left_img_path = self.l_imgz_seqz_dir_local + left_imgz_name
            right_img_path = self.r_imgz_seqz_dir_local + right_imgz_name
        # else:
        #     left_img_path = self.l_imgz_seqz_dir_local2 + left_imgz_name
        #     right_img_path = self.r_imgz_seqz_dir_local + right_imgz_name

        # timestep = float(left_imgz_name.split(".")[0].split("_")[1]) # Kept if you use a custom message interface to also pass timestep value
        timestep = float(left_imgz_name.split(".")[0])
        self.frame_id = self.frame_id + 1  
        #print(img_look_up_path)
        # print(f"Frame ID: {frame_id}")

        # imgL = cv2.imread(left_img_path)      # BGR
        # imgR = cv2.imread(right_img_path)

        # stereo_img = cv2.hconcat([imgL, imgR])     # (height, width*2, 3)

        # msg = self.br.cv2_to_imgmsg(stereo_img, encoding="bgr8")
        # msg.header.stamp = rclpy.clock.Clock().now().to_msg()

        # Based on the tutorials
        left_img_msg = self.br.cv2_to_imgmsg(cv2.imread(left_img_path), encoding="bgr8")
        right_img_msg = self.br.cv2_to_imgmsg(cv2.imread(right_img_path), encoding="bgr8")
        timestep_msg = Float64()
        timestep_msg.data = timestep

        # Publish RGB image and timestep, must be in the order shown below. I know not very optimum, you can use a custom message interface to send both
        # Publish the left and right images and the timestep
        try:
            self.publish_timestep_msg_.publish(timestep_msg)
            self.pub_left.publish(left_img_msg)
            self.pub_right.publish(right_img_msg)  # Both images are sent to the C++ node
            # self.pub_stereo.publish(msg)
        except CvBridgeError as e:
            print(e)
    # ****************************************************************************************

    def send_localization_mode(self, mode="Localization"):
        """
        Sends mode switch command to CPP node.
        mode: "Localization" or "SLAM"
        """
        if mode not in ["Localization", "SLAM"]:
            print(f"❌ Invalid mode: {mode}")
            return

        msg = String()
        msg.data = mode
        self.publish_localization_mode_.publish(msg)
        print(f"📤 Sent mode switch command: {mode}")

# main function
def main(args = None):
    rclpy.init(args=args) # Initialize node
    n = StereoDriver("stereo_py_node") #* Initialize the node
    rate = n.create_rate(20) # https://answers.ros.org/question/358343/rate-and-sleep-function-in-rclpy-library-for-ros2/
    
    #* Blocking loop to initialize handshake
    while(n.send_config == True):
        n.handshake_with_cpp_node()
        rclpy.spin_once(n)
        #self.rate.sleep(10) # Potential bug, breaks code

        if(n.send_config == False):
            break
        
    print(f"Handshake complete")

    for idx, (l_img, r_img) in enumerate(zip(n.l_imgz_seqz, n.r_imgz_seqz)):
        try:
            rclpy.spin_once(n)  # Keep the node running
            n.run_stereo_node(idx, l_img, r_img)  # Send Stereo image
            rate.sleep()
        except KeyboardInterrupt:
            break
    
    finish_msg = String()
    finish_msg.data = "done"
    n.publish_finish_.publish(finish_msg)
    print("✅ All images sent. Published 'done' signal to /stereo_py_driver/finished")

    time.sleep(30)

    for idx, (l_img, r_img) in enumerate(zip(n.l_imgz_seqz_local, n.r_imgz_seqz_local)):
        try:
            rclpy.spin_once(n)  # Keep the node running
            n.run_stereo_node(idx, l_img, r_img, mode="Localization")  # Send Stereo image
            rate.sleep()
        except KeyboardInterrupt:
            break
    
    finish_msg = String()
    finish_msg.data = "done"
    n.publish_finish_.publish(finish_msg)
    print("✅ All images sent. Published 'done' signal to /stereo_py_driver/finished")

    time.sleep(30)

    # Cleanup
    cv2.destroyAllWindows() # Close all image windows
    n.destroy_node() # Release all resource related to this node
    rclpy.shutdown()

# Dunders, this .py is the main file
if __name__=="__main__":
    main()
