#ifndef LOGGER_H
#define LOGGER_H

/**
 * @file logger.h
 * @brief Header file for the logging module
 * 
 * Provides functions to initiate the logger, log messages with different levels,
 * and clean up logger resources.
 */


/**
 * Initializes the logger by opening the specified log file and initializing the mutex.
 *
 * @param log_file_path Path to the log file. If the file does not exist, it will be created.
 * @return 0 on successful initialization, -1 on failure.
 */
int logger_init(const char* log_file_path);


/**
 * Logs a formatted message with a specified log level.
 *
 * @param level The severity level of the log (e.g., "INFO", "ERROR").
 * @param format The format string (printf-style) for the log message.
 * @param ... Additional arguments corresponding to the format string.
 * @return 0 on success, -1 on failure.
 */
 int logger_log(const char* level, const char* format, ...);


/** 
 * Cleans the logger by closing the log file and destroying the mutex
 */
void logger_cleanup();

#endif /* LOGGER_H */