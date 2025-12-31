#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <stddef.h>

/**
 * Parses a log file line-by-line.
 * Currently implemented as a skeleton that opens the file.
 * @param filename Path to the log file to parse
 * @return 0 on success, non-zero on error
 */
int parse_log_file(const char *filename);

#endif /* LOG_PARSER_H */

