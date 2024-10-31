#include "ipc.h"
#include <libwebsockets.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static struct lws_context* context = NULL;
static struct lws_protocols protocols[];

// Structure to hold all connected clients
typedef struct client_node {
    struct lws* wsi;
    struct client_node* next;
} client_node_t;

static client_node_t* clients = NULL;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Callback function for WebSocket events
static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason,
                              void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            // Add new client to the list
            pthread_mutex_lock(&clients_mutex);
            {
                client_node_t* new_client = malloc(sizeof(client_node_t));
                new_client->wsi = wsi;
                new_client->next = clients;
                clients = new_client;
                printf("New client connected. Total clients: %d\n", 
                       clients ? 1 : 0); // Simplistic count
            }
            pthread_mutex_unlock(&clients_mutex);
            break;

        case LWS_CALLBACK_CLOSED:
            // Remove client from the list
            pthread_mutex_lock(&clients_mutex);
            {
                client_node_t **curr = &clients;
                while (*curr) {
                    if ((*curr)->wsi == wsi) {
                        client_node_t *to_remove = *curr;
                        *curr = (*curr)->next;
                        free(to_remove);
                        printf("Client disconnected. Remaining clients: %d\n", 
                               clients ? 1 : 0); // Simplistic count
                        break;
                    }
                    curr = &(*curr)->next;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            break;

        default:
            break;
    }

    return 0;
}

// Define the protocol
static struct lws_protocols protocols[] = {
    {
        .name = "real-time-protocol",
        .callback = websocket_callback,
        .per_session_data_size = 0,
        .rx_buffer_size = 0,
        .id = 0,
        .user = NULL,
        .tx_packet_size = 0,
    },
    { NULL, NULL, 0, 0, 0, NULL, 0 } // Terminator
};

// Thread function to run the WebSocket server
static void* server_thread_func(void* arg) {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = *(int*)arg;
    info.protocols = protocols;
    info.options = 0;

    context = lws_create_context(&info);
    if (context == NULL) {
        fprintf(stderr, "libwebsockets initialization failed\n");
        return NULL;
    }

    printf("WebSocket server started on port %d\n", *(int*)arg);

    // Run the event loop
    while (lws_service(context, 1000) >= 0) {
        // Continue running
    }

    lws_context_destroy(context);
    return NULL;
}

static pthread_t server_thread;

// Initialize the IPC module by starting the WebSocket server
int ipc_init(int port) {
    // Start the server thread
    if (pthread_create(&server_thread, NULL, server_thread_func, &port) != 0) {
        perror("Failed to create IPC server thread");
        return -1;
    }

    // Give the server some time to start
    sleep(1);
    return 0;
}

// Broadcast data to all connected clients
int ipc_broadcast(const char* data) {
    pthread_mutex_lock(&clients_mutex);
    client_node_t* current = clients;
    int count = 0;

    while (current) {
        // Send data to each client
        int len = strlen(data);
        unsigned char* buf = malloc(LWS_PRE + len);
        if (!buf) {
            pthread_mutex_unlock(&clients_mutex);
            return -1;
        }

        memcpy(&buf[LWS_PRE], data, len);

        int bytes = lws_write(current->wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
        if (bytes < len) {
            fprintf(stderr, "Failed to send data to client\n");
            free(buf);
            pthread_mutex_unlock(&clients_mutex);
            return -1;
        }

        free(buf);
        current = current->next;
        count++;
    }

    pthread_mutex_unlock(&clients_mutex);
    return (count > 0) ? 0 : -1;
}

// Clean up the IPC module by stopping the server and freeing resources
void ipc_cleanup() {
    if (context) {
        lws_context_destroy(context);
        context = NULL;
    }

    // Disconnect all clients
    pthread_mutex_lock(&clients_mutex);
    client_node_t* current = clients;
    while (current) {
        struct lws* wsi = current->wsi;
        lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, NULL, 0);
        lws_callback_on_writable(wsi);
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);

    // Wait for the server thread to finish
    pthread_join(server_thread, NULL);

    // Free client list
    pthread_mutex_lock(&clients_mutex);
    current = clients;
    while (current) {
        client_node_t* to_free = current;
        current = current->next;
        free(to_free);
    }
    clients = NULL;
    pthread_mutex_unlock(&clients_mutex);

    pthread_mutex_destroy(&clients_mutex);
}