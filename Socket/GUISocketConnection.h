//
// Created by Akbar Khusanbaev on 26/12/24.
//

#ifndef GUISOCKETCONNECTION_H
#define GUISOCKETCONNECTION_H

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#define GUI_SOCKET_PORT 8101
#define BUFFER_SIZE 16384
/// Returns Socket. Or -1 if there are any kind of error
int establish_gui_socket();

/// Returns Socket Connection in a client perspective. Or -1 if there are errors.
int accept_client_connection(int *gui_socket);

/// Client Thread Arguments Data Structure.
typedef struct
{
    int gui_client_sock;
    int backend_sock;
    int client_number;
} client_thread_args;

/// Handles Client Tread Arguments Object.
void *client_thread_handler(void *args);

/// Sends Request from Client to Server.
int send_to_server_side(int *server_side_socket, const char *buffer);

/// Reads From Server Incoming Response.
int read_from_server_side(int *server_side_socket, char **buffer);

/// Forwards Response to GUI client from Server Side.
int send_to_gui_client_chunked_data(int *gui_client_sock, const char *buffer, int length);

/// Handles Connections. Called from client_thread_handler().
void handle_gui_client_connection(int *gui_client_sock, int *backend_sock, int client_number);

#endif //GUISOCKETCONNECTION_H
