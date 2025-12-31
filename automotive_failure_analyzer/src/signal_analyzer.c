#include "signal_analyzer.h"
#include <string.h>

static const SignalInfo signal_table[] = {
    {SIGSEGV, "SIGSEGV", "Segmentation fault - invalid memory access"},
    {SIGABRT, "SIGABRT", "Abort signal - abnormal termination"},
    {SIGBUS, "SIGBUS", "Bus error - invalid memory access alignment"},
    {SIGFPE, "SIGFPE", "Floating-point exception - arithmetic error"}
};

static const size_t signal_table_size = sizeof(signal_table) / sizeof(signal_table[0]);

const SignalInfo *analyze_signal(int signal_num) {
    for (size_t i = 0; i < signal_table_size; i++) {
        if (signal_table[i].signal_num == signal_num) {
            return &signal_table[i];
        }
    }
    return NULL;
}

