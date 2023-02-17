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
    long long n{0};

    FILE *arq = fopen("vascoletra.txt", "wb");
    Mensagem *msg;

    while (true) {
        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
            if (buffer[0] == 0x7e) {
                
                #ifdef DEBUG
                fprintf(stderr, "RECV (%d bytes):\n", retval);
                #endif
                
                msg = new Mensagem{retval, buffer};
                
                if ((msg->tipo == Dados) && (msg->sequencia == seq)) {

                    unsigned char crc = msg->crc8();
                    if (crc != msg->crc) {
                        std::cout << "Crc original: " << std::bitset<8>(msg->crc) << std::endl << std::endl;
                        std::cout << "Crc falso: " << std::bitset<8>(crc) << std::endl << std::endl;
                    }

                    std::cout << fwrite(msg->dados, 1, msg->tamanho, arq) << std::endl;
                    seq = ((seq + 1) % 16);
                    n++;
                    delete msg;

                } else if (msg->tipo == Fim) {
                    delete msg;
                    break;
                }
            }
        }

    }

    fclose(arq);
    std::cout << n << std::endl;
    return 0;
}