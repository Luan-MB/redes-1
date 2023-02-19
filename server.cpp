#include <cstdio>
#include <bitset>
#include <bits/stdc++.h>

#include "raw_socket.h"
#include "Mensagem.hpp"
#include "Controller.hpp"

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

/*     struct timeval timeout = { .tv_sec = 5 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)); */

    printf("Escutando...\n");

    char *buffer = (char *) malloc(MAX_MSG_SIZE);
    unsigned int retval;

    Mensagem *msg, *response;

    while (true) {

        if ((retval = Controller::recvMessage(socket, buffer)) == 20) {
            if (buffer[0] == 0x7e) {

                unsigned char seq{0x0};

                msg = new Mensagem{retval, buffer};
                if ((msg->tipo == Inicio) && (msg->sequencia == seq)) {
                    response = new Mensagem{Ack, seq, 16};

                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                        seq = (seq + 1) % 16;
                    }

                    while (true) {
                        if ((retval = Controller::recvMessage(socket, buffer)) > 0) {

                            msg = new Mensagem{retval, buffer};

                            if (((msg->tipo == Midia) || (msg->tipo == Texto)) && (msg->sequencia == seq)) {
                                response = new Mensagem{Ack, seq, 16};

                                if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                    fprintf(stderr, "SEND (%d bytes):\n", retval);
                                    seq = (seq + 1) % 16;
                                }

                                break;
                            }
                        }
                    }

                    if (msg->tipo == Midia) {
                        
                        char *nome = (char *) malloc (msg->tamanho);

                        memcpy(nome, msg->dados, msg->tamanho);

                        FILE *arq = fopen(nome, "wb");

                        while (true) {
                            if ((retval = Controller::recvMessage(socket, buffer)) > 0) {
                                if (buffer[0] == 0x7e) {
                                    
                                    #ifdef DEBUG
                                    fprintf(stderr, "RECV (%d bytes):\n", retval);
                                    #endif
                                    
                                    msg = new Mensagem{retval, buffer};
                                    
                                    if (msg->tipo == Dados) {

                                        if (msg->sequencia == seq) {
                                            unsigned char crc = msg->crc8();
                                            if (crc != msg->crc) {
                                                response = new Mensagem{Nack, seq, 16};
                                                if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                                    fprintf(stderr, "SEND (%d bytes):\n", retval);
                                                } else
                                                    perror("send()");
                                            } else {

                                                fwrite(msg->dados, 1, msg->tamanho, arq);
                                                response = new Mensagem{Ack, seq, 16};

                                                if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                                    fprintf(stderr, "SEND (%d bytes):\n", retval);
                                                    seq = (seq + 1) % 16;
                                                } else
                                                    perror("send()");
                                            }
                                        }

                                    } else if (msg->tipo == Fim) {
                                        delete msg;
                                        break;
                                    }

                                    delete msg;
                                }
                            } else {
                                std::cout << "Timeout\n";
                            }
                        }

                        fclose(arq);
                    
                        free(nome);
                    }
                }
            }
        }
    }

    return 0;
}