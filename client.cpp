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

    FILE *log = fopen("client.log", "w");
    if (!log) {
        std::cerr << "Falha ao criar arquivo de log" << std::endl;
        return 1;
    }

    char opt;

    while (true) {

        std::cout << std::endl << "a) Enviar arquivo" << std::endl
                << "i) Inserir mensagem" << std::endl
                << "q) Sair" << std::endl;
        std::cin >> opt;

        if (opt == 'a') {
            
            std::string file_name, file_path;

            std::cout << std::endl << "Caminho do arquivo: ";
            std::cin >> file_path;

            std::cout << std::endl << "Nome do arquivo: ";
            std::cin >> file_name;

            std::cout << std::endl;

            FILE *arq = fopen(file_path.c_str(), "rb");
            if (!arq) {
                std::cerr << "Falha ao abrir o arquivo: " << file_path << std::endl;
            } else {

                char send_buffer[3780];
                size_t size;
                unsigned int retval;
                char *recv_buffer = (char *) malloc(MAX_MSG_SIZE);
                Mensagem *response;
                unsigned char seq{0x0};

                Mensagem inicio{Inicio, seq, 16};
                if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                    fprintf(log, "SEND (%d bytes): seq = %d, tipo = Inicio\n", retval, seq);

                while (true) {
                    if ((retval = Controller::recvAck(socket, recv_buffer)) == 20) {
                        response = new Mensagem{retval, recv_buffer};

                        if ((response->tipo == Ack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): seq = %d, tipo = Ack\n", retval, seq);
                            seq = (seq + 1) % 16;
                            delete response;
                            break;
                        } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): tipo = Nack\n", retval);
                            if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                                fprintf(log, "RE-SEND (%d bytes): tipo = Inicio\n", retval);
                            delete response;
                        }
                    } else if (retval == -1) {

                        if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                            fprintf(log, "RE-SEND (%d bytes): tipo = Inicio\n", retval);
                    }
                }

                Mensagem media{Midia, seq, (unsigned char) file_name.length(), file_name.c_str()};
                if ((retval = Controller::sendMessage(socket, &media)) >= 0)
                    fprintf(log, "SEND (%d bytes): seq = %d, tipo = Media\n", retval, seq);

                while (true) {
                    if ((retval = Controller::recvAck(socket, recv_buffer)) == 20) {
                        response = new Mensagem{retval, recv_buffer};

                        if ((response->tipo == Ack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): seq = %d, tipo = Ack\n", retval, seq);
                            delete response;
                            seq = (seq + 1) % 16;
                            break;
                        } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): tipo = Nack\n", retval);
                            if ((retval = Controller::sendMessage(socket, &media)) >= 0)
                                fprintf(log, "RE-SEND (%d bytes): tipo = Media\n", retval);
                            delete response;
                        }
                    } else if (retval == -1) {

                        if ((retval = Controller::sendMessage(socket, &media)) >= 0)
                            fprintf(log, "RE-SEND (%d bytes): tipo = Media\n", retval);
                    }
                }

                Mensagem *msg;
                char sub_buffer[MAX_DATA_SIZE];
                unsigned int i;
            
                while (size = fread(send_buffer, 1, 3780, arq)) {
                    unsigned int n_packs = (size / MAX_DATA_SIZE);

                    for (i=0; i<n_packs; ++i) {
                        memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], MAX_DATA_SIZE);
                        msg = new Mensagem{Dados, seq, MAX_DATA_SIZE, sub_buffer};

                        if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                            fprintf(log, "SEND (%d bytes): seq = %d, tipo = Dados\n", retval, seq);

                        while (true) {
                            if ((retval = Controller::recvAck(socket, recv_buffer)) == 20) {
                                response = new Mensagem{retval, recv_buffer};

                                if ((response->tipo == Ack) && (response->sequencia == seq)) {
                                    fprintf(log, "RECV (%d bytes): seq = %d, tipo = Ack\n", retval, seq);
                                    seq = (seq + 1) % 16;
                                    delete response;
                                    break;
                                } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                                    fprintf(log, "RECV (%d bytes): seq = %d, tipo = Nack\n", retval, seq);
                                    if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                        fprintf(log, "RE-SEND (%d bytes): seq = %d, tipo = Dados\n", retval, seq);
                                    delete response;
                                }
                            } else if (retval == -1) {

                                if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                    fprintf(log, "RE-SEND (%d bytes): seq = %d, tipo = Dados\n", retval, seq);
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
            }

        } else if (opt == 'i') {
            
            std::string message, eol;

            std::cout << std::endl << "Insira a mensagem: ";
            
            std::getline(std::cin, eol);
            std::getline(std::cin, message);

            std::cout << message << std::endl;

        } else if (opt == 'q') {
            std::cout << "Saindo...\n";
            break;
        }
    }

    fclose(log);
    return 0;
}