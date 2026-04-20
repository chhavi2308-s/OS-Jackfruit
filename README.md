# 🐳 Lightweight Multi-Container Runtime in C

A lightweight Linux container runtime in C with a long-running supervisor and a kernel-space memory monitor.

---

# 👩‍💻 Team Information

## Team Members

**Member 1:** Chhavi S Wadhwa
**SRN:** PES1UG24CS132

**Member 2:** Chetha Samrutha
**SRN:** PES1UG24CS131

---
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

1. Build Success
   <img width="759" height="262" alt="image" src="https://github.com/user-attachments/assets/37aa7d0e-e46b-471a-ab70-da6a97599e42" />

2. Container Start
<img width="916" height="112" alt="image" src="https://github.com/user-attachments/assets/02836471-0b63-4e73-a915-72a374a4d204" />

3. Multi-Container (ps) Output
<img width="900" height="94" alt="image" src="https://github.com/user-attachments/assets/296d7547-1bfb-4d3c-bd4e-a57cbbec1673" />

4. CPU Workload Execution
<img width="913" height="112" alt="image" src="https://github.com/user-attachments/assets/2e6f6c69-2839-4e67-a6db-5dab401fe75e" />

5. IO Workload Execution
<img width="902" height="73" alt="image" src="https://github.com/user-attachments/assets/972bd643-ecc1-46da-bfc7-8a2967418723" />

6. Scheduling Experiment (top)
<img width="741" height="163" alt="image" src="https://github.com/user-attachments/assets/8f1d5b7d-abb5-4aa9-a8d5-a5d23cc10b17" />
<img width="740" height="449" alt="image" src="https://github.com/user-attachments/assets/2be06829-0882-4d18-9946-788bcd38aa00" />

7. Logging (cat container.log)
<img width="915" height="420" alt="image" src="https://github.com/user-attachments/assets/88f50533-0418-4951-ab86-cea13e54dff5" />

8. Container Stop
<img width="914" height="509" alt="image" src="https://github.com/user-attachments/assets/7689a8b3-0240-4489-8e0b-00d6d5e45fef" />

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



