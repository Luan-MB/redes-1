#include "raw_socket.h"
#include "Mensagem.hpp"
#include <cstdio>
#include <bitset>
#include <bits/stdc++.h>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    struct timeval timeout = { .tv_sec = 1 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));

    printf("Escutando...\n");

    char *buffer = (char *) malloc(67);
    unsigned int retval;

    Mensagem *msg, *response;

    while (true) {

        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
            if (buffer[0] == 0x7e) {

                msg = new Mensagem{retval, buffer};

                if (msg->tipo == Midia) {

                    char *nome = (char *) malloc (msg->tamanho);

                    memcpy(nome, msg->dados, msg->tamanho);

                    unsigned char seq{0x0};

                    FILE *arq = fopen(nome, "wb");

                    while (true) {
                        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
                            if (buffer[0] == 0x7e) {
                                
                                #ifdef DEBUG
                                fprintf(stderr, "RECV (%d bytes):\n", retval);
                                #endif
                                
                                msg = new Mensagem{retval, buffer};
                                
                                if (msg->tipo == Dados) {

                                    if (msg->sequencia == seq) {
                                        unsigned char crc = msg->crc8();
                                        if (crc != msg->crc) {
                                            std::cout << "Crc original: " << std::bitset<8>(msg->crc) << std::endl << std::endl;
                                            std::cout << "Crc falso: " << std::bitset<8>(crc) << std::endl << std::endl;
                                        }

                                        fwrite(msg->dados, 1, msg->tamanho, arq);
                                        response = new Mensagem{Ack, seq, 16};

                                        if ((retval = send(socket, response->montaPacote(), response->getTamanhoPacote(), 0)) >= 0) {
                                            fprintf(stderr, "SEND (%d bytes):\n", retval);
                                            seq = (seq + 1) % 16;
                                        } else
                                            perror("send()");
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
                }
            }
        }
    }

    return 0;
}