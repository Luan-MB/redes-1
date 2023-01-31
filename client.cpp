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

    while (true) {
        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
            if (buffer[0] == 0x7e) {
                fprintf(stderr, "RECV (%d bytes):\n", retval);
                Mensagem msg{retval, buffer};
                
                if (msg.tipo == Inicio) {

                    std::string message;

                    while (true) {
                        if ((retval = recv(socket, buffer, 67, 0)) > 0) {
                            if (buffer[0] == 0x7e) {
                                fprintf(stderr, "RECV (%d bytes):\n", retval);
                                Mensagem msg{retval, buffer};

                                if (msg.tipo == Texto) {
                                    std::string subMessage;

                                    memcpy(&subMessage, &msg.dados[3], msg.tamanho);  

                                    message += subMessage;
                                } else if (msg.tipo == Fim) {
                                    break;
                                }

                            }
                        }
                    }

                    std::cout << message;
                }
            }
        }
    }

    return 0;
}