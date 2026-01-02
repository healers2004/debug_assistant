# Automotive Failure Analyzer

A C-based CLI tool for analyzing Linux process failures in automotive subsystems.

## Overview

This tool accepts POSIX signals, errno values, and log files to output failure classifications and root causes using rule-based logic.

## Building

To build the project, run:

```bash
make
```

This will create the `auto_analyze` binary.

To clean build artifacts:

```bash
make clean
```

## Usage

```bash
./auto_analyze -s <signal> -e <errno> -l <log_file>
```

### Options

- `-s <int>`: Signal number (e.g., 11 for SIGSEGV)
- `-e <int>`: Errno value (e.g., 14 for EFAULT)
- `-l <path>`: Path to log file

### Example

```bash
./auto_analyze -s 11 -e 14 -l error.log
```

## Supported Signals

- SIGSEGV (11): Segmentation fault
- SIGABRT (6): Abort signal
- SIGBUS (7): Bus error
- SIGFPE (8): Floating-point exception

## Supported Errno Values

- EFAULT (14): Bad address
- EINVAL (22): Invalid argument
- ENOMEM (12): Out of memory
- EPIPE (32): Broken pipe

## Failure Types

- Memory Corruption
- Invalid State
- Resource Exhaustion
- Timing/Race

## Requirements

- C11 compatible compiler (gcc recommended)
- POSIX-compliant system
- Make utility