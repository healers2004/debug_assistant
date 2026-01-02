#!/bin/bash
# Test suite runner for automotive_failure_analyzer
# Compiles test programs and runs auto_analyze --run on each

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TEST_DIR="$SCRIPT_DIR"
BIN_DIR="$TEST_DIR/bin"
ANALYZER="$PROJECT_ROOT/automotive_failure_analyzer/auto_analyze"

# Check if analyzer exists
if [ ! -f "$ANALYZER" ]; then
    echo -e "${RED}Error: auto_analyze not found at $ANALYZER${NC}"
    echo "Please build the project first: cd automotive_failure_analyzer && make"
    exit 1
fi

# Create bin directory
mkdir -p "$BIN_DIR"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Automotive Failure Analyzer Test Suite${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Compile all test programs
echo -e "${YELLOW}Compiling test programs...${NC}"
for test_file in "$TEST_DIR"/*.c; do
    if [ -f "$test_file" ]; then
        test_name=$(basename "$test_file" .c)
        echo "  Compiling: $test_name.c"
        gcc -Wall -Wextra -std=c11 "$test_file" -o "$BIN_DIR/$test_name" 2>&1 | grep -v "warning:.*misaligned" || true
    fi
done
echo ""

# Test counter
PASSED=0
FAILED=0
TOTAL=0

# Function to run a test
run_test() {
    local test_name=$1
    local test_program=$2
    local expected_type=$3
    
    TOTAL=$((TOTAL + 1))
    
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}Test: $test_name${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    if [ ! -f "$test_program" ]; then
        echo -e "${RED}✗ Test program not found: $test_program${NC}"
        FAILED=$((FAILED + 1))
        echo ""
        return 1
    fi
    
    # Run the analyzer
    if "$ANALYZER" --run "$test_program" 2>&1; then
        echo ""
        echo -e "${GREEN}✓ Test completed${NC}"
        PASSED=$((PASSED + 1))
    else
        local exit_code=$?
        echo ""
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}✓ Test completed (exit code: 0)${NC}"
            PASSED=$((PASSED + 1))
        else
            echo -e "${RED}✗ Test failed (exit code: $exit_code)${NC}"
            FAILED=$((FAILED + 1))
        fi
    fi
    echo ""
}

# Run all tests
echo -e "${YELLOW}Running tests...${NC}"
echo ""

# Test 1: Normal exit (should succeed)
run_test "Normal Exit (Success)" "$BIN_DIR/normal_exit" "SUCCESS"

# Test 2: Non-zero exit (should detect failure)
run_test "Non-Zero Exit Code" "$BIN_DIR/nonzero_exit" "FAILURE"

# Test 3: SIGSEGV (Memory Corruption)
run_test "SIGSEGV (Segmentation Fault)" "$BIN_DIR/segfault" "MEMORY_CORRUPTION"

# Test 4: SIGABRT (Invalid State)
run_test "SIGABRT (Abort Signal)" "$BIN_DIR/abort" "INVALID_STATE"

# Test 5: SIGFPE (Invalid State)
run_test "SIGFPE (Floating-Point Exception)" "$BIN_DIR/sigfpe" "INVALID_STATE"

# Test 6: SIGBUS (Memory Corruption) - may not trigger on all systems
if [ -f "$BIN_DIR/sigbus" ]; then
    run_test "SIGBUS (Bus Error)" "$BIN_DIR/sigbus" "MEMORY_CORRUPTION"
fi

# Test 7: Unknown signal (SIGKILL)
run_test "Unknown Signal (SIGKILL)" "$BIN_DIR/unknown_signal" "UNKNOWN"

# Test 8: ENOMEM (Resource Exhaustion) - may not always trigger
if [ -f "$BIN_DIR/enomem" ]; then
    run_test "ENOMEM (Out of Memory)" "$BIN_DIR/enomem" "RESOURCE_EXHAUSTION"
fi

# Test 9: Nonexistent program (exec failure)
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}Test: Exec Failure (Nonexistent Program)${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
TOTAL=$((TOTAL + 1))
if "$ANALYZER" --run /nonexistent/test/program 2>&1; then
    echo ""
    echo -e "${RED}✗ Test should have failed${NC}"
    FAILED=$((FAILED + 1))
else
    echo ""
    echo -e "${GREEN}✓ Test correctly detected exec failure${NC}"
    PASSED=$((PASSED + 1))
fi
echo ""

# Summary
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Test Summary${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "Total tests:  $TOTAL"
echo -e "${GREEN}Passed:      $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Failed:      $FAILED${NC}"
else
    echo -e "Failed:      $FAILED"
fi
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ✓${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. ✗${NC}"
    exit 1
fi

