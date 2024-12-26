//
// Created by Akbar Khusanbaev on 26/12/24.
//

#include "SocketBridge.h"
#include "GUISocketConnection.h"
#include "ServerSideSocket.h"


void connect_socket(int *gui_socket, int *server_side_socket) {
    // GUI connection
    *gui_socket = establish_gui_socket();
    if (gui_socket < 0) {
        exit(EXIT_FAILURE);
    }
    printf("C Socket Bridge listening on port %d\n", GUI_SOCKET_PORT);
    fflush(stdout);
    // Server Side connection
    *server_side_socket = establish_server_side_socket();
    if (server_side_socket < 0) {
        close(*gui_socket);
        exit(EXIT_FAILURE);
    }
    printf("Server side socket has benn established successfully in port %d\n", SERVER_SIDE_PORT);
    fflush(stdout);
}

void accept_connections(int *gui_socket, int server_side_socket) {
    int connections_count = 0;

    while(true) {
        int gui_client_connection = accept_client_connection(gui_socket);
        if (gui_client_connection < 0) {
            continue;
        }

        connections_count++;

        client_thread_args *args = malloc(sizeof(client_thread_args));
        if (!args)
        {
            perror("malloc failed");
            close(gui_client_connection);
            continue;
        }
        args->gui_client_sock = gui_client_connection;
        args->backend_sock = server_side_socket;
        args->client_number = connections_count;

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread_handler, args) != 0)
        {
            perror("pthread_create failed");
            close(gui_client_connection);
            free(args);
            continue;
        }

        pthread_detach(tid);
    }
}