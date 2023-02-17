#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <bitset>

#include "Mensagem.hpp"
#include "raw_socket.h"

int main (int argc, char **argv) {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char send_buffer[3780];
    size_t size;
    unsigned int retval;

    char *recv_buffer = (char *) malloc(67);

    FILE *arq = fopen(argv[1], "rb");

    Mensagem *msg, *response;
    unsigned char seq{0x0};
    char sub_buffer[MAX_DATA_SIZE];
    unsigned int i;
    
    while (size = fread(send_buffer, 1, 3780, arq)) {
        unsigned int n_packs = (size / MAX_DATA_SIZE);

        for (i=0; i<n_packs; ++i) {
            memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], MAX_DATA_SIZE);
            msg = new Mensagem{Dados, seq, MAX_DATA_SIZE, sub_buffer};

            if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0) {
                fprintf(stderr, "SEND (%d bytes):\n", retval);
            } else
                perror("send()");

            while (true) {
                if ((retval = recv(socket, recv_buffer, 67, 0)) > 0) {
                    if (recv_buffer[0] == 0x7e) {
                        response = new Mensagem{retval, recv_buffer};

                        if ((response->tipo == Nack) && (response->sequencia == seq))
                            send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0);
                        else if ((response->tipo == Ack) && (response->sequencia == seq)) {
                            std::cout << "Recebeu Ack do " << std::bitset<4>(response->sequencia) << std::endl;
                            break;
                        }
                    }
                }
            }
            
            seq = (seq + 1) % 16;
            delete msg;
        }

        unsigned int remainder = size % MAX_DATA_SIZE;

        if (remainder > 0) {
            memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], remainder);
            msg = new Mensagem{Dados, seq, (unsigned char) remainder, sub_buffer};
            seq = (seq + 1) % 16;

            msg->imprimeCamposMsg();

            if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0) {
                fprintf(stderr, "SEND (%d bytes):\n", retval);
            } else
                perror("send()");

            delete msg;
        }

    }

    fclose(arq);

    msg = new Mensagem{Fim, 16};
    send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0);
    delete msg;

    return 0;
}