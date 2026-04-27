# City Infrastructure Reporting System (Phase 1)

## Overview
`city_manager` is a C-based command-line application designed for the UNIX environment. It represents a reporting and monitoring system for city infrastructure issues (like damaged roads, broken lighting, flooding, etc.). 

This system securely stores reports on disk, enforces strict access control using UNIX file permissions (`chmod`, `stat`), and utilizes symbolic links to track active districts.

## Compilation
To compile the project, ensure all source files are linked together. Run the following command in your terminal:

```bash
gcc city_manager.c operations.c helpers.c -o city_manager
