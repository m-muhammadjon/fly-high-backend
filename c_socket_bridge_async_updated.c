#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define GUI_PORT 65432    // Port for the GUI to connect
#define BACKEND_PORT 8111 // Port to connect to the backend
#define BUFFER_SIZE 16384 // Increase buffer size to 16 KB
                          // Match chunk size with the backend's chunk size

// ---------------------------
// Function prototypes
// ---------------------------
int setup_gui_socket();
int accept_gui_connection(int gui_sock);
int connect_to_backend();
int send_to_backend(int backend_sock, const char *buffer);
int read_from_backend_chunked(int backend_sock, char **buffer);
int send_to_gui_chunked(int gui_client_sock, const char *buffer, int length);
void close_connections(int gui_client_sock, int backend_sock);
void handle_gui_client(int gui_client_sock, int backend_sock, int client_number);

// ---------------------------
// Thread function to handle each client
// ---------------------------
typedef struct
{
    int gui_client_sock;
    int backend_sock;
    int client_number;
} client_thread_args;

void *client_thread(void *arg)
{
    client_thread_args *args = (client_thread_args *)arg;
    handle_gui_client(args->gui_client_sock, args->backend_sock, args->client_number);
    close(args->gui_client_sock);
    free(args); // Free thread arguments
    pthread_exit(NULL);
}

// ---------------------------
// Main function
// ---------------------------
int main()
{
    int gui_sock, backend_sock;

    // Setup GUI socket
    gui_sock = setup_gui_socket();
    if (gui_sock < 0)
    {
        exit(EXIT_FAILURE);
    }

    printf("C Socket Bridge listening on port %d\n", GUI_PORT);
    fflush(stdout);

    // Connect to backend (only one backend connection in this design)
    backend_sock = connect_to_backend();
    if (backend_sock < 0)
    {
        close(gui_sock);
        exit(EXIT_FAILURE);
    }

    // Main loop: accept multiple GUI clients
    int client_count = 0;
    while (1)
    {
        int gui_client_sock = accept_gui_connection(gui_sock);
        if (gui_client_sock < 0)
        {
            continue;
        }

        client_count++;

        // Prepare arguments for the thread
        client_thread_args *args = (client_thread_args *)malloc(sizeof(client_thread_args));
        if (!args)
        {
            perror("malloc failed");
            close(gui_client_sock);
            continue;
        }
        args->gui_client_sock = gui_client_sock;
        args->backend_sock = backend_sock;
        args->client_number = client_count;

        // Create a thread for each client
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, (void *)args) != 0)
        {
            perror("pthread_create failed");
            close(gui_client_sock);
            free(args);
            continue;
        }

        // Detach thread
        pthread_detach(tid);
    }

    // Cleanup
    close(backend_sock);
    close(gui_sock);
    return 0;
}

// ---------------------------
// Function to handle GUI client
// ---------------------------
void handle_gui_client(int gui_client_sock, int backend_sock, int client_number)
{
    char *buffer = NULL; // Declare buffer as a pointer
    int bytes_read;

    printf("Client #%d connected\n", client_number);
    fflush(stdout);

    while (1)
    {
        // Read data from GUI client
        char request_buffer[BUFFER_SIZE];
        memset(request_buffer, 0, BUFFER_SIZE);
        bytes_read = read(gui_client_sock, request_buffer, BUFFER_SIZE - 1);
        if (bytes_read < 0)
        {
            perror("Read from GUI failed");
            break;
        }
        else if (bytes_read == 0)
        {
            printf("Client #%d disconnected\n", client_number);
            fflush(stdout);
            break;
        }

        request_buffer[bytes_read] = '\0';
        printf("Received from Client #%d: %s\n", client_number, request_buffer);

        // Send data to backend
        if (send_to_backend(backend_sock, request_buffer) < 0)
        {
            break;
        }

        // Read response from backend in chunks
        int total_length = read_from_backend_chunked(backend_sock, &buffer); // Pass address of buffer
        if (total_length < 0)
        {
            break;
        }

        // Debug: Log backend response
        printf("Backend response for Client #%d: %s\n", client_number, buffer);
        fflush(stdout);

        // Forward response to GUI client in chunks
        if (send_to_gui_chunked(gui_client_sock, buffer, total_length) < 0)
        {
            break;
        }

        // Free dynamically allocated memory after use
        free(buffer);
        buffer = NULL; // Reset pointer for safety
    }
}

