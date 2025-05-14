#!/usr/bin/env python3

import os
import shutil
import subprocess
import rclpy
from rclpy.node import Node
from std_msgs.msg import String


# ---------------------------
# COLMAP 관련 유틸 함수
# ---------------------------
def create_folder_structure(base_folder):
    os.makedirs(os.path.join(base_folder, 'final', 'sparse', '0'), exist_ok=True)
    os.makedirs(os.path.join(base_folder, 'final', 'images'), exist_ok=True)


def run_colmap_cmd(args):
    subprocess.run(["colmap"] + args, check=True)


def run_full_colmap_pipeline(base_folder, image_dir, db_path):
    sparse_out_dir = os.path.join(base_folder, 'final', 'sparse', '0')
    image_store_dir = os.path.join(base_folder, 'final', 'images')

    create_folder_structure(base_folder)

    # Feature Extraction
    run_colmap_cmd([
        "feature_extractor",
        "--database_path", db_path,
        "--image_path", image_dir,
        "--ImageReader.camera_model", "PINHOLE",
        "--ImageReader.single_camera", "1"
    ])

    # Feature Matching
    run_colmap_cmd([
        "exhaustive_matcher",
        "--database_path", db_path
    ])

    # Sparse Reconstruction (mapping)
    run_colmap_cmd([
        "mapper",
        "--database_path", db_path,
        "--image_path", image_dir,
        "--output_path", os.path.join(base_folder, 'final', 'sparse')
    ])

    # Optional: Convert BIN → TXT
    run_colmap_cmd([
        "model_converter",
        "--input_path", sparse_out_dir,
        "--output_path", sparse_out_dir,
        "--output_type", "TXT"
    ])

    # 이미지 보존 (옮겨 저장)
    for img in os.listdir(image_dir):
        shutil.move(os.path.join(image_dir, img), os.path.join(image_store_dir, img))

    print("[INFO] COLMAP full SfM pipeline complete.")


# ---------------------------
# ROS2 노드
# ---------------------------
class ColmapPipelineNode(Node):
    def __init__(self):
        super().__init__('colmap_pipeline_node')
        self.sub = self.create_subscription(
            String, '/mono_py_driver/SLAM_done', self.callback, 10
        )

        self.gs_pub = self.create_publisher(
            String, '/gs_source_path', 10
        )

        self.get_logger().info("COLMAP Pipeline Node ready (image-only mode).")

    def callback(self, msg):
        if msg.data:
            self.get_logger().info("Starting COLMAP image-only reconstruction...")

            self.base_folder = msg.data
            self.image_dir = os.path.join(self.base_folder, "SLAM_images")
            self.db_path = os.path.join(self.base_folder, "database.db")

            try:
                run_full_colmap_pipeline(self.base_folder, self.image_dir, self.db_path)
                self.get_logger().info("COLMAP reconstruction complete.")

                final_source_path = os.path.join(self.base_folder, "final")
                msg = String()
                msg.data = final_source_path
                self.gs_pub.publish(msg)
                self.get_logger().info(f"Published GS source path: {final_source_path}")
                return

            except Exception as e:
                self.get_logger().error(f"Error in reconstruction: {e}")


# ---------------------------
# 메인
# ---------------------------
def main(args=None):
    rclpy.init(args=args)
    node = ColmapPipelineNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
