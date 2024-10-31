#ifndef IPC_H
#define IPC_H

/**
 * Initializethe IPC module by starting the WebSocket server.
 * @param port The port number on which the webSocket server will listen,
 * @return 0 on success, -1 on failure.
 */
int icp_init(int port);

/**
 * Broadcasts data to all connected WebSocket clients. 
 * @param data The data to send (in JSON format).
 * @return 0 on success, -1 on failure.
 */
int icp_broadcast(const char* data);


/**
 * Cleans up the IPC module by stopping the WebSocket server and releasing resources.
 */
void ipc_cleanup();

#endif /*IPC_H*/