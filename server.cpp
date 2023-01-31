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

    std::cout << message.length() << std::endl;

    Mensagem msg{Texto, 0, (unsigned char) message.length(), message.c_str()};

    char *pacote{msg.montaPacote()};
    unsigned int tamanhoPacote{msg.getTamanhoPacote()};

    int retval;

    if ((retval = send(socket, pacote, tamanhoPacote, 0)) >= 0)
        fprintf(stderr, "SEND (%d bytes):\n", retval);
    else
        perror("send()");

    return 0;
}