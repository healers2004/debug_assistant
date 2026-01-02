# Automotive Failure Analyzer

A C11, POSIX-only CLI tool for analyzing Linux process failures using deterministic, rule-based logic. Combines POSIX signals, errno values, and log file keywords to classify failures and provide actionable debugging steps.

## Version History

- **v1.0**: Manual analysis mode - accepts signals, errno, and log files as input
- **v2.0**: Runtime supervision mode - runs programs and automatically extracts failure metadata

## Design Philosophy

The architecture mirrors real debugging workflows: check crash signal first (most definitive), then errno (context-specific), then logs (fallback evidence).

## Building

### Prerequisites

- C11 compatible compiler (gcc recommended)
- POSIX-compliant system (Linux, WSL, etc.)
- Make utility

### Compilation

```bash
cd automotive_failure_analyzer
make
```

This creates the `auto_analyze` binary.

### Clean Build Artifacts

```bash
make clean
```

## Usage

### V1: Manual Analysis Mode

Analyze failures from manually provided evidence (signals, errno, logs).

#### Basic Usage

```bash
./auto_analyze -s <signal> -e <errno> -l <log_file>
```

#### Options

- `-s <int>`: Signal number (e.g., 11 for SIGSEGV)
- `-e <int>`: Errno value (e.g., 14 for EFAULT)
- `-l <path>`: Path to log file

At least one of `-s`, `-e`, or `-l` must be provided.

#### V1 Examples

```bash
# Analyze SIGSEGV with EFAULT
./auto_analyze -s 11 -e 14

# Analyze with log file
./auto_analyze -s 6 -l error.log

# Analyze errno only
./auto_analyze -e 12
```

#### V1 Output Example

```
=== Failure Analysis Report ===

Failure Type: Memory Corruption
Root Cause:   Invalid memory access - bad address (EFAULT)

Debug Steps:
1. Run with valgrind: valgrind --leak-check=full <program>
2. Use AddressSanitizer: gcc -fsanitize=address <sources>
3. Check stack traces with gdb: gdb <program> core
4. Review pointer arithmetic and array bounds
================================
```

### V2: Runtime Supervision Mode

Run programs and automatically observe their termination behavior.

#### Basic Usage

```bash
./auto_analyze --run <program> [args...]
```

#### V2 Examples

```bash
# Run a program and analyze its termination
./auto_analyze --run /path/to/program arg1 arg2

# Run with test program (see Test Suite section)
./auto_analyze --run test_programs/bin/segfault

# Successful program execution
./auto_analyze --run /bin/echo "Hello, World!"
# Output: Program exited normally. No failure detected.

# Program that crashes
./auto_analyze --run test_programs/bin/segfault
# Output: Analyzes SIGSEGV and provides failure report
```

#### V2 Output Examples

**Successful Execution:**
```
Program exited normally. No failure detected.
```

**Signal Termination:**
```
Observed Termination:
- Signal: 11 (SIGSEGV)
- Core dump: yes

=== Failure Analysis Report ===
...
```

**Unknown Signal:**
```
Observed Termination:
- Signal: 9 (Unknown)
- Core dump: no

Unknown signal encountered (signal 9)

=== Failure Analysis Report ===
Failure Type: Unknown Failure
...
```

**Non-Zero Exit Code:**
```
Observed Termination:
- Exit code: 1
- Signal: none

Failure detected, but no terminating signal was reported.
Classification: Unknown Failure
```

### Combining V1 and V2 Options

V1 options (`-s`, `-e`, `-l`) can be used alongside `--run` for additional context:

```bash
# Run program and also analyze a log file
./auto_analyze --run /path/to/program -l error.log
```

## Supported Signals

- **SIGSEGV (11)**: Segmentation fault - invalid memory access
- **SIGABRT (6)**: Abort signal - abnormal termination
- **SIGBUS (7)**: Bus error - invalid memory access alignment
- **SIGFPE (8)**: Floating-point exception - arithmetic error

## Supported Errno Values

- **EFAULT (14)**: Bad address → Memory Corruption
- **EINVAL (22)**: Invalid argument → Invalid State
- **ENOMEM (12)**: Out of memory → Resource Exhaustion
- **EPIPE (32)**: Broken pipe → Invalid State

## Failure Types

The tool classifies failures into four categories:

