# City Infrastructure Reporting System

## Overview
This project is a suite of C-based applications designed for the UNIX environment, implementing a reporting and monitoring system for city infrastructure issues (such as damaged roads, broken lighting, and flooding). 

The system securely stores and organizes reports on disk, enforces strict access control using UNIX file permissions (`chmod`, `stat`), and uses symbolic links to track active districts. As the system evolves across phases, it introduces background process monitoring coordinated via signals, and inter-process communication using pipes and I/O redirection with `dup2`.

---

## Programs

The suite consists of three main components:

* **`city_manager` (Phase 1 & 2):** The core command-line tool for inspectors and managers to file, manage, and query infrastructure reports. 
* **`monitor_reports` (Phase 2):** A background process that listens for signals to monitor when new reports are added.  
* **`city_hub` (Phase 3):** An interactive command-line interface that acts as a central control hub to manage the monitor and aggregate district workload scores using child processes and pipes.  

---

## Compilation

To compile the project, run the following commands in your terminal to link the appropriate source files:

### Core System & Monitor
```bash
# Compile the main city manager along with its helper and operation modules
gcc city_manager.c helpers.c operations.c -o city_manager

# Compile the monitor program
gcc monitor_reports.c -o monitor_reports
```

### Phase 3 Utilities
```bash
# Compile the interactive hub
gcc city_hub.c -o city_hub

# Compile the external scorer utility
gcc scorer.c -o scorer
```

---

## Operations & Commands

### `city_manager`
The core reporting tool. Most commands require specifying a role and user, e.g., `--role manager --user alice`.

| Command | Arguments | Required Role | Description |
| :--- | :--- | :--- | :--- |
| **`--add`** | `<district_id>` | Inspector / Manager | Appends a new report. Notifies `monitor_reports` via `SIGUSR1`. |
| **`--list`** | `<district_id>` | Any | Lists all reports in a district. |
| **`--view`** | `<district_id> <report_id>` | Any | Prints full details of a specific report. |
| **`--remove_report`** | `<district_id> <report_id>` | Manager | Removes a single report from the system. |
| **`--update_threshold`** | `<district_id> <value>` | Manager | Updates the severity threshold in `district.cfg`. |
| **`--filter`** | `<district_id> <condition>` | Any | Filters and displays reports matching a specific condition. |
| **`--remove_district`** | `<district_id>` | Manager | *(Phase 2)* Deletes the entire district directory, contents, and symlink using a child process executing `rm -rf`. |

### `city_hub` (Interactive Interface)
Started by running `./city_hub`. Once inside the prompt, you can use the following commands:

* **`start_monitor`**: Forks a background process (`hub_mon`), which sets up a pipe and forks the `monitor_reports` program. The hub reads and displays the monitor's standard output dynamically.
* **`calculate_scores <list_of_districts>`**: Spawns a separate external scorer process for each district to calculate the sum of severity levels for each inspector. Outputs are collected by the hub via pipes using `dup2()` redirection and combined into a workload report.

---

## Usage Examples

### Phase 1 & 2: Managing Reports & Districts
```bash
# Add a report to the downtown district
./city_manager --role manager --user alice --add downtown

# List all reports in the downtown district
./city_manager --role inspector --user bob --list downtown

# Filter reports with a severity of 2 or higher
./city_manager --role inspector --user bob --filter downtown "severity:>=:2"

# Remove a specific report
./city_manager --role manager --user alice --remove_report downtown 17

# Remove an entire district
./city_manager --role manager --user alice --remove_district downtown
```

### Phase 2: Running the Standalone Monitor
```bash
# Starts the monitor, creating a hidden .monitor_pid file
./monitor_reports 
```

### Phase 3: Using the Hub
```bash
# Launch the interactive hub
./city_hub

# Inside the hub interface:
> start_monitor
> calculate_scores downtown uptown
```