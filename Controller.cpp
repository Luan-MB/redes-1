#include "Controller.hpp"
#include "raw_socket.h"
#include <sys/time.h>

int Controller::sendMessage(int socket, Mensagem* msg) {
    return send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0);
}

int Controller::recvMessage(int socket, char *recv_buffer) {
    return recv(socket, recv_buffer, MAX_MSG_SIZE, 0);
}

long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
int valid_protocol(char* buffer, int tamanho_buffer) {
    if (tamanho_buffer <= 0) { return 0; }
    return buffer[0] == 0x7e;
}

int Controller::recvAck(int socket, char *recv_buffer) {
    long long start = timestamp();
    struct timeval timeout = { .tv_sec = 0, .tv_usec = TIMEOUT * 1000 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    int bytes_lidos;
    do {
        bytes_lidos = recv(socket, recv_buffer, MAX_MSG_SIZE, 0);
        if (valid_protocol(recv_buffer, bytes_lidos)) { return bytes_lidos; }
    } while (timestamp() - start <= TIMEOUT);
    return -1;
}