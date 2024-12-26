#include "Socket/SocketBridge.h"

int main()
{
    int gui_socket, server_side_socket;

    connect_socket(gui_socket, server_side_socket);
    accept_connections(gui_socket, server_side_socket);

    return 0;
}