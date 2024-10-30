#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>

/* Static variables to hold the log file pointer and mutex */
static FILE* log_file = NULL;
static pthread_mutex_t log_mutex;


/**
 * Initialize the logger by opening the the log file and initialize the mutex.
 *
 * @param log_file_path path to the log file.
 * @return 0 on success, -1 on failure.
 */
int logger_init(const char* log_file_path) {
    if(log_file_path == NULL) {
        return -1;
    }

    /* Open the log file in append mode */
    log_file = fopen(log_file_path, "a");
    if(log_file == NULL) {
        return -1;
    }

    /* Initialize the mutex for thread-safe logging */
    if(pthread_mutex_init(&log_mutex, NULL) != 0) {
        fclose(log_file);
        log_file = NULL;
        return -1;
    }

    return 0;
}


/**
 * Logs a message with the specified level and format.
 *
 * This function is thread-safe and ensures that log messages from different
 * threads do not interleave.
 *
 * @param level The severity level of the log (e.g., "INFO", "ERROR").
 * @param format Format the format string for log message.
 * @param ... Additional arguments for the format string.
 * @return 0 on success, -1 on failure
 */
int logger_log(const char* level, const char* format, ...) {
    if(log_file == NULL || level == NULL || format == NULL) {
        return -1;
    }

    /* Lock the mutex to ensure exclusive access to the log file */
    if(pthread_mutex_lock(&log_mutex) != 0) {
        return -1;
    }

    /* Get the current time */
    time_t raw_time;
    struct tm* time_info;
    char time_buffer[20]; /* Buffer to hold formatted time */

    time(&raw_time);
    time_info = localtime(&raw_time);
    if(time_info == NULL) {
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }

    /* Format the time as "YYYY-MM-DD HH:MM:SS" */
    if(strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info) == 0) {
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }

    /* Write the timestamp and log level to the file */
    fprintf(log_file, "[%s] [%s] ", time_buffer, level);

    /* Handle the variable arguments for the formatted message */
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    /* Add a newline character */
    fprintf(log_file, "\n");

    /* Flush the output to ensure it's written to the file */
    fflush(log_file);

    /* Unlock the mutex */
    pthread_mutex_unlock(&log_mutex);

    return 0;
}

/**
 * Cleans up the logger by closing the log file and destroying the mutex.
 */
void logger_cleanup() {
    if (log_file != NULL) {
        fclose(log_file);
        log_file = NULL;
    }

    /* Destroy the mutex */
    pthread_mutex_destroy(&log_mutex);
}