1. **Memory Corruption**: Invalid memory access (SIGSEGV, SIGBUS, EFAULT)
2. **Invalid State**: Invalid operation or state violation (SIGABRT, SIGFPE, EINVAL, EPIPE)
3. **Resource Exhaustion**: System resource limit exceeded (ENOMEM)
4. **Timing/Race**: Concurrency issues (deadlock, timeout keywords in logs)

## Edge Case Handling

### Unknown Signals

If a signal is not in the supported set, the tool reports:

```
Unknown signal encountered (signal N)
Failure Type: Unknown Failure
```

### Exec Failures

If a program cannot be launched:

```
Failed to execute target program.
Reason: Command not found or exec failed
```

### Normal Exits

Programs that exit with code 0 are reported as successful:

```
Program exited normally. No failure detected.
```

### Non-Zero Exit Codes

Programs that exit with non-zero codes but no signal:

```
Failure detected, but no terminating signal was reported.
Classification: Unknown Failure
```

## Test Suite

A comprehensive test suite is provided in the `test_programs/` directory.

### Running Tests

```bash
# From project root
./test_programs/run_tests.sh

# Or from test_programs directory
cd test_programs
./run_tests.sh
```

### Test Programs

The suite includes 9 test programs:

1. **normal_exit.c**: Exits successfully (code 0)
2. **nonzero_exit.c**: Exits with non-zero code
3. **segfault.c**: Causes SIGSEGV (Memory Corruption)
4. **abort.c**: Causes SIGABRT (Invalid State)
5. **sigfpe.c**: Causes SIGFPE (Invalid State)
6. **sigbus.c**: Attempts to cause SIGBUS (architecture-dependent)
7. **unknown_signal.c**: Raises SIGKILL (unsupported signal)
8. **enomem.c**: Attempts to trigger ENOMEM (system-dependent)
9. **Exec failure test**: Tests nonexistent program handling

### Test Suite Features

- Automatic compilation of all test programs
- Colored output for easy reading
- Full analyzer output for each test
- Summary statistics (total, passed, failed)

See `test_programs/README.md` for detailed documentation.

## Project Structure

```
automotive_failure_analyzer/
├── README.md              # This file
├── Makefile              # Build system
├── include/              # Header files
│   ├── signal_analyzer.h
│   ├── errno_mapper.h
│   ├── log_parser.h
│   ├── failure_rules.h
│   └── process_runner.h (V2)
├── src/                  # Source files
│   ├── main.c
│   ├── signal_analyzer.c
│   ├── errno_mapper.c
│   ├── log_parser.c
│   ├── failure_rules.c
│   └── process_runner.c (V2)
└── auto_analyze          # Compiled binary

test_programs/            # Test suite (in project root)
├── README.md
├── run_tests.sh
├── bin/                  # Compiled test programs
└── *.c                   # Test program sources
```

## Module Overview

### signal_analyzer
Maps POSIX signal numbers to human-readable descriptions. Uses static lookup table for O(n) search (n=4, acceptable).

### errno_mapper
Maps system call error codes to failure categories. Simple switch statement for explicit, compiler-optimized mapping.

### log_parser
Extracts failure-related keywords from log files using case-insensitive `strstr()` matching. Returns boolean flags for rule engine.

### failure_rules
Combines signal, errno, and log data using 9 explicit rules. Priority: signals → errno → logs. Produces failure type, root cause, and actionable debug steps.

### process_runner (V2)
Runs target programs using `fork()`, `execvp()`, and `waitpid()`. Extracts termination metadata (exit codes, signals, core dumps).

### main
CLI interface and orchestration. Manual argument parsing to handle `--run` consuming remaining arguments. Integrates all modules.

## Debug Steps

The tool provides real, actionable debugging commands:

- **Memory Corruption**: `valgrind --leak-check=full`, `gcc -fsanitize=address`, `gdb <program> core`
- **Invalid State**: `strace`, `ulimit -c unlimited`, review assertions
- **Resource Exhaustion**: `ulimit -v`, `top`, `ps aux`, check for leaks
- **Timing/Race**: `gcc -fsanitize=thread`, review mutexes, check deadlocks

## Backward Compatibility

V2 is fully backward compatible with V1:
- All V1 command-line options (`-s`, `-e`, `-l`) work unchanged
- All V1 analysis logic unchanged
- All V1 output format unchanged
- V1 commands produce identical results

## Requirements

- C11 compatible compiler (gcc recommended)
- POSIX-compliant system (Linux, WSL, macOS with POSIX support)
- Make utility
