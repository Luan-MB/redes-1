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

    printf("Escutando...\n");

    char *buffer = (char *) malloc(67);

    unsigned int retval;
    unsigned char seq{0x0};

    FILE *arq = fopen("vasco.txt", "wb");
    Mensagem *msg, *response;

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
                        response = new Mensagem{Ack, msg->sequencia, 16};
                        send(socket, response->montaPacote(), response->getTamanhoPacote(), 0);
                        std::cout << "Enviou Ack do " << std::bitset<4>(response->sequencia) << std::endl;
                        seq = (seq + 1) % 16;

                        delete response;
                    } else {
                        response = new Mensagem{Nack, seq, 16};
                        send(socket, response->montaPacote(), response->getTamanhoPacote(), 0);
                        delete response;
                    }

                    delete msg;

                } else if (msg->tipo == Fim) {
                    delete msg;
                    break;
                }
            }
        }
    }

    fclose(arq);

    return 0;
}