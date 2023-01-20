#include "raw_socket.h"
#include <stdio.h>

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char *buffer = malloc(12);

    printf("%ld\n", recv(socket, buffer, 18, 0));
    printf("%s\n", buffer);

    return 0;
}