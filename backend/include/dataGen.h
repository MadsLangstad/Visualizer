#ifndef DATA_GEN_H
#define DATA_GEN_H

/**
 * Initializes the data generator by starting a thread.
 * @return 0 on success, -1 on failure.
 */
int data_get_init();


/**
 * Cleans up the data generator by stopping the thread and releasing resources.
 */
void data_gen_cleanup();


/**
 * Retrieves the latest generated data (thread-safe)
 * @return the latest generated data as an integer.
 */
int data_gen_get_data();

#endif /* DATA_GEN_H */