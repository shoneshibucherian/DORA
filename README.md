# DORA: Dynamic Objective-Reaching Autonomous Rovers

🚀 **NASA MINDS Competition Project**

The **DORA** project was created to participate in the NASA MINDS competition. The objective of this project is to develop an autonomous swarm robotics system to explore uncharted and high-risk areas of the Moon without risking the lives of astronauts. 

This repository focuses on the hardware build and software codebase of **DORA** (the giant ATV lead rover) and its **Boots** (the smaller follower rovers).
<img width="5712" height="4284" alt="IMG_7763" src="https://github.com/user-attachments/assets/c6148d35-73e9-4203-aac5-23f83145548c" />

---

## 📌 System Overview & Swarm Concept

Our system uses a "lead and follower" swarm hierarchy to maximize exploration efficiency:
* **The Lead Rover (DORA):** A giant ATV that serves as the command center, coordinating the swarm, mapping the terrain, and sending instructions.
* **The Follower Rovers (BOOTS):** Smaller, agile rovers controlled by DORA, sent to explore high-risk and unreachable terrain.

---

## ⚙️ Hardware & Drive System

The ATV uses a **Bicycle Drive System** designed to simplify locomotion while ensuring precise control:
* **Forward/Backward Motion:** Powered by a single DC Motor.
* **Steering (Left/Right):** Driven by an ASME Servo Motor.

### Component Connections & Signal Flow:
1. **User Input:** Movement commands (e.g., "move forward 5 meters") are sent via a Remote Controller or autonomy scripts to the **Raspberry Pi**.
2. **Main Computation:** The **Raspberry Pi** runs the core robotics stack, processing mapping (G-Mapping) and sending velocity commands to the `ros2_control` system.
3. **Microcontroller Delegation:** Command payloads are passed to an **Arduino Nano** as the dedicated motor controller to handle low-level loop control, reducing computing load on the Raspberry Pi.
4. **Actuators:**
   * **Steering:** The Arduino Nano sends PWM signals directly to the **ASME Servo Motor**.
   *<img width="5712" height="4284" alt="IMG_7761" src="https://github.com/user-attachments/assets/c9f48f61-9888-4145-99ab-b588ee4b8939" />
 
   * **Drive:** The Arduino Nano commands the **Cytron Motor Driver** to drive the **DC Motor**.
   * <img width="5712" height="4284" alt="IMG_7760" src="https://github.com/user-attachments/assets/726b3906-8a00-472b-ab1e-256a90e1b29b" />


---

## 💻 Software Stack & Hierarchy

The software architecture divides heavy computation and hardware-level task execution to optimize performance:

* **Operating System:** Ubuntu 24.04 LTS
* **Robot Operating System:** ROS 2 (Jazzy Jalisco)
* **High-Level Tasks (Raspberry Pi):**
  * **Lidar Processing & G-Mapping:** Generates real-time 2D maps of the environment.
  * **Bicycle Drive Control:** Adapted from standard `ros2_control` examples to transform simple `X` direction movements into hardware-understandable variables.
* **Low-Level Tasks (Arduino Nano):**
  * **Feedback Loop:** Runs dedicated control loops to ensure the ATV matches the requested physical outputs (e.g., maintain 5 meters per second) based on driver feedback.
  <img width="2400" height="1792" alt="final dia" src="https://github.com/user-attachments/assets/2b7d753f-06f9-4c89-b0ba-ddf5cab58a9d" />

  ### Quick Start Commands

To get up and running on your machine (Ubuntu 24.04 with ROS 2 Jazzy):

1. **Clone the repository and switch to the `trail` branch:**
   ```bash
   git clone [https://github.com/shoneshibucherian/DORA.git](https://github.com/shoneshibucherian/DORA.git)
   cd DORA
   git checkout joystick_control

2. **Install dependencies:**
   Make sure you are in the workspace root, then resolve required packages using `rosdep`:
   ```bash
   rosdep install --from-paths src --ignore-src -y
   ```

3. **Build the workspace:**
   Build the projects using `colcon`. This will automatically generate your `build/`, `install/`, and `log/` directories:
   ```bash
   colcon build
   ```

4. **Source & Launch:**
   Configure your terminal environment and launch the main robot control and mapping nodes:
   ```bash
   source install/setup.bash
   ros2 launch boots diff_drive.launch.py
   ```
