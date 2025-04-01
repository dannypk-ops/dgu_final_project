import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import subprocess

class GSLauncher(Node):
    def __init__(self):
        super().__init__('gs_launcher_node')
        self.subscription = self.create_subscription(
            String,
            '/gs_source_path',
            self.listener_callback,
            10
        )
        self.get_logger().info("🚀 Gaussian Splatting Launcher Node Ready!")

    def listener_callback(self, msg):
        source_path = msg.data
        self.get_logger().info(f"📂 Received path: {source_path}")
        try:
            # train.py 실행 (본인이 Gaussian Splatting 클론해둔 위치에 맞게 cwd 수정)
            subprocess.run(['python3', 'train.py', '-s', source_path],
                           cwd='/home/jk/ros2_test/src/gaussian-splatting',
                           check=True)
            self.get_logger().info("✅ GS training finished!")
        except subprocess.CalledProcessError as e:
            self.get_logger().error(f"❌ GS training failed: {e}")

def main(args=None):
    rclpy.init(args=args)
    node = GSLauncher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
