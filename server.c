#include "raw_socket.h"
#include <stdio.h>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");
    
    printf("%ld\n", send(socket, "Oi alionco\n", 12, 0));

    return 0;
}