#!/usr/bin/env python3

import os
import subprocess
import rclpy
from rclpy.node import Node
from std_msgs.msg import String

# ---------------------------
# COLMAP CLI 호출 유틸
# ---------------------------
def run_colmap_cmd(args):
    subprocess.run(["colmap"] + args, check=True)

# ---------------------------
# SfM 파이프라인 (GUI 흐름 동일)
# ---------------------------
def run_sfm_pipeline(base_folder: str, image_dir: str, db_path: str):
    # 출력 디렉토리
    sparse_dir = os.path.join(base_folder, "final", "sparse")
    os.makedirs(sparse_dir, exist_ok=True)

    # 1) 기존 DB 제거
    if os.path.exists(db_path):
        os.remove(db_path)

    # 2) Feature Extraction
    print(image_dir)

    run_colmap_cmd([
        "feature_extractor",
        "--database_path", db_path,
        "--image_path", image_dir,
        "--ImageReader.camera_model", "PINHOLE"
    ])

    # ---------------------------
    # 3) Feature Matching (Exhaustive) — GUI 파라미터 그대로
    # ---------------------------
    run_colmap_cmd([
        "exhaustive_matcher",
        "--database_path", db_path,
    ])

    # 4) Mapper (SfM 수행: pose, 3D 포인트 자동 생성)
    run_colmap_cmd([
        "mapper",
        "--database_path", db_path,
        "--image_path", image_dir,
        "--output_path", sparse_dir,
    ])

    # 5) TXT 모델 변환 (optional)
    model_subdir = os.path.join(sparse_dir, "0")
    run_colmap_cmd([
        "model_converter",
        "--input_path", model_subdir,
        "--output_path", model_subdir,
        "--output_type", "TXT"
    ])

    print(f"[INFO] COLMAP SfM reconstruction complete. Output: {sparse_dir}")

# ---------------------------
# ROS2 노드
# ---------------------------
class ColmapPipelineNode(Node):
    def __init__(self):
        super().__init__('colmap_pipeline_node')
        self.sub = self.create_subscription(
            String, '/stereo_py_driver/SLAM_done', self.callback, 10
        )
        self.gs_pub = self.create_publisher(
            String, '/gs_source_path', 10
        )
        self.get_logger().info("COLMAP Pipeline Node ready.")

    def callback(self, msg: String):
        base_folder = msg.data
        self.get_logger().info(f"Starting COLMAP SfM on: {base_folder}")

        # 경로 설정
        image_dir = os.path.join(base_folder, "SLAM_images")
        db_path   = os.path.join(base_folder, "database.db")

        try:
            run_sfm_pipeline(base_folder, image_dir, db_path)
            self.get_logger().info("COLMAP SfM complete.")

            # 결과 경로 publish
            gs_msg = String()
            gs_msg.data = os.path.join(base_folder, "final")
            self.gs_pub.publish(gs_msg)
            self.get_logger().info(f"Published GS source path: {gs_msg.data}")

        except subprocess.CalledProcessError as e:
            self.get_logger().error(f"COLMAP command failed: {e}")
        except Exception as e:
            self.get_logger().error(f"Error in SfM pipeline: {e}")

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
