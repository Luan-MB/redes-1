#include "raw_socket.h"
#include "Mensagem.hpp"
#include <string.h>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char *buffer = (char *) malloc(67);

    unsigned int retval;
    std::string message;

    while (true) {
        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
            if (buffer[0] == 0x7e) {
                fprintf(stderr, "RECV (%d bytes):\n", retval);
                Mensagem msg{retval, buffer};

                std::string message = msg.dados;
                std::cout << message << std::endl;
            }
        }
    }

    return 0;
}