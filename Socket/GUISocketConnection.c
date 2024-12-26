//
// Created by Akbar Khusanbaev on 26/12/24.
//

#include "GUISocketConnection.h"

int establish_gui_socket() {
    int gui_socket;
    struct sockaddr_in gui_addr;
    int opt = 1;

    if ((gui_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation failed (IN GUI)");
        return -1;
    }

    if (setsockopt(gui_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(gui_socket);
        return -1;
    }

    // Bind GUI socket
    gui_addr.sin_family = AF_INET;
    gui_addr.sin_addr.s_addr = INADDR_ANY;
    gui_addr.sin_port = htons(GUI_SOCKET_PORT);

    if (bind(gui_socket, (struct sockaddr *)&gui_addr, sizeof(gui_addr)) < 0)
    {
        perror("Bind failed (IN GUI)");
        close(gui_socket);
        return -1;
    }

    if (listen(gui_socket, 5) < 0)
    {
        perror("Listen failed (IN GUI)");
        close(gui_socket);
        return -1;
    }

    return gui_socket;
}

int accept_client_connection(int *gui_socket) {
    int gui_client_connection;
    struct sockaddr_in gui_client_addr;
    socklen_t gui_client_addr_len = sizeof(gui_client_addr);

    gui_client_connection = accept(*gui_socket, (struct sockaddr *)&gui_client_addr, &gui_client_addr_len);
    if (gui_client_connection < 0)
    {
        perror("GUI accept failed");
        return -1;
    }

    printf("Accepted GUI client: IP = %s, Port = %d\n",
           inet_ntoa(gui_client_addr.sin_addr), ntohs(gui_client_addr.sin_port));
    return gui_client_connection;
}

int send_to_server_side(int *server_side_socket, const char *buffer) {
    if (send(*server_side_socket, buffer, strlen(buffer), 0) < 0)
    {
        perror("Send to server side failed");
        return -1;
    }
    return 0;
}

int read_from_server_side(int *server_side_socket, char **buffer) {
    uint32_t len_of_response;
    if (recv(*server_side_socket, &len_of_response, sizeof(len_of_response), 0) <= 0)
    {
        perror("Failed to read length header from server side");
        return -1;
    }
    len_of_response = ntohl(len_of_response);

    *buffer = (char *)malloc(len_of_response);
    if (*buffer == NULL)
    {
        perror("Failed to allocate memory for server side response");
        return -1;
    }

    int bytes_received = 0;
    while (bytes_received < len_of_response)
    {
        int chunk = recv(*server_side_socket, *buffer + bytes_received, BUFFER_SIZE, 0);
        if (chunk <= 0)
        {
            perror("Failed to read data from Server Side");
            free(*buffer);
            return -1;
        }
        bytes_received += chunk;
    }
    return len_of_response;
}

int send_to_gui_client_chunked_data(int *gui_client_sock, const char *buffer, int length) {
    uint32_t net_length = htonl(length);
    if (send(*gui_client_sock, &net_length, sizeof(net_length), 0) < 0)
    {
        perror("Failed to send length header to GUI");
        return -1;
    }

    int bytes_sent = 0;
    while (bytes_sent < length)
    {
        int chunk = send(*gui_client_sock, buffer + bytes_sent, length - bytes_sent, 0);
        if (chunk < 0)
        {
            perror("Failed to send data to GUI");
            return -1;
        }
        bytes_sent += chunk;
    }
    return 0;
}

void handle_gui_client_connection(int *gui_client_sock, int *backend_sock, int client_number)
{
    char *buffer = NULL;
    int bytes_read;

    printf("Client No. %d connected\n", client_number);
    fflush(stdout);

    while (true)
    {
        char request_buffer[BUFFER_SIZE];
        memset(request_buffer, 0, BUFFER_SIZE);
        bytes_read = read(*gui_client_sock, request_buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0)
        {
            perror("Read from GUI failed");
            break;
        }
        if (bytes_read == 0)
        {
            printf("Client #%d disconnected\n", client_number);
            fflush(stdout);
            break;
        }

        request_buffer[bytes_read] = '\0';
        printf("Received from Client #%d: %s\n", client_number, request_buffer);

        if (send_to_server_side(backend_sock, request_buffer) < 0)
        {
            break;
        }

        int total_length = read_from_server_side(backend_sock, &buffer); // Pass address of buffer
        if (total_length < 0)
        {
            break;
        }

        printf("Server Side response to Client No. %d: %s\n", client_number, buffer);
        fflush(stdout);

        if (send_to_gui_client_chunked_data(gui_client_sock, buffer, total_length) < 0)
        {
            break;
        }

        free(buffer);
        buffer = NULL;
    }
}

void *client_thread_handler(void *args)
{
    client_thread_args *arguments = args;
    handle_gui_client_connection(&arguments->gui_client_sock, &arguments->backend_sock, arguments->client_number);
    close(arguments->gui_client_sock);
    free(arguments);
    pthread_exit(NULL);
}