#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <stddef.h>

typedef struct {
    int has_segfault_keywords;      /* Found "segfault", "segmentation", "SIGSEGV" */
    int has_memory_keywords;         /* Found "memory", "malloc", "free", "leak" */
    int has_timeout_keywords;        /* Found "timeout", "deadlock", "hung" */
    int has_resource_keywords;       /* Found "out of memory", "ENOMEM", "resource" */
} LogAnalysis;

/**
 * Parses a log file line-by-line and scans for failure-related keywords.
 * @param filename Path to the log file to parse (NULL is valid, returns empty analysis)
 * @param analysis Output parameter to be populated with keyword flags
 * @return 0 on success, non-zero on error
 */
int parse_log_file(const char *filename, LogAnalysis *analysis);

#endif /* LOG_PARSER_H */

