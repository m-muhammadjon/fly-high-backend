//
// Created by Akbar Khusanbaev on 26/12/24.
//

#include "ServerSideSocket.h"

int establish_server_side_socket() {
    int server_side_socket;
    struct sockaddr_in backend_addr;

    if ((server_side_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed (IN SERVER_SIDE)");
        return -1;
    }

    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(SERVER_SIDE_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr) <= 0)
    {
        perror("Invalid address (IN SERVER_SIDE)");
        close(server_side_socket);
        return -1;
    }

    if (connect(server_side_socket, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0)
    {
        perror("Connection failed (IN SERVER_SIDE)");
        close(server_side_socket);
        return -1;
    }

    printf("Successfully connected to SERVER_SIDE\n");
    return server_side_socket;
}
