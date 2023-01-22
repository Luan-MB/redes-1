#include "raw_socket.h"
#include "Mensagem.hpp"


int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char txt[] = "hino do palmeiras😁";
    Mensagem msg{Texto, 0, 22, txt};

    char *pacote{msg.montaPacote()};
    unsigned int tamanhoPacote{msg.getTamanhoPacote()};
    
    msg.imprimeCamposMsg();
    int retval;

    if ((retval = send(socket, pacote, tamanhoPacote, 0)) >= 0)
        fprintf(stderr, "SEND (%d bytes):\t", tamanhoPacote);
    else
        perror("send()");

    return 0;
}