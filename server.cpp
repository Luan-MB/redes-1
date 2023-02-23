#include <cstdio>
#include <bitset>
#include <bits/stdc++.h>
#include <ctime>

#include "raw_socket.h"
#include "Mensagem.hpp"
#include "Controller.hpp"

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("Deu ruim familia\n");
        return 1;
    }

    printf("\033[1;36mEscutando...\n\n\033[0m");

    FILE *log = fopen("server.log", "w");
    if (!log) {
        std::cerr << "\033[31mFalha ao criar arquivo de log do server!\033[0m" << std::endl;
        return 1;
    }

    char *buffer = (char *) malloc(MSG_SIZE);
    unsigned int retval;
    Mensagem *msg, *response;
    std::string user_name;

    while(true) {
        if ((retval = Controller::recvMessage(socket, buffer)) >= 0) {
            if (buffer[0] == 0x7e) {
                
                msg = new Mensagem{buffer};
                
                if ((msg->tipo == Conexao) && (msg->sequencia == 0)) {
                    fprintf(log, "RECV (%d bytes): seq = NULL, tipo = CONEXAO\n", msg->tamanho);

                    response = new Mensagem{Ack, 0};

                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                        fprintf(log, "SEND (%d bytes): seq = NULL, tipo ACK\n", response->tamanho);
                    }
                    user_name = msg->dados;
                    user_name = user_name.substr(0, msg->tamanho);
                    break;
                }
            }
        }
    }

    fprintf(log, "O usuario %s conectou-se ao server\n", user_name.c_str());

    while (true) {

        if ((retval = Controller::recvMessage(socket, buffer)) >= 0) {
            if (buffer[0] == 0x7e) {

                unsigned char seq{0x0};

                fprintf(log, "RECV (%d bytes): seq = %02d, tipo = Inicio\n", retval, seq);

                msg = new Mensagem{buffer};
                if ((msg->tipo == Inicio) && (msg->sequencia == seq)) {
                    response = new Mensagem{Ack, seq};

                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                        fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
                        seq = (seq + 1) % 16;
                    }

                    while (true) {
                        if ((retval = Controller::recvMessage(socket, buffer)) >= 0) {

                            msg = new Mensagem{buffer};

                            if (((msg->tipo == Midia) || (msg->tipo == Texto)) && (msg->sequencia == seq))
                                break;
                        }
                    }

                    if (msg->tipo == Midia) {
                        
                        std::string file_name = msg->dados;
                        file_name = "downloads/" + file_name.substr(0, msg->tamanho);

                        FILE *arq = fopen(file_name.c_str(), "wb");
                        if (!arq) {
                            
                            response = new Mensagem{Erro, seq, 1, Caminho};

                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                fprintf(log, "SEND (%d bytes): seq = %02d, tipo = Erro\n", retval, seq);
                            }

                            delete response;

                        } else {

                            response = new Mensagem{Ack, seq};

                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
                                seq = (seq + 1) % 16;
                            }

                            while (true) {
                                if ((retval = Controller::recvMessage(socket, buffer)) >= 0) {
                                    if (buffer[0] == 0x7e) {
                                        
                                        #ifdef DEBUG
                                        fprintf(stderr, "RECV (%d bytes):\n", retval);
                                        #endif
                                        
                                        msg = new Mensagem{buffer};
                                        
                                        if (msg->sequencia == seq) {

                                            if (msg->tipo == Mask) {
                                                Controller::maskMessage(msg);
                                                msg->tipo == Dados;
                                            }

                                            if (msg->tipo == Dados) {
                                                unsigned char crc = msg->crc8();
                                                if (crc != msg->crc) {
                                                    response = new Mensagem{Nack, seq};
                                                    if ((retval = Controller::sendMessage(socket, response)) >= 0)
                                                        fprintf(log, "SEND (%d bytes): seq = %02d, tipo Nack\n", retval, seq);

                                                } else {

                                                    fwrite(msg->dados, 1, msg->tamanho, arq);
                                                    response = new Mensagem{Ack, seq};

                                                    if ((retval = Controller::sendMessage(socket, response)) >= 0)
                                                        fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
                                                        seq = (seq + 1) % 16;
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

                            time_t now = time(0);
                            tm *ltm = localtime(&now);

                            printf("\033[1;35m[%02d-%02d-%d  %02d:%02d:%02d]<%s>: \033[0m", ltm->tm_mday, 
                            ltm->tm_mon + 1, 1900 + ltm->tm_year, 5+ltm->tm_hour, 30+ltm->tm_min, ltm->tm_sec, user_name.c_str());
                            std::cout << "Arquivo recebido! Pode ser encontrado em " 
                            << file_name << std::endl << std::endl;
                        }
                    
                    } else if (msg->tipo == Texto) {

                        response = new Mensagem{Ack, seq};

                        if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                            fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
                            seq = (seq + 1) % 16;
                        }

                        std::string message, sub_message;

                        sub_message = msg->dados;
                        message += sub_message.substr(0, msg->tamanho);
                        
                        delete msg;

                        while (true) {
                            if ((retval = Controller::recvMessage(socket, buffer)) >= 0) {
                                if (buffer[0] == 0x7e) {
                                    
                                    msg = new Mensagem{buffer};

                                    if (msg->sequencia == seq) {
                                        if (msg->tipo == Texto) {
                                            sub_message = msg->dados;
                                            message += sub_message.substr(0, msg->tamanho);

                                            delete msg;
                                            
                                            response = new Mensagem{Ack, seq};

                                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                                fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
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

                        time_t now = time(0);
                        tm *ltm = localtime(&now);

                        printf("\033[1;35m[%02d-%02d-%d  %02d:%02d:%02d]<%s>: \033[0m", ltm->tm_mday, 
                        ltm->tm_mon + 1, 1900 + ltm->tm_year, 5+ltm->tm_hour, 30+ltm->tm_min, ltm->tm_sec, user_name.c_str());
                        std::cout << message  << std::endl;
                    }
                } else if (msg->tipo == Quit) {
                    break;
                }
            }
        }
    }

    printf("\033[1;36m\nEncerrando...\n\033[0m");
    fclose(log);

    return 0;
}