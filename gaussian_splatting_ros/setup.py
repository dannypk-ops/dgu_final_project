from setuptools import find_packages, setup

package_name = 'gaussian_splatting_ros'

setup(
    name=package_name,
    version='0.1.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
         ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='(your name)',
    maintainer_email='(your email)',
    description='ROS 2 wrapper for Gaussian Splatting training',
    license='MIT',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'gs_launcher_node = gaussian_splatting_ros.gs_launcher_node:main',
        ],
    },
)