// ---------------------------
// Function to send data to the backend
// ---------------------------
int send_to_backend(int backend_sock, const char *buffer)
{
    if (send(backend_sock, buffer, strlen(buffer), 0) < 0)
    {
        perror("Send to backend failed");
        return -1;
    }
    return 0;
}

// ---------------------------
// Function to read chunked data from backend
// ---------------------------
int read_from_backend_chunked(int backend_sock, char **buffer)
{
    uint32_t total_length;
    if (recv(backend_sock, &total_length, sizeof(total_length), 0) <= 0)
    {
        perror("Failed to read length header from backend");
        return -1;
    }
    total_length = ntohl(total_length); // Convert to host byte order
    // printf("Backend response total length: %u\n", total_length);

    // Allocate memory for the full payload
    *buffer = (char *)malloc(total_length);
    if (*buffer == NULL)
    {
        perror("Failed to allocate memory for backend response");
        return -1;
    }

    // Read the payload in larger chunks
    int bytes_received = 0;
    while (bytes_received < total_length)
    {
        int chunk = recv(backend_sock, *buffer + bytes_received, BUFFER_SIZE, 0);
        if (chunk <= 0)
        {
            perror("Failed to read data from backend");
            free(*buffer); // Free allocated memory on failure
            return -1;
        }
        bytes_received += chunk;
    }
    return total_length;
}

// ---------------------------
// Function to send chunked data to GUI client
// ---------------------------
int send_to_gui_chunked(int gui_client_sock, const char *buffer, int length)
{
    uint32_t net_length = htonl(length); // Convert to network byte order
    if (send(gui_client_sock, &net_length, sizeof(net_length), 0) < 0)
    {
        perror("Failed to send length header to GUI");
        return -1;
    }

    int bytes_sent = 0;
    while (bytes_sent < length)
    {
        int chunk = send(gui_client_sock, buffer + bytes_sent, length - bytes_sent, 0);
        if (chunk < 0)
        {
            perror("Failed to send data to GUI");
            return -1;
        }
        bytes_sent += chunk;
    }
    return 0;
}

// ---------------------------
// Function to set up the GUI listening socket
// ---------------------------
int setup_gui_socket()
{
    int gui_sock;
    struct sockaddr_in gui_addr;
    int opt = 1;

    // Create socket for GUI connections
    if ((gui_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("GUI socket creation failed");
        return -1;
    }

    // Set socket options to reuse address and port
    if (setsockopt(gui_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(gui_sock);
        return -1;
    }

    // Bind GUI socket
    gui_addr.sin_family = AF_INET;
    gui_addr.sin_addr.s_addr = INADDR_ANY;
    gui_addr.sin_port = htons(GUI_PORT);

    if (bind(gui_sock, (struct sockaddr *)&gui_addr, sizeof(gui_addr)) < 0)
    {
        perror("GUI socket bind failed");
        close(gui_sock);
        return -1;
    }

    // Listen for GUI connections
    if (listen(gui_sock, 5) < 0)
    {
        perror("GUI socket listen failed");
        close(gui_sock);
        return -1;
    }

    return gui_sock;
}

// ---------------------------
// Function to accept GUI client connection
// ---------------------------
int accept_gui_connection(int gui_sock)
{
    int gui_client_sock;
    struct sockaddr_in gui_client_addr;
    socklen_t gui_client_addr_len = sizeof(gui_client_addr);

    gui_client_sock = accept(gui_sock, (struct sockaddr *)&gui_client_addr, &gui_client_addr_len);
    if (gui_client_sock < 0)
    {
        perror("GUI accept failed");
        return -1;
    }

    printf("Accepted GUI client: IP = %s, Port = %d\n",
           inet_ntoa(gui_client_addr.sin_addr), ntohs(gui_client_addr.sin_port));
    return gui_client_sock;
}

// ---------------------------
// Function to connect to backend
// ---------------------------
int connect_to_backend()
{
    int backend_sock;
    struct sockaddr_in backend_addr;

    if ((backend_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Backend socket creation failed");
        return -1;
    }

    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(BACKEND_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &backend_addr.sin_addr) <= 0)
    {
        perror("Invalid backend address");
        close(backend_sock);
        return -1;
    }

    if (connect(backend_sock, (struct sockaddr *)&backend_addr, sizeof(backend_addr)) < 0)
    {
        perror("Connect to backend failed");
        close(backend_sock);
        return -1;
    }

    printf("Connected to backend\n");
    return backend_sock;
}
