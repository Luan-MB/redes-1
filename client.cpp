#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <bitset>

#include "Mensagem.hpp"
#include "Controller.hpp"
#include "raw_socket.h"

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Conectado...\n");

    char opt;

    while (true) {

        std::cout << std::endl << "a) Enviar arquivo" << std::endl
                << "q) Sair" << std::endl;
        std::cin >> opt;

        if (opt == 'a') {

            char send_buffer[3780];
            size_t size;
            unsigned int retval;
            char *recv_buffer = (char *) malloc(MAX_MSG_SIZE);
            
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
            if ((retval = Controller::sendMessage(socket, &media)) >= 0) {
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

                    if ((retval = Controller::sendMessage(socket, msg)) >= 0) {
                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                    } else
                        perror("send()");

                    while (true) {
                        if ((retval = Controller::recvAck(socket, recv_buffer)) == 20) {
                            response = new Mensagem{retval, recv_buffer};

                            if ((response->tipo == Ack) && (response->sequencia == seq)) {
                                seq = (seq + 1) % 16;
                                delete response;
                                break;
                            }
                        } else if (retval == -1) {

                            if ((retval = Controller::sendMessage(socket, msg)) >= 0) {
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

                    if ((retval = Controller::sendMessage(socket, msg)) >= 0) {
                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                    } else
                        perror("send()");

                    delete msg;
                }
            }

            fclose(arq);

            Mensagem fim{Fim, 16};
            if ((retval = Controller::sendMessage(socket, &fim)) >= 0) {
                fprintf(stderr, "SEND (%d bytes):\n", retval);
            } else
                perror("send()");
        } else if (opt == 'q') {
            std::cout << "Saindo...'\n";
            break;
        }
    }

    return 0;
}