# 🐳 Lightweight Multi-Container Runtime in C

A lightweight Linux container runtime in C with a long-running supervisor and a kernel-space memory monitor.

---

# 1. Team Information

## Team Members

**Member 1:** Chhavi S Wadhwa
**SRN:** PES1UG24CS132

**Member 2:** Chetha Samrutha
**SRN:** PES1UG24CS131

## 📌 Overview

This project implements a **lightweight container runtime** in C using core Linux system calls. It demonstrates how containers can be built from scratch using process creation, filesystem isolation, and basic supervision techniques.

The system supports running **multiple containers**, executing workloads inside them, monitoring their execution, and capturing logs.

---

## 🎯 Objectives

* Understand how containers work internally
* Implement process isolation using Linux primitives
* Run and manage multiple containers
* Execute and analyze different workloads
* Capture and log container output

---

## ⚙️ Key Features

* ✅ Multi-container support
* ✅ CLI-based container management (`start`, `stop`, `ps`)
* ✅ Filesystem isolation using `chroot()`
* ✅ Process creation using `fork()`
* ✅ Workload execution inside containers
* ✅ Logging using `dup2()` redirection
* ✅ Basic container lifecycle management

---

## 🏗️ System Architecture

### 🔹 Container Creation

1. `fork()` creates a child process
2. `chroot()` changes root filesystem
3. `chdir("/")` ensures correct working directory
4. `exec()` runs the container process

---

### 🔹 Container Management

* **start** → creates container
* **ps** → lists containers (from file storage)
* **stop** → terminates container

---

### 🔹 Logging Mechanism

* Standard output (`stdout`) and error (`stderr`) are redirected
* `dup2()` is used to write logs into `container.log`
* `fflush()` ensures real-time logging

---

## 🖥️ Technologies Used

* C Programming
* Linux System Calls
* VirtualBox (Ubuntu VM)
* GCC Compiler

---

## 🚀 How to Run

### 🔹 1. Build the Project

```bash
make
```

---

### 🔹 2. Load Kernel Module

```bash
sudo insmod monitor.ko
```

---

### 🔹 3. Start Containers

```bash
sudo ./engine start alpha ../rootfs-alpha /bin/sh
sudo ./engine start beta ../rootfs-beta /bin/sh
```

---

### 🔹 4. List Containers

```bash
sudo ./engine ps
```

---

### 🔹 5. Run Workloads

```bash
sudo ./engine start cpu1 ../rootfs-alpha /cpu_hog
sudo ./engine start io1 ../rootfs-beta /io_pulse
```

---

### 🔹 6. Check Logs

```bash
cat ../rootfs-beta/container.log
```

---

### 🔹 7. Stop Containers

```bash
sudo ./engine stop cpu1
sudo ./engine stop io1
```

---

## 🧪 Workloads

### 🔹 CPU-bound (`cpu_hog`)

* Runs an infinite loop
* Consumes high CPU

---

### 🔹 IO-bound (`io_pulse`)

* Prints output periodically
* Uses `sleep()`
* Lower CPU usage

---

## 📊 Scheduling Experiment

### Observation:

* CPU-bound process → ~100% CPU usage
* IO-bound process → low CPU usage

### Conclusion:

The Linux scheduler efficiently distributes CPU time based on process behavior, ensuring fairness.

---

## 📸 Screenshots Included

* Build success
* Container start
* Multi-container (`ps`) output
* CPU workload execution
* IO workload execution
* Scheduling (`top`)
* Logging (`cat container.log`)
* Container stop

---

## ⚠️ Limitations

* No namespace isolation
* No IPC-based supervisor
* Basic file-based container tracking
* Limited security isolation

---

## 💡 Future Enhancements

* Implement namespaces (`clone()`)
* Add IPC-based supervisor
* Resource limits (CPU/Memory)
* Advanced logging system
* Network isolation

---

## 🧠 Viva Preparation

### 🔹 What is `chroot()`?

Changes the root directory of a process, isolating filesystem access.

---

### 🔹 Why use `fork()`?

To create a new process for container execution.

---

### 🔹 Why logging is needed?

To track container output and debug execution.

---

### 🔹 CPU vs IO bound?

* CPU-bound → heavy computation
* IO-bound → waits for IO operations

---

### 🔹 Why compile inside container?

To avoid GLIBC compatibility issues.

---

## 🏁 Conclusion

This project demonstrates the core concepts behind container runtimes. By using simple Linux primitives, we successfully implemented a basic multi-container system with logging and workload execution, providing a strong foundation for understanding modern container technologies.

---



