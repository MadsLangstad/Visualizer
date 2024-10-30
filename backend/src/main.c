#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "logger.h"
#include "dataGen.h"

int main(int argc, char *argv[]) {
    /* Initialize the logger */
    if (logger_init("backend.log") != 0) {
        fprintf(stderr, "Failed to initialize logger.\n");
        exit(EXIT_FAILURE);
    }
    logger_log("INFO", "Backend started successfully.");

    /* Initialize the data generator */
    if (data_get_init() != 0) {
        logger_log("ERROR", "Failed to initialize data generator.");
        fprintf(stderr, "Failed to initialize data generator.\n");
        logger_cleanup();
        return EXIT_FAILURE;
    }
    logger_log("INFO", "Data generator initialized successfully.");

    printf("Data generator initialized. Retrieving data for 5 seconds...\n");

    /* Retrieve data every 500 milliseconds for 5 seconds */
    int i = 0;
    for (i; i < 10; ++i) {
        int data;
        data = data_gen_get_data();
        
        /* Prepare a log message */
        char log_message[50];
        snprintf(log_message, sizeof(log_message), "Retrieved data: %d", data);
        logger_log("INFO", log_message);
        
        printf("Retrieved data: %d\n", data);
        usleep(500000); /* 500 milliseconds */
    }

    /* Clean up the data generator */
    data_gen_cleanup();
    logger_log("INFO", "Data generator cleaned up.");
    printf("Data generator cleaned up.\n");

    /* Clean up the logger */
    logger_cleanup();

    return 0;
}
