Project Jackfruit: Lightweight Container Engine & Monitor
🏗 Overview
Project Jackfruit is a simplified containerization engine designed to demonstrate core operating system concepts, including process isolation, filesystem namespacing, resource monitoring via kernel modules, and CPU scheduling using nice values.

The system consists of two primary components:

The Supervisor: A background daemon that manages the lifecycle of containers and enforces resource limits.

The Kernel Monitor: A custom Linux kernel module (monitor.ko) that tracks memory usage (RSS) and enforces soft/hard limits through SIGKILL signals.

🚀 Key Features
Filesystem Isolation: Uses chroot logic to isolate containers within specific root filesystems (rootfs-alpha, rootfs-beta).

Resource Monitoring: Real-time tracking of memory consumption.

Resource Enforcement: * Soft Limit: Logs a warning when a container exceeds a specific threshold.

Hard Limit: Terminates the process once it hits a critical memory ceiling.

Priority Scheduling: Support for nice value adjustments to demonstrate CPU time allocation differences between "fast" and "slow" processes.

Clean Teardown: Automated cleanup of all child processes and socket files upon supervisor exit.

🛠 Installation & Setup
1. Prerequisites
Ensure you are running on a Linux environment (tested on Ubuntu 22.04) with kernel headers installed.

2. Compilation
Compile the engine, the helper binaries (cpu_hog, memory_hog), and the kernel module using the provided Makefile:

Bash
make clean
make
3. Loading the Kernel Module
The monitor must be loaded into the kernel before starting the supervisor:

Bash
sudo insmod monitor.ko
# Verify the major number and create the device node
MAJOR=$(awk '$2=="monitor" {print $1}' /proc/devices)
sudo mknod /dev/container_monitor c $MAJOR 0
sudo chmod 666 /dev/container_monitor
💻 Usage Guide
Starting the Supervisor
Open a dedicated terminal and start the core management process:

Bash
sudo ./engine supervisor ./rootfs-base
Launching Containers
In a separate terminal, use the start command:

Bash
# General Syntax
sudo ./engine start [name] [rootfs_path] [executable] [args...] [--nice value]

# Example: Memory Monitoring Test
sudo ./engine start mem_kill ./rootfs-alpha /memory_hog 150 10
Monitoring Resource Limits
To see the kernel module in action (SS 5 & 6), monitor the kernel ring buffer:

Bash
sudo dmesg -w | grep "container_monitor"
Viewing Container Logs
Bash
sudo ./engine logs [container_name]
Managing Processes
Bash
# List all running containers
sudo ./engine ps
📊 Evaluation Scenarios
Memory Enforcement (Screenshots 5 & 6)
When a container running memory_hog exceeds the defined limits, the kernel module will log:

[container_monitor] SOFT LIMIT: Logged when memory usage is significant.

[container_monitor] HARD LIMIT: Logged just before the process is issued a SIGKILL.

Priority Scheduling (Screenshot 7)
By running two containers with different nice values, you can observe the difference in the accumulator values in the logs:

High Priority (--nice -10): Higher accumulator (more CPU cycles).

Low Priority (--nice 19): Lower accumulator (fewer CPU cycles).

🧹 Teardown
To stop the engine and all associated containers, navigate to the Supervisor terminal and press Ctrl+C. The engine is designed to:

Identify all active child PIDs.

Send SIGKILL to each.

Unmount /proc filesystems.

Remove the communication socket /tmp/mini_runtime.sock.

Author: [Your Name / PES1UG24CS132]

Course: Operating Systems Lab

Date: April 2026
