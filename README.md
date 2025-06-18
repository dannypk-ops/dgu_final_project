# 📦 FMS-based Digital Twin Framework for Intelligent Logistics Automation

> Capstone Design Class, Dongguk University  
> **Project Period**: March 2025 – June 2025  
> **Team Project** | Individual Contribution Highlighted Below  

## 🔍 Overview

This project proposes a real-time, high-resolution **3D Digital Twin system** capable of reflecting environmental changes occurring in smart logistics environments. By integrating **ORB-SLAM3**, **COLMAP**, and **Gaussian Splatting**, our system can dynamically update and visualize 3D maps as the environment evolves — without requiring full re-scanning.

The system is developed on **ROS 2 Humble**, and modularized into SLAM, Keyframe Update, and 3D Visualization nodes.

## 🧩 Key Features

- **Monocular ORB-SLAM3**: Extracts keyframes and constructs Covisibility Graphs from image sequences.
- **Keyframe Replacement Algorithm**: Dynamically updates the scene with newly acquired images.
- **Sparse Reconstruction + Gaussian Splatting**: Generates high-fidelity 3D visualization from updated keyframes.
- **ROS 2 Pipeline**: Real-time integration with actual robot systems.

## 🙋‍♂️ My Contributions

I was responsible for the following core components of the system:

### 1. **Keyframe Update & Scene Graph Replacement**
- Developed a module to **insert new image sequences** collected from the robot into the global scene graph.
- Implemented a **Covisibility-based similarity comparison** to calculate shared ORB feature weights between new and existing keyframes.
- Designed a replacement logic using a **dynamic threshold (mean + std)** to selectively **replace outdated keyframes**.
- Ensured efficient integration with the SLAM system to reflect **local environmental changes** in the global map.

### 2. **Updated Keyframe-Based 3D Reconstruction**
- Converted the updated keyframe set into a **COLMAP-compatible format** for sparse reconstruction.
- Integrated **Gaussian Splatting** to generate smooth, high-resolution 3D representations from updated keyframe data.
- Focused on preserving **spatiotemporal consistency** across updates while minimizing computational overhead.

### 3. **Real-World Testing & Validation**
- Verified the system not only with robot-captured images, but also with externally recorded sequences.
- Confirmed the robustness of the pipeline under dynamic environmental conditions.

## ⚙️ Development Environment

| Tool / Framework | Version / Details         |
|------------------|---------------------------|
| OS               | Ubuntu 22.04 LTS          |
| ROS              | ROS 2 Humble              |
| SLAM             | ORB-SLAM3 (Monocular)     |
| Visualization    | COLMAP, Gaussian Splatting|
| Language         | Python 3.9, C++           |
| Libraries        | OpenCV, NumPy, PyTorch    |

## 📸 Result Sample ( Demo )

### Keyframe Update & Scene Graph Replacement
![KakaoTalk_20250619_001947284_01](https://github.com/user-attachments/assets/886f9545-3b92-4af5-8205-81200df3e050)

### Updated Keyframe-Based 3D Reconstruction
![KakaoTalk_20250619_002422700](https://github.com/user-attachments/assets/e27a73c6-0dc6-4a3a-9a4d-b94feb0f6cbe)

![image](https://github.com/user-attachments/assets/aab634d1-c4b7-4de8-b53d-6d81485843a9)


