# Multi-Container Runtime

A lightweight Linux container runtime in C with a long-running supervisor and a kernel-space memory monitor.

---

# 1. Team Information

## Team Members

**Member 1:** Chhavi S Wadhwa
**SRN:** PES1UG24CS132

**Member 2:** Chetha Samrutha
**SRN:** PES1UG24CS131

---

# 2. Project Summary

This project implements a lightweight Linux container runtime in C with:

* A long-running parent supervisor
* Multi-container management
* Supervisor CLI using IPC
* Bounded-buffer concurrent logging
* Kernel-space memory monitoring using an LKM
* Soft and hard memory limit enforcement
* Linux scheduling experiments

The project is divided into two major parts:

## User-Space Runtime + Supervisor (`engine.c`)

This component:

* Launches isolated containers using Linux namespaces
* Maintains metadata for multiple containers
* Accepts CLI commands (`start`, `run`, `ps`, `logs`, `stop`)
* Captures stdout/stderr using pipes
* Implements bounded-buffer logging using producer-consumer synchronization
* Handles lifecycle management and signal delivery

## Kernel-Space Monitor (`monitor.c`)

This component:

* Implements a Linux Kernel Module (LKM)
* Creates `/dev/container_monitor`
* Tracks registered container PIDs
* Monitors memory usage using periodic RSS checks
* Enforces:

  * Soft memory limit → warning only
  * Hard memory limit → SIGKILL

---

# 3. Build, Load, and Run Instructions

This project is designed for:

* Ubuntu 22.04 / 24.04 VM
* Secure Boot OFF
* No WSL

---

## Step 1 — Install Dependencies

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

---

## Step 2 — Run Environment Check

```bash
cd boilerplate
chmod +x environment-check.sh
sudo ./environment-check.sh
```

Fix any reported issues before continuing.

---

## Step 3 — Prepare Root Filesystem

```bash
cd ~/container-project/OS-Jackfruit

mkdir rootfs-base

wget https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/x86_64/alpine-minirootfs-3.20.3-x86_64.tar.gz

tar -xzf alpine-minirootfs-3.20.3-x86_64.tar.gz -C rootfs-base
```

Create writable copies for containers:

```bash
cp -a ./rootfs-base ./rootfs-alpha
cp -a ./rootfs-base ./rootfs-beta
```

Do not commit these folders to GitHub.

---

## Step 4 — Build Project

```bash
cd boilerplate
make
```

This builds:

* engine
* monitor.ko
* cpu_hog
* io_pulse
* memory_hog

---

## Step 5 — Load Kernel Module

```bash
sudo insmod monitor.ko
```

Verify device creation:

```bash
ls -l /dev/container_monitor
```

Expected:

```bash
/dev/container_monitor
```

---

## Step 6 — Start Supervisor

```bash
sudo ./engine supervisor ./rootfs-base
```

Expected:

```bash
Supervisor running...
```

Keep this terminal open.

---

## Step 7 — Start Containers (New Terminal)

```bash
cd ~/container-project/OS-Jackfruit/boilerplate

sudo ./engine start alpha ../rootfs-alpha /bin/sh --soft-mib 48 --hard-mib 80

sudo ./engine start beta ../rootfs-beta /bin/sh --soft-mib 64 --hard-mib 96
```

Expected:

```bash
Started container alpha (PID xxxx)
Started container beta (PID xxxx)
```

---

## Step 8 — View Running Containers

```bash
sudo ./engine ps
```

Expected:

```bash
ID      PID      STATUS
alpha   xxxx     RUNNING
beta    xxxx     RUNNING
```

---

## Step 9 — View Logs

```bash
sudo ./engine logs alpha
```

Or directly:

```bash
cat logs/alpha.log
```

Expected:

```bash
hello from container
hello from container
```

---

## Step 10 — Run Memory Test

Copy workload:

```bash
cp memory_hog ../rootfs-alpha/
```

Launch:

```bash
sudo ./engine start memtest ../rootfs-alpha /memory_hog --soft-mib 64 --hard-mib 96
```

Check kernel logs:

```bash
dmesg | tail
```

Expected:

```bash
Monitor: Soft limit exceeded
Monitor: Killing PID XXXX (HARD limit)
```

---

## Step 11 — Run Scheduling Experiment

Copy workloads:

```bash
cp cpu_hog ../rootfs-alpha/
cp io_pulse ../rootfs-beta/
```

Launch:

```bash
sudo ./engine start cpu ../rootfs-alpha /cpu_hog --nice 5

sudo ./engine start io ../rootfs-beta /io_pulse --nice -5
```

Observe:

```bash
top
```

or

```bash
ps -eo pid,ni,comm
```

This demonstrates scheduler behavior.

---

## Step 12 — Stop Containers

```bash
sudo ./engine stop alpha
sudo ./engine stop beta
sudo ./engine stop cpu
sudo ./engine stop io
```

---

## Step 13 — Cleanup

Check zombies:

```bash
ps aux | grep defunct
```

Inspect logs:

