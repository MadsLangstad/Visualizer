#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "logger.h"
#include "dataGen.h"
#include "ipc.h"
#include <pthread.h>
#include <string.h>
#include <json-c/json.h> // For JSON formatting

// Thread to periodically send data to frontend
void* broadcast_thread_func(void* arg) {
    while (1) {
        int data = data_gen_get_data();

        // Prepare JSON data
        json_object* jobj = json_object_new_object();
        json_object_object_add(jobj, "value", json_object_new_int(data));
        const char* json_str = json_object_to_json_string(jobj);

        // Broadcast to all connected clients
        if (ipc_broadcast(json_str) != 0) {
            // No clients connected or error
        }

        json_object_put(jobj); // Free JSON object

        // Sleep for 100 milliseconds
        usleep(100000);
    }
    return NULL;
}

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

    /* Initialize the IPC module */
    if (ipc_init(8080) != 0) { // Choose an appropriate port
        logger_log("ERROR", "Failed to initialize IPC module.");
        fprintf(stderr, "Failed to initialize IPC module.\n");
        data_gen_cleanup();
        logger_cleanup();
        return EXIT_FAILURE;
    }
    logger_log("INFO", "IPC module initialized successfully.");

    /* Start the broadcast thread */
    pthread_t broadcast_thread;
    if (pthread_create(&broadcast_thread, NULL, broadcast_thread_func, NULL) != 0) {
        logger_log("ERROR", "Failed to create broadcast thread.");
        fprintf(stderr, "Failed to create broadcast thread.\n");
        ipc_cleanup();
        data_gen_cleanup();
        logger_cleanup();
        return EXIT_FAILURE;
    }
    logger_log("INFO", "Broadcast thread started.");

    /* Let the backend run indefinitely */
    printf("Backend running. Press Ctrl+C to exit.\n");
    logger_log("INFO", "Backend is running.");

    // Handle graceful shutdown on SIGINT
    void handle_sigint(int sig) {
        printf("\nShutting down backend...\n");
        logger_log("INFO", "Shutting down backend...");

        // Clean up modules
        ipc_cleanup();
        data_gen_cleanup();
        logger_log("INFO", "Backend shut down successfully.");
        logger_cleanup();

        exit(0);
    }

    signal(SIGINT, handle_sigint);

    // Keep the main thread alive
    while (1) {
        pause(); // Wait for signals
    }

    return 0;
}
