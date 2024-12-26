//
// Created by Akbar Khusanbaev on 26/12/24.
//

#ifndef SERVERSIDESOCKET_H
#define SERVERSIDESOCKET_H

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_SIDE_PORT 8111

/// Returns Server Side Socket. Or -1 in case any error occurs.
int establish_server_side_socket();

#endif //SERVERSIDESOCKET_H
