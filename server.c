#include "raw_socket.h"
#include <stdio.h>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char msg[] = "hino do palmeiras";
    
    int retval;

    if ((retval = send(socket, msg, 18, 0)) >= 0)
        fprintf(stderr, "SEND (%ld bytes):\t", sizeof(msg));
    else
        perror("send()");

    return 0;
}