```bash
dmesg | tail
```

Unload kernel module:

```bash
sudo rmmod monitor
```

---

# 4. Demo with Screenshots

Each screenshot must include a short caption.

---

## Screenshot 1 — Multi-Container Supervision

Show:

* supervisor running
* two or more containers active

Command:

```bash
sudo ./engine ps
```

Caption:

Two containers managed by a single long-running supervisor.

---

## Screenshot 2 — Metadata Tracking

Show:

```bash
sudo ./engine ps
```

with:

* container ID
* PID
* state
* memory limits

Caption:

Supervisor metadata table showing tracked containers.

---

## Screenshot 3 — Bounded-Buffer Logging

Show:

```bash
cat logs/alpha.log
```

Caption:

Persistent per-container logging through bounded-buffer producer-consumer pipeline.

---

## Screenshot 4 — CLI and IPC

Show:

```bash
sudo ./engine start alpha ...
```

and supervisor response.

Caption:

CLI command sent to supervisor through UNIX socket IPC.

---

## Screenshot 5 — Soft-Limit Warning

Show:

```bash
dmesg | tail
```

with soft limit warning.

Caption:

Kernel monitor warning generated after soft memory limit exceeded.

---

## Screenshot 6 — Hard-Limit Enforcement

Show:

```bash
dmesg | tail
```

with kill event.

Caption:

Container terminated automatically after exceeding hard memory limit.

---

## Screenshot 7 — Scheduling Experiment

Show:

```bash
top
```

or

```bash
ps -eo pid,ni,comm
```

Caption:

CPU-bound and I/O-bound workloads under different scheduling priorities.

---

## Screenshot 8 — Clean Teardown

Show:

```bash
ps aux | grep defunct
```

Caption:

No zombie processes remain after shutdown and cleanup.

---

# 5. Engineering Analysis

---

## A. Isolation Mechanisms

Process isolation is achieved using:

* PID namespace
* UTS namespace
* Mount namespace

Filesystem isolation is implemented using:

* chroot()

Each container sees its own root filesystem and its own `/proc`.

The host kernel is still shared across all containers.

This provides lightweight virtualization without full VMs.

---

## B. Supervisor and Process Lifecycle

A long-running supervisor is useful because it:

* manages all containers centrally
* tracks metadata
* handles signals
* reaps child processes
* prevents zombies

It improves reliability compared to launching one container and exiting.

---

## C. IPC, Threads, and Synchronization

Two IPC paths are used:

### Path A — Logging

Container → Supervisor using pipes

### Path B — Control

CLI → Supervisor using UNIX domain socket

Synchronization uses:

* mutex
* condition variables

Without synchronization:

* logs may be lost
* metadata may corrupt
* deadlocks may occur

Bounded-buffer design ensures safe producer-consumer behavior.

---

## D. Memory Management and Enforcement

RSS measures:

* physical memory actually used

It does not measure:

* swapped memory
* shared kernel memory

Soft limit:

warning only

Hard limit:

forced termination

Kernel-space enforcement is required because only the kernel has reliable access to process memory state.

---

## E. Scheduling Behavior

Linux scheduling balances:

* fairness
* responsiveness
* throughput

CPU-bound workloads consume more CPU time.

I/O-bound workloads sleep often and wake frequently.

Nice values influence priority.

Lower nice value → higher priority.

This behavior was observed during experiments.

---

# 6. Design Decisions and Tradeoffs

---

## Namespace Isolation

### Choice

Used chroot + namespaces

### Tradeoff

Simpler than pivot_root but slightly weaker isolation

### Justification

Faster development and sufficient for assignment requirements

---

## Supervisor Architecture

### Choice

Single persistent parent supervisor

### Tradeoff

More complex state management

### Justification

Required for multi-container management

---

## IPC + Logging

### Choice

UNIX socket + pipes + bounded buffer

### Tradeoff

Higher synchronization complexity

### Justification

Separates control path and logging path cleanly

---

## Kernel Monitor

### Choice

LKM with ioctl registration

### Tradeoff

Kernel debugging is harder

### Justification

Required for correct memory enforcement

---

## Scheduling Experiments

### Choice

cpu_hog + io_pulse + nice values

### Tradeoff

Simple workloads but clear observations

### Justification

Easy to reproduce and demonstrates scheduler behavior well

---

# 7. Scheduler Experiment Results

| Workload | Nice Value | Behavior        |
| -------- | ---------: | --------------- |
| cpu_hog  |          5 | High CPU usage  |
| io_pulse |         -5 | Faster response |

Observation:

* lower nice value received better scheduling priority
* CPU-bound process consumed more CPU share
* I/O-bound process remained responsive

This demonstrates Linux fairness and responsiveness goals.

---

# 8. Submission Files

Repository contains:

* engine.c
* monitor.c
* monitor_ioctl.h
* cpu_hog.c
* io_pulse.c
* memory_hog.c
* Makefile
* README.md

CI-safe smoke check:

```bash
make -C boilerplate ci
```

This verifies compilation on GitHub Actions.

---
