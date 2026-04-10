# Win Network Driver - GUI

## Overview

The **Win Network Driver - GUI** is a Windows application that visually presents detailed information about all active TCP connections on your system. It provides a real-time, user-friendly interface that allows you to monitor network activity, displaying various details such as local and remote IP addresses, ports, and data transfer statistics. This tool is particularly useful for network administrators and users who want to monitor their network connections without the need for installation or system reboot.

## Features

### Primary Features
- **Real-Time Connection Monitoring:**
  - Displays information about all active TCP connections.
  - Shows local and remote IP addresses, ports, process names, and IDs.
  - Monitors data sent and received for each connection.
  
- **User-Defined Update Interval:**
  - The data in the GUI automatically refreshes at a user-specified interval.

### Additional Features (Optional)
- **IPv6 Support:**
  - Monitors both IPv4 and IPv6 connections.

- **Connection Management:**
  - Allows users to close selected TCP connections directly from the GUI.

- **Pre-Startup Connection Display:**
  - Displays connections that were active before the application was started.

- **TCP Connection States:**
  - Displays the current state of TCP connections (e.g., LISTEN, ESTABLISHED, CLOSE-WAIT, etc.).

- **UDP Endpoint Monitoring:**
  - Provides information about active UDP endpoints.

- **DNS Name Resolution:**
  - Optionally displays DNS names instead of IP addresses.

## User Interface Design

### Key UI Components
- **Connection Table:**
  - Displays a list of active TCP connections with columns for process name, process ID, local IP, local port, remote IP, remote port, bytes sent, and bytes received.
  
- **Update Interval Control:**
  - An input field or slider for users to set the update interval for refreshing the connection data.

- **Connection Management Buttons:**
  - A "Close Connection" button for terminating the selected TCP connection.
  
- **Status Indicators:**
  - Icons or text labels indicating the current state of each connection (e.g., LISTEN, ESTABLISHED).

- **Settings Panel:**
  - Options to toggle IPv6 support, display UDP endpoints, and choose between IP addresses or DNS names.

### UI Design Mockups

#### Design 1
![Design 1](images/design1.png)

## How to Use

1. **Launching the Application:**
   - Run the executable file. No installation or system reboot is required.

2. **Monitoring Connections:**
   - The main table will display all active TCP connections, updating at the interval you specify.
   
3. **Adjusting Settings:**
   - Use the settings panel to toggle additional features such as IPv6 support, UDP endpoint display, or DNS resolution.

4. **Managing Connections:**
   - Select a connection from the table and click "Close Connection" to terminate it.

For more detailed usage instructions or troubleshooting, please refer to the in-app help section or documentation.
