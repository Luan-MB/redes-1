#include "raw_socket.h"
#include "Mensagem.hpp"

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char *buffer = (char *) malloc(67);

    int retval;

    if ((retval = recv(socket, buffer, 67, 0)) > 0) {
        fprintf(stderr, "RECV (%d bytes):\n", retval);
        Mensagem msg{retval, buffer};
        msg.imprimeCamposMsg();
        msg.imprimeDados();
    }

    return 0;
}