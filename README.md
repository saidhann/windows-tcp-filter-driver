# 🌐 TCP Connection Monitor

A **Windows network monitoring application** that displays live TCP connections system-wide — similar to Sysinternals TCPView. Built across three layers: a **kernel-mode WDK driver** that intercepts TCP traffic at Ring 0, a **C++ communication library** that bridges kernel and userspace via IOCTL, and a **Qt desktop UI** that presents connection data in a real-time table.

> ⚠️ **Kernel-mode software.** This driver runs at Ring 0. A bug can cause a system crash (BSOD). Only deploy on test machines with kernel debugging enabled, or inside a virtual machine.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [How It Works](#how-it-works)
  - [Kernel Driver](#kernel-driver)
  - [Communication Library](#communication-library)
  - [Qt UI](#qt-ui)
- [Prerequisites](#prerequisites)
- [Building](#building)
  - [Kernel Driver & Communication Library](#kernel-driver--communication-library)
  - [Qt UI](#qt-ui-1)
- [Deployment](#deployment)
  - [Test Signing Setup](#test-signing-setup)
  - [Installing the Driver](#installing-the-driver)
  - [Running the Application](#running-the-application)
- [Development Environment](#development-environment)
- [Safety & Testing](#safety--testing)
- [License](#license)

---

## Overview

TCP Connection Monitor hooks into the Windows networking stack at the kernel level to collect real-time TCP connection data. The Qt frontend displays this data in a live, auto-refreshing table showing every active connection on the system — local and remote addresses, ports, connection state, and owning process — much like Sysinternals TCPView, but built from scratch with a custom kernel driver.

This project spans the full stack from **Ring 0 kernel code** to a **desktop GUI**, demonstrating Windows kernel-mode driver development, IOCTL-based kernel/userspace communication, and Qt application development within a single cohesive system.

---

## Features

- 🖥️ **Live connection table** — real-time view of all active TCP connections system-wide
- 📋 **Per-connection details** — local address, remote address, local port, remote port, connection state, owning process
- 🔄 **Auto-refresh** — connection list updates continuously as connections are established and closed
- 🔌 **Kernel-level data collection** — data sourced directly from the Windows networking stack via a WDK driver, not from user-mode APIs
- 🪟 **Qt desktop UI** — clean native Windows GUI built with the Qt framework
- 📡 **IOCTL communication bridge** — `CommunicationLibrary` decouples the UI from low-level kernel handle management
- 🏗️ **Three-layer architecture** — kernel driver, userspace library, and GUI are fully separated into independent projects

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Space (Ring 3)                      │
│                                                                 │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │                        Qt UI                              │  │
│  │   ┌──────────────────────────────────────────────────┐   │  │
│  │   │  Live Connection Table                           │   │  │
│  │   │  Local Addr   Remote Addr   Port  State  Process │   │  │
│  │   │  ──────────   ───────────   ────  ─────  ─────── │   │  │
│  │   │  192.168.1.5  142.250.1.1   443   ESTAB  chrome  │   │  │
│  │   │  127.0.0.1    127.0.0.1     8080  LISTEN  nginx  │   │  │
│  │   │  ...                                             │   │  │
│  │   └──────────────────────────────────────────────────┘   │  │
│  └───────────────────────┬───────────────────────────────────┘  │
│                          │                                      │
│  ┌───────────────────────▼───────────────────────────────────┐  │
│  │                 CommunicationLibrary                      │  │
│  │         C++ IOCTL wrapper — DeviceIoControl()             │  │
│  └───────────────────────┬───────────────────────────────────┘  │
└──────────────────────────┼──────────────────────────────────────┘
                           │  IOCTL
           ════════════════╪════════════════  Ring 0 boundary
                           │
┌──────────────────────────┼──────────────────────────────────────┐
│                   Kernel Space (Ring 0)                         │
│                                                                 │
│  ┌───────────────────────▼───────────────────────────────────┐  │
│  │                   KernelDriver.sys                        │  │
│  │                                                           │  │
│  │   Device Object ──► IRP Dispatch ──► IOCTL Handler       │  │
│  │                                           │               │  │
│  │                         Network Filter ◄──┘               │  │
│  │                         (WFP Callout)                     │  │
│  └───────────────────────────────────────────────────────────┘  │
│                          │                                      │
│              ┌───────────▼────────────┐                         │
│              │  Windows TCP/IP Stack  │                         │
│              │      (tcpip.sys)       │                         │
│              └────────────────────────┘                         │
└─────────────────────────────────────────────────────────────────┘
```

---

## Project Structure

```
tcp-connection-monitor/
├── KernelDriver/             # Ring 0 — WDK kernel-mode driver
│   └── *.cpp / *.c           # DriverEntry, WFP filter callbacks, IOCTL dispatch
│
├── ComunicationLibrary/      # Ring 3 — IOCTL wrapper library
│   └── *.cpp / *.hpp         # Opens device handle, wraps DeviceIoControl calls
│
├── QtUI/                     # Ring 3 — Qt desktop frontend
│   ├── CMakeLists.txt        # Qt CMake build
│   └── *.cpp / *.h           # Main window, connection table model, refresh timer
│
└── KernelDriver.sln          # Visual Studio solution
```

---

## How It Works

### Kernel Driver

`KernelDriver.sys` is a Windows kernel-mode driver built with the WDK. On load it:

1. Creates a named **device object** (`\Device\KernelDriver`) and a symbolic link so userspace code can open a handle to it
2. Registers a **Windows Filtering Platform (WFP) callout** at the network layer — this callback fires on every TCP connection event (connect, established, close)
3. Maintains an internal list of active connections with their metadata: local/remote address, port, state, and owning process ID
4. Handles **IOCTL requests** from `CommunicationLibrary` — responding with the current connection snapshot when polled

### Communication Library

`ComunicationLibrary` is a userspace C++ library that wraps the kernel interaction into a clean API:

```cpp
// Open connection to the driver
CommunicationLibrary comm;
comm.open();

// Poll current TCP connection list
std::vector<TcpConnection> connections = comm.getConnections();

// Each TcpConnection contains:
//   localAddress, remoteAddress, localPort, remotePort, state, processId
```

The library handles opening the device handle, constructing IOCTL input/output buffers, and marshalling the raw kernel data into C++ structs. The Qt UI never touches a Windows kernel handle directly.

### Qt UI

The Qt frontend opens a `CommunicationLibrary` instance on startup and uses a `QTimer` to poll for updated connection data at a configurable interval. Each poll result is compared against the previous snapshot:

- **New connections** are added to the table (optionally highlighted)
- **Closed connections** are removed or greyed out
- **Changed state** connections are updated in place

The main window displays a sortable `QTableView` with columns for local address, remote address, local port, remote port, connection state, and process name (resolved from the process ID via `OpenProcess` / `QueryFullProcessImageName`).

---

## Prerequisites

### Development Machine

- **Windows 10/11** (x64)
- **Visual Studio 2022** with "Desktop development with C++" workload
- **Windows Driver Kit (WDK)** — version matching Visual Studio
  - Download: https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk
- **Qt 6** (or Qt 5) with MSVC kit
  - Download: https://www.qt.io/download
- **CMake ≥ 3.16** (bundled with Visual Studio or installed separately)

### Test / Deployment Machine

- **Windows 10/11** (x64)
- **Test signing enabled** — required for unsigned development driver builds (see below)
- Recommended: a **virtual machine** (Hyper-V, VMware, VirtualBox) for safe driver testing

---

## Building

### Kernel Driver & Communication Library

1. Open `KernelDriver.sln` in Visual Studio 2022
2. Ensure the WDK extension is installed (the driver project will fail to load without it)
3. Select `Release | x64`
4. Build the solution — produces:
   - `KernelDriver.sys` — kernel-mode driver binary
   - `ComunicationLibrary.lib` — static library for linking into the Qt app

### Qt UI

```bash
cd QtUI
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
cmake --build . --config Release
```

Or open the `QtUI/CMakeLists.txt` directly in Qt Creator and build from there.

> **Note:** Link the Qt project against `ComunicationLibrary.lib` from the VS build output. Ensure the `KernelDriver.sys` binary is available for deployment alongside the Qt executable.

---

## Deployment

### Test Signing Setup

Development builds are unsigned. Enable test signing mode on the target machine:

```cmd
:: Run as Administrator
bcdedit /set testsigning on
shutdown /r /t 0
```

A "Test Mode" watermark will appear on the desktop after reboot. Revert with:

```cmd
bcdedit /set testsigning off
```

> ⚠️ Only enable test signing on dedicated test machines — never on production systems.

### Installing the Driver

Copy `KernelDriver.sys` to the target machine and register it as a service:

```cmd
:: Run as Administrator
sc create KernelDriver type= kernel binPath= "C:\Path\To\KernelDriver.sys"
sc start KernelDriver
```

Verify it loaded successfully:

```cmd
sc query KernelDriver
```

To stop and remove:

```cmd
sc stop KernelDriver
sc delete KernelDriver
```

### Running the Application

Once the driver is loaded, launch the Qt application (run as Administrator — required to open the kernel device handle):

```
TcpConnectionMonitor.exe
```

The connection table will populate immediately and refresh automatically.

---

## Development Environment

For kernel-mode development it is strongly recommended to use a **two-machine debugging setup**:

- **Host** — runs Visual Studio 2022 + WinDbg
- **Target** (VM) — runs the driver under test

Enable kernel debugging over the network on the target:

```cmd
:: On the target VM — run as Administrator
bcdedit /debug on
bcdedit /dbgsettings net hostip:<host-ip> port:50000 key:1.2.3.4
```

Then attach WinDbg on the host. This lets you set breakpoints in kernel code, inspect memory, and catch BSODs before they reboot the VM with `!analyze -v`.

---

## Safety & Testing

Kernel-mode bugs manifest as **Blue Screen of Death (BSOD)** crashes with no warning. Follow these practices:

- **Always develop in a VM** — snapshot before loading a new driver build so you can roll back instantly
- **Enable Driver Verifier** on the test machine to catch memory corruption and IRQL violations at runtime:
  ```cmd
  verifier /standard /driver KernelDriver.sys
  ```
- **Use Debug builds** during development — WDK Debug builds include additional runtime assertions
- **Save crash dumps** — configure the test machine to write kernel memory dumps on crash:
  *System Properties → Advanced → Startup and Recovery → Kernel memory dump*
- **Never test on your primary machine** without a full backup

---

## License

This project does not currently specify a license. Please contact the author before using or distributing this work.

---

> Built using C/C++, the Windows Driver Kit, Qt, and a healthy respect for Ring 0.
