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

    FILE *arq = fopen("vasco.mp3", "wq");
    Mensagem *msg;

    while (true) {
        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
            if (buffer[0] == 0x7e) {
                
                #ifdef DEBUG
                fprintf(stderr, "RECV (%d bytes):\n", retval);
                #endif
                
                msg = new Mensagem{retval, buffer};
                
                if ((msg->tipo == Dados) && (msg->sequencia == seq)) {

                    fwrite(msg->dados, 1, msg->tamanho, arq);
                    seq = ((seq + 1) % 16);
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