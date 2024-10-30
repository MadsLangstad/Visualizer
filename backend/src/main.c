#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int main(int iArgC, char *apszArgV[]) {
    
    if(logger_init("backend.log") != 0) {
        fprintf(stderr, "Failed to initialize logger.\n");
        exit(EXIT_FAILURE);
    }
    printf("Starting backend...\n");
    logger_log("INFO", "Backend started successfully.\n");

    printf("Hello\n");
    
    logger_cleanup();
    return 0;
}