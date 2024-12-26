//
// Created by Akbar Khusanbaev on 26/12/24.
//

#ifndef SOCKETBRIDGE_H
#define SOCKETBRIDGE_H
/// Funcion to connect all sockets
void connect_socket(int gui_socket, int server_side_socket);

/// Function to accept connections
void accept_connections(int gui_socket, int server_side_socket);

#endif //SOCKETBRIDGE_H
