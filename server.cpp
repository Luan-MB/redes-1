#include <cstdio>
#include <bitset>
#include <bits/stdc++.h>

#include "raw_socket.h"
#include "Mensagem.hpp"
#include "Controller.hpp"

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("Escutando...\n");

    char *buffer = (char *) malloc(MAX_MSG_SIZE);
    unsigned int retval;

    Mensagem *msg, *response;

    while (true) {

        if ((retval = Controller::recvMessage(socket, buffer)) == 20) {
            if (buffer[0] == 0x7e) {

                unsigned char seq{0x0};

                msg = new Mensagem{retval, buffer};
                if ((msg->tipo == Inicio) && (msg->sequencia == seq)) {
                    response = new Mensagem{Ack, seq, 16};

                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                        seq = (seq + 1) % 16;
                    }

                    while (true) {
                        if ((retval = Controller::recvMessage(socket, buffer)) > 0) {

                            msg = new Mensagem{retval, buffer};

                            if (((msg->tipo == Midia) || (msg->tipo == Texto)) && (msg->sequencia == seq))
                                break;
                        }
                    }

                    if (msg->tipo == Midia) {
                        
                        std::string file_name = msg->dados;
                        file_name = file_name.substr(0, msg->tamanho);

                        FILE *arq = fopen(file_name.c_str(), "wb");
                        if (!arq) {
                            
                            response = new Mensagem{Erro, seq, 1, Caminho};
                            response->imprimeCamposMsg();

                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                fprintf(stderr, "SEND (%d bytes):\n", retval);
                            }

                            delete response;

                        } else {

                            response = new Mensagem{Ack, seq, 16};

                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                fprintf(stderr, "SEND (%d bytes):\n", retval);
                                seq = (seq + 1) % 16;
                            }

                            while (true) {
                                if ((retval = Controller::recvMessage(socket, buffer)) > 0) {
                                    if (buffer[0] == 0x7e) {
                                        
                                        #ifdef DEBUG
                                        fprintf(stderr, "RECV (%d bytes):\n", retval);
                                        #endif
                                        
                                        msg = new Mensagem{retval, buffer};
                                        
                                        if (msg->sequencia == seq) {

                                            if (msg->tipo == Mask) {
                                                Controller::maskMessage(msg);
                                                msg->tipo == Dados;
                                            }

                                            if (msg->tipo == Dados) {
                                                unsigned char crc = msg->crc8();
                                                if (crc != msg->crc) {
                                                    response = new Mensagem{Nack, seq, 16};
                                                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                                                    } else
                                                        perror("send()");
                                                } else {

                                                    fwrite(msg->dados, 1, msg->tamanho, arq);
                                                    response = new Mensagem{Ack, seq, 16};

                                                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                                        fprintf(stderr, "SEND (%d bytes):\n", retval);
                                                        seq = (seq + 1) % 16;
                                                    } else
                                                        perror("send()");
                                                }
                                                
                                            } else if (msg->tipo == Fim) {
                                                std::cout << "fim\n";
                                                delete msg;
                                                break;
                                            }
                                        }

                                        delete msg;
                                    }
                                } else {
                                    std::cout << "Timeout\n";
                                }
                            }

                            fclose(arq);
                        }
                    
                    } else if (msg->tipo == Texto) {

                        std::string message, sub_message;

                        sub_message = msg->dados;
                        message += sub_message.substr(0, msg->tamanho);
                        delete msg;

                        while (true) {
                            if ((retval = Controller::recvMessage(socket, buffer)) > 0) {
                                if (buffer[0] == 0x7e) {
                                    
                                    msg = new Mensagem{retval, buffer};

                                    if (msg->sequencia == seq) {
                                        msg->imprimeCamposMsg();
                                        if (msg->tipo == Texto) {
                                            sub_message = msg->dados;
                                            message += sub_message.substr(0, msg->tamanho);

                                            delete msg;
                                            
                                            response = new Mensagem{Ack, seq, 16};

                                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                                fprintf(stderr, "SEND (%d bytes):\n", retval);
                                                seq = (seq + 1) % 16;
                                            }
                                        } else if (msg->tipo == Fim) {
                                            delete msg;
                                            break;
                                        }
                                    }   
                                }
                            }
                        }

                        std::cout << message  << std::endl;
                    }
                }
            }
        }
    }

    return 0;
}