import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import subprocess
from collections import deque
import threading
import os

class GSLauncher(Node):
    def __init__(self):
        super().__init__('gs_launcher_node')
        self.subscription = self.create_subscription(
            String,
            '/gs_source_path',
            self.listener_callback,
            10
        )
        self.get_logger().info("🚀 GS Launcher Node Ready!")

        self.queue = deque()
        self.is_running = False
        self.lock = threading.Lock()

    def listener_callback(self, msg):
        with self.lock:
            self.queue.append(msg.data)
            self.get_logger().info(f"📥 Queued path: {msg.data}")
            if not self.is_running:
                self.process_next()

    def process_next(self):
        if not self.queue:
            return

        source_path = self.queue.popleft()
        checkpoint = '/home/jk/ros2_test/gs_output/model.model.pth'
        
        if not os.path.exists(checkpoint):
            print("initializing GS Map")
            checkpoint = None

        self.is_running = True
        self.get_logger().info(f"🚀 Launching training for: {source_path}")

        gs_dir = '/home/jk/ros2_test/src/gaussian-splatting'

        def run_process(checkpoint):
            try:
                if not checkpoint:
                    subprocess.run(
                        ['python3', 'train.py', '-s', source_path],
                        cwd=gs_dir, check=True
                    )
                else:
                    subprocess.run(
                        ['python3', 'train.py', '-s', source_path, '--start_checkpoint', checkpoint],
                        cwd=gs_dir, check=True
                    )
                self.get_logger().info("✅ GS training finished!")
            except subprocess.CalledProcessError as e:
                self.get_logger().error(f"❌ GS training failed: {e}")
            finally:
                with self.lock:
                    self.is_running = False
                    self.process_next()

        threading.Thread(target=run_process, args=(checkpoint,)).start()

def main(args=None):
    rclpy.init(args=args)
    node = GSLauncher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()