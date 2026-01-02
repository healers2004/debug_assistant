# Test Suite for Automotive Failure Analyzer

This directory contains a comprehensive test suite for the `auto_analyze` tool, demonstrating V2 capabilities including signal handling, exit code handling, and root-cause analysis.

## Structure

```
test_programs/
├── README.md           # This file
├── run_tests.sh        # Test runner script
├── bin/                # Compiled test programs (created automatically)
├── segfault.c          # Causes SIGSEGV
├── abort.c             # Causes SIGABRT
├── normal_exit.c       # Exits normally with code 0
├── nonzero_exit.c      # Exits with non-zero code
├── unknown_signal.c    # Raises SIGKILL (unsupported signal)
├── sigfpe.c            # Causes SIGFPE (division by zero)
├── sigbus.c            # Attempts to cause SIGBUS (architecture-dependent)
└── enomem.c            # Attempts to trigger ENOMEM
```

## Running the Tests

### Prerequisites

1. Build the `auto_analyze` tool first:
   ```bash
   cd automotive_failure_analyzer
   make
   ```

2. Ensure you have `gcc` and `bash` installed.

### Execute All Tests

From the project root directory:

```bash
./test_programs/run_tests.sh
```

Or from within the `test_programs/` directory:

```bash
cd test_programs
./run_tests.sh
```

## Test Programs

### 1. `normal_exit.c`
- **Purpose**: Tests successful program execution
- **Expected**: "Program exited normally. No failure detected."
- **Exit Code**: 0

### 2. `nonzero_exit.c`
- **Purpose**: Tests detection of non-zero exit codes
- **Expected**: "Failure detected, but no terminating signal was reported."
- **Exit Code**: 1

### 3. `segfault.c`
- **Purpose**: Tests SIGSEGV detection and analysis
- **Expected**: Memory Corruption classification with SIGSEGV signal
- **Signal**: 11 (SIGSEGV)

### 4. `abort.c`
- **Purpose**: Tests SIGABRT detection and analysis
- **Expected**: Invalid State classification with SIGABRT signal
- **Signal**: 6 (SIGABRT)

### 5. `sigfpe.c`
- **Purpose**: Tests SIGFPE detection and analysis
- **Expected**: Invalid State classification with SIGFPE signal
- **Signal**: 8 (SIGFPE)

### 6. `sigbus.c`
- **Purpose**: Tests SIGBUS detection (may not trigger on all architectures)
- **Expected**: Memory Corruption classification if SIGBUS occurs
- **Signal**: 7 (SIGBUS)
- **Note**: Bus errors are architecture-dependent and may not occur on all systems

### 7. `unknown_signal.c`
- **Purpose**: Tests handling of unsupported signals
- **Expected**: "Unknown Failure" classification with signal 9 (SIGKILL)
- **Signal**: 9 (SIGKILL) - not in supported signal set

### 8. `enomem.c`
- **Purpose**: Tests ENOMEM detection (may not always trigger)
- **Expected**: Normal exit if ENOMEM is successfully triggered
- **Note**: System limits may prevent ENOMEM from occurring

### 9. Exec Failure Test
- **Purpose**: Tests handling of nonexistent programs
- **Expected**: "Failed to execute target program" error message
- **Program**: `/nonexistent/test/program`

## Test Output

The test suite provides:
- **Colored output** for easy reading (green for pass, red for fail)
- **Clear separators** between tests
- **Full analyzer output** for each test
- **Summary statistics** at the end

## Manual Testing

You can also run individual tests manually:

```bash
# Compile a test program
gcc -Wall -Wextra -std=c11 test_programs/segfault.c -o test_programs/bin/segfault

# Run the analyzer on it
./automotive_failure_analyzer/auto_analyze --run test_programs/bin/segfault
```

## Adding New Tests

To add a new test:

1. Create a new `.c` file in `test_programs/`
2. The test runner will automatically compile and run it
3. Ensure the test program is safe and won't crash the analyzer itself

## Notes

- All test programs are compiled with `-Wall -Wextra` for strict warnings
- The `bin/` directory is created automatically and contains compiled binaries
- Some tests (like `sigbus.c` and `enomem.c`) may not always trigger their intended conditions due to system-specific behavior
- The test suite is POSIX-compatible and should work on Linux, WSL, and other POSIX systems

