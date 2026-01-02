#include "log_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void to_lowercase(char *str) {
    for (size_t i = 0; str[i]; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

int parse_log_file(const char *filename, LogAnalysis *analysis) {
    /* Initialize analysis structure */
    if (analysis == NULL) {
        return -1;
    }
    
    analysis->has_segfault_keywords = 0;
    analysis->has_memory_keywords = 0;
    analysis->has_timeout_keywords = 0;
    analysis->has_resource_keywords = 0;

    /* NULL filename is valid (no log file provided) */
    if (filename == NULL) {
        return 0;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    char line[1024];
    char line_lower[1024];
    
    while (fgets(line, sizeof(line), file) != NULL) {
        /* Make a lowercase copy for case-insensitive matching */
        strncpy(line_lower, line, sizeof(line_lower) - 1);
        line_lower[sizeof(line_lower) - 1] = '\0';
        to_lowercase(line_lower);

        /* Scan for keywords using strstr */
        if (strstr(line_lower, "segfault") != NULL ||
            strstr(line_lower, "segmentation") != NULL ||
            strstr(line_lower, "sigsegv") != NULL) {
            analysis->has_segfault_keywords = 1;
        }

        if (strstr(line_lower, "memory") != NULL ||
            strstr(line_lower, "malloc") != NULL ||
            strstr(line_lower, "free") != NULL ||
            strstr(line_lower, "leak") != NULL ||
            strstr(line_lower, "corruption") != NULL) {
            analysis->has_memory_keywords = 1;
        }

        if (strstr(line_lower, "timeout") != NULL ||
            strstr(line_lower, "deadlock") != NULL ||
            strstr(line_lower, "hung") != NULL ||
            strstr(line_lower, "stuck") != NULL) {
            analysis->has_timeout_keywords = 1;
        }

        if (strstr(line_lower, "out of memory") != NULL ||
            strstr(line_lower, "enomem") != NULL ||
            strstr(line_lower, "resource") != NULL ||
            strstr(line_lower, "exhausted") != NULL) {
            analysis->has_resource_keywords = 1;
        }
    }

    fclose(file);
    return 0;
}

