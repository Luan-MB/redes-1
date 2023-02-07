#include "raw_socket.h"
#include "Mensagem.hpp"
#include <string>
#include <stdio.h>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    int retval;

    std::string message;
    char buffer[100];
    
    while (fgets(buffer, 100, stdin))
        message += buffer;

    unsigned int num_packs{((unsigned int) message.length() / MAX_DATA_SIZE) + 1};
    unsigned int remaining_size{(unsigned int) message.length()};

    std::cout << num_packs << std::endl;

    Mensagem inicioTransmissao{Inicio, 16};
    if ((retval = send(socket, inicioTransmissao.montaPacote(), inicioTransmissao.getTamanhoPacote(), 0)) >= 0) {
        fprintf(stderr, "SEND (%d bytes):\n", retval);
    } else
        perror("send()");

    for (int i=0; i<num_packs; ++i) {

        Mensagem *msg;

        if (remaining_size > MAX_DATA_SIZE) {
            std::string message_part{message.substr(MAX_DATA_SIZE*i, MAX_DATA_SIZE)};
            msg = new Mensagem{Texto, (unsigned char) (i % 16), MAX_DATA_SIZE, message_part.c_str()};
            remaining_size -= MAX_DATA_SIZE;
        } else {
            std::string message_part{message.substr(MAX_DATA_SIZE*i, remaining_size)};
            msg = new Mensagem{Texto, (unsigned char) (i % 16), (unsigned char) remaining_size, message_part.c_str()};
        }

        if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0) {
            fprintf(stderr, "SEND (%d bytes):\n", retval);
        } else
            perror("send()");
        delete msg;
    }

    Mensagem fimTransmissao{Fim, 16};
    if ((retval = send(socket, fimTransmissao.montaPacote(), fimTransmissao.getTamanhoPacote(), 0)) >= 0) {
        fprintf(stderr, "SEND (%d bytes):\n", retval);
    } else
        perror("send()");

    return 0;
}