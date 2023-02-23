#include "Controller.hpp"
#include "raw_socket.h"
#include <sys/time.h>

int valid_protocol(char* buffer, int tamanho_buffer) {
    if (tamanho_buffer <= 0) { return 0; }
    return buffer[0] == 0x7e;
}

int Controller::sendMessage(int socket, Mensagem* msg) {
    return send(socket, msg->montaPacote(), MSG_SIZE, 0);
}

void Controller::maskMessage(Mensagem *msg) {

    if (msg->tipo != Mask)
        msg->tipo = Mask;
    
    for (int i=0; i<msg->tamanho; ++i) {
        msg->dados[i] ^= 0xFF;
    }
}

int Controller::resendMessage(int socket, Mensagem* msg) {

    bool mask = false;

    for (int i=0; i<msg->tamanho; ++i) {
        if ((msg->dados[i] == -127) || (msg->dados[i] == -120)) { // 0x81 e 0x88
            mask = true;
            break;
        }
    }

    if (mask) {
        Controller::maskMessage(msg);
    }

    return sendMessage(socket, msg);
}

int Controller::recvMessage(int socket, char *recv_buffer) {
    return recv(socket, recv_buffer, MSG_SIZE, 0);
}

long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}

int Controller::recvAck(int socket, char *recv_buffer) {
    long long start = timestamp();
    struct timeval timeout = { .tv_sec = 0, .tv_usec = TIMEOUT * 1000 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    int bytes_lidos;
    do {
        bytes_lidos = recv(socket, recv_buffer, MSG_SIZE, 0);
        if (valid_protocol(recv_buffer, bytes_lidos)) { return bytes_lidos; }
    } while (timestamp() - start <= TIMEOUT);
    return -1;
}