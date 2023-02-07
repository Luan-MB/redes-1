#include "raw_socket.h"
#include "Mensagem.hpp"
#include <string.h>
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

    while (true) {
        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
            if (buffer[0] == 0x7e) {
                
                #ifdef DEBUG
                fprintf(stderr, "RECV (%d bytes):\n", retval);
                #endif
                
                Mensagem *msg = new Mensagem{retval, buffer};

                if (msg->tipo == Inicio) {

                    std::string message;
                    unsigned char seq{0x0};

                    while (true) {

                        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
                            if (buffer[0] == 0x7e) {
                                
                                #ifdef DEBUG
                                fprintf(stderr, "RECV (%d bytes):\n", retval);
                                #endif

                                Mensagem *streamMsg = new Mensagem{retval, buffer};

                                if ((streamMsg->tipo == Texto) && (streamMsg->sequencia == seq)) {
                                    std::string messagePart = streamMsg->dados;
                                    message += messagePart.substr(0, streamMsg->tamanho);
                                    seq = ((seq + 1) % 16);
                                    delete streamMsg;

                                } else if (streamMsg->tipo == Fim) {
                                    delete streamMsg;
                                    break;
                                }
                                
                            }
                        }
                    }

                    std::cout << message << std::endl;


                    delete msg;
                }
            }
        }

    }

    return 0;
}