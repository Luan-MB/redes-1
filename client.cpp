#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <bitset>
#include <sys/time.h>

#include "Mensagem.hpp"
#include "raw_socket.h"

long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
int valid_protocol(char* buffer, int tamanho_buffer) {
    if (tamanho_buffer <= 0) { return 0; }
    // insira a sua validação de protocolo aqui
    return buffer[0] == 0x7e;
}
 
// retorna -1 se deu timeout, ou quantidade de bytes lidos
int receive_message(int socket, int timeoutMillis, char* buffer, int tamanho_buffer) {
    long long start = timestamp();
    struct timeval timeout = { .tv_sec = 0, .tv_usec = timeoutMillis * 1000 };
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    int bytes_lidos;
    do {
        bytes_lidos = recv(socket, buffer, tamanho_buffer, 0);
        if (valid_protocol(buffer, bytes_lidos)) { return bytes_lidos; }
    } while (timestamp() - start <= timeoutMillis);
    return -1;
}

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char send_buffer[3780];
    size_t size;
    unsigned int retval;

    char *recv_buffer = (char *) malloc(67);
    
    std::string file_name, file_path;

    std::cout << std::endl << "Caminho do arquivo: ";
    std::cin >> file_path;

    std::cout << std::endl << "Nome do arquivo: ";
    std::cin >> file_name;

    std::cout << std::endl;

/*     Mensagem inicio{Inicio, 16};
    if ((retval = send(socket, inicio.montaPacote(), inicio.getTamanhoPacote(), 0)) >= 0) {
        fprintf(stderr, "SEND (%d bytes):\n", retval);
    } else
        perror("send()"); */

    Mensagem media{Midia, (unsigned char) file_name.length(), file_name.c_str()};
    if ((retval = send(socket, media.montaPacote(), media.getTamanhoPacote(), 0)) >= 0) {
        fprintf(stderr, "SEND (%d bytes):\n", retval);
    } else
        perror("send()");

    FILE *arq = fopen(file_path.c_str(), "rb");

    Mensagem *msg, *response;
    unsigned char seq{0x0};
    char sub_buffer[MAX_DATA_SIZE];
    unsigned int i;
    
    while (size = fread(send_buffer, 1, 3780, arq)) {
        unsigned int n_packs = (size / MAX_DATA_SIZE);

        for (i=0; i<n_packs; ++i) {
            memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], MAX_DATA_SIZE);
            msg = new Mensagem{Dados, seq, MAX_DATA_SIZE, sub_buffer};

            if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0) {
                fprintf(stderr, "SEND (%d bytes):\n", retval);
            } else
                perror("send()");

            while (true) {
                if ((retval = receive_message(socket, 300, recv_buffer, 67)) == 20) {
                    response = new Mensagem{retval, recv_buffer};

                    if ((response->tipo == Ack) && (response->sequencia == seq)) {
                        seq = (seq + 1) % 16;
                        delete response;
                        break;
                    }
                } else if (retval == -1) {

                    if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0) {
                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                    } else
                        perror("send()");
                }
            }
        
            delete msg;
        }

        unsigned int remainder = size % MAX_DATA_SIZE;

        if (remainder > 0) {
            memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], remainder);
            msg = new Mensagem{Dados, seq, (unsigned char) remainder, sub_buffer};
            seq = (seq + 1) % 16;

            if ((retval = send(socket, msg->montaPacote(), msg->getTamanhoPacote(), 0)) >= 0) {
                fprintf(stderr, "SEND (%d bytes):\n", retval);
            } else
                perror("send()");

            delete msg;
        }
    }

    fclose(arq);

    Mensagem fim{Fim, 16};
    if ((retval = send(socket, fim.montaPacote(), fim.getTamanhoPacote(), 0)) >= 0) {
        fprintf(stderr, "SEND (%d bytes):\n", retval);
    } else
        perror("send()");

    return 0;
}