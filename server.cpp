#include "raw_socket.h"
#include "Mensagem.hpp"
#include <string>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    std::string message;
    
    std::getline(std::cin, message);

    unsigned int num_packs{((unsigned int) message.length() / 63) + 1};
    unsigned int remaining_size{(unsigned int) message.length()};

    for (int i=0; i<num_packs; ++i) {

        Mensagem *msg;

        if (remaining_size > 63) {
            std::string message_part{message.substr(63*i, 63)};
            msg = new Mensagem{Texto, (unsigned char) i, 63, message_part.c_str()};
            remaining_size -= 63;
        } else {
            std::string message_part{message.substr(63*i, remaining_size)};
            msg = new Mensagem{Texto, (unsigned char) i, (unsigned char) remaining_size, message_part.c_str()};
        }
        
        int retval;

        if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0)
            fprintf(stderr, "SEND (%d bytes):\n", retval);
        else
            perror("send()");

        delete msg;
    }

    return 0;
}