#!/usr/bin/env python3

import os
import shutil
import subprocess
import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool, String

# ---------------------------
# COLMAP 관련 유틸 함수
# ---------------------------
def create_folder_structure(base_folder):
    os.makedirs(os.path.join(base_folder, 'init', 'sparse'), exist_ok=True)
    os.makedirs(os.path.join(base_folder, 'final', 'sparse', '0'), exist_ok=True)
    os.makedirs(os.path.join(base_folder, 'final', 'images'), exist_ok=True)

def write_cameras_txt(folder, camera_model="PINHOLE", width=752, height=480, fx=458.654, fy=457.296, cx=367.215, cy=248.375):
    with open(os.path.join(folder, "cameras.txt"), "w") as f:
        f.write("# CAMERA_ID, MODEL, WIDTH, HEIGHT, PARAMS[]\n")
        f.write(f"1 {camera_model} {width} {height} {fx} {fy} {cx} {cy}\n")

def write_images_txt(folder, poses_path, image_dir):
    images = sorted([img for img in os.listdir(image_dir) if img.endswith('.png')])
    with open(poses_path, 'r') as f:
        poses = [line.strip() for line in f.readlines() if line.strip()]

    if len(images) != len(poses):
        raise ValueError(f"The number of images ({len(images)}) and poses ({len(poses)}) must be the same.")

    with open(os.path.join(folder, "images.txt"), "w") as f:
        for i, (pose, img) in enumerate(zip(poses, images), 1):
            parts = pose.split()
            tx, ty, tz = parts[1:4]
            qx, qy, qz, qw = parts[4:8]
            f.write(f"{i} {qw} {qx} {qy} {qz} {tx} {ty} {tz} 1 {img}\n\n")

def write_empty_points3D(folder):
    with open(os.path.join(folder, "points3D.txt"), "w") as f:
        f.write("# Empty points3D file\n")

def move_images_to_dir(image_dir, triangulated_sparse_folder):
    for img in os.listdir(image_dir):
        shutil.move(os.path.join(image_dir, img), os.path.join(triangulated_sparse_folder, img))

# ---------------------------
# COLMAP 파이프라인
# ---------------------------
def run_colmap_cmd(args):
    subprocess.run(["colmap"] + args, check=True)

def run_sparse_reconstruction(base_folder, poses_path, image_dir, db_path):
    # created_sparse_folder = os.path.join(base_folder, 'created', 'sparse')
    # triangulated_sparse_folder = os.path.join(base_folder, 'final', 'sparse', '0')
    # image_store_dir = os.path.join(base_folder, 'SLAM_images', 'images')

    create_folder_structure(base_folder)

    created_sparse_folder = os.path.join(base_folder, 'init')
    final_folder = os.path.join(base_folder, 'final', 'sparse', '0')
    image_store_dir = os.path.join(base_folder, 'final', 'images')

    write_cameras_txt(created_sparse_folder)
    write_images_txt(created_sparse_folder, poses_path, image_dir)
    write_empty_points3D(created_sparse_folder)

    # Feature Extraction
    run_colmap_cmd([
        "feature_extractor",
        "--database_path", db_path,
        "--image_path", image_dir,
        "--ImageReader.camera_model", "PINHOLE",
        "--ImageReader.single_camera", "1"
    ])

    # Feature Matching
    run_colmap_cmd(["exhaustive_matcher", "--database_path", db_path])

    # Point Triangulation
    run_colmap_cmd([
        "point_triangulator",
        "--database_path", db_path,
        "--image_path", image_dir,
        "--input_path", created_sparse_folder,
        "--output_path", final_folder
    ])

    # Bundle Adjustment (Pose 고정)
    run_colmap_cmd([
        "bundle_adjuster",
        "--input_path", final_folder,
        "--output_path", final_folder
    ])

    # 모델 변환 (BIN 저장)
    run_colmap_cmd([
        "model_converter",
        "--input_path", final_folder,
        "--output_path", final_folder,
        "--output_type", "BIN"
    ])

    move_images_to_dir(image_dir, image_store_dir)

    print("[INFO] Sparse reconstruction and undistortion finished successfully.")

# ---------------------------
# ROS2 노드
# ---------------------------
class ColmapPipelineNode(Node):
    def __init__(self):
        super().__init__('colmap_pipeline_node')
        self.sub = self.create_subscription(
            String, '/mono_py_driver/SLAM_done', self.callback, 10
        )

        self.gs_pub = self.create_publisher(  # ← 새로운 publisher 추가
            String, '/gs_source_path', 10
        )

        self.get_logger().info("COLMAP Pipeline Node ready.")

    def callback(self, msg):
        if msg.data:
            self.get_logger().info("Starting COLMAP reconstruction...")

            self.base_folder = msg.data;
            self.poses_path = os.path.join(self.base_folder, "SLAM_pose", "trajectory.txt")
            self.image_dir = os.path.join(self.base_folder, "SLAM_images")
            self.db_path = os.path.join(self.base_folder, "database.db")

            try:
                run_sparse_reconstruction(self.base_folder, self.poses_path, self.image_dir, self.db_path)
                self.get_logger().info("COLMAP reconstruction complete.")

                # Gaussian Splatting 노드에 source path 전달
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