#include <cstdio>
#include <bitset>
#include <bits/stdc++.h>
#include <ctime>
#include <sys/statvfs.h>

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

    char *buffer = (char *) malloc(MAX_MSG_SIZE);
    unsigned int retval;

    Mensagem *msg, *response;

    while (true) {

        if ((retval = Controller::recvMessage(socket, buffer)) == 20) {
            if (buffer[0] == 0x7e) {

                unsigned char seq{0x0};

                msg = new Mensagem{retval, buffer};
                if ((msg->tipo == Inicio) && (msg->sequencia == seq)) {
                    fprintf(log, "----- INICIO DE TRANSMISSAO -----\n");
                    fprintf(log, "RECV (%d bytes): seq = %02d, tipo = Inicio\n", retval, seq);


                    response = new Mensagem{Ack, seq, 16};

                    if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                        fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
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
                        
                        struct statvfs st;
                        unsigned int file_size;
                        memcpy(&file_size, msg->dados, 4);
                        fprintf(log, "Transmissão de arquivo com tamanho %d\n", file_size);
                        std::string file_name = &msg->dados[4];
                        file_name = file_name.substr(0, (msg->tamanho-4));

                        statvfs(".", &st);

                        if (((st.f_bsize * st.f_bavail) * 0.9) >= file_size) {

                            FILE *arq = fopen(file_name.c_str(), "wb");
                            if (!arq) {
                                
                                response = new Mensagem{Erro, seq, 1, Caminho};

                                if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                    fprintf(log, "SEND (%d bytes): seq = %02d, tipo = Erro\n", retval, seq);
                                }

                                delete response;

                            } else {

                                response = new Mensagem{Ack, seq, 16};

                                if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                    fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
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
                                                        if ((retval = Controller::sendMessage(socket, response)) >= 0)
                                                            fprintf(log, "SEND (%d bytes): seq = %02d, tipo Nack\n", retval, seq);

                                                    } else {

                                                        fwrite(msg->dados, 1, msg->tamanho, arq);
                                                        response = new Mensagem{Ack, seq, 16};

                                                        if ((retval = Controller::sendMessage(socket, response)) >= 0)
                                                            fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
                                                            seq = (seq + 1) % 16;
                                                    }
                                                    
                                                } else if (msg->tipo == Fim) {
                                                    delete msg;
                                                    break;
                                                }
                                            }

                                            delete msg;
                                        }
                                    }
                                }

                                fclose(arq);

                                time_t now = time(0);
                                tm *ltm = localtime(&now);

                                printf("\033[1;35m[%02d-%02d-%d  %02d:%02d:%02d]<client>: \033[0m", ltm->tm_mday, 
                                ltm->tm_mon + 1, 1900 + ltm->tm_year, 5+ltm->tm_hour, 30+ltm->tm_min, ltm->tm_sec);
                                std::cout << "Arquivo recebido! Pode ser encontrado em " 
                                << file_name << std::endl;
                            }
                        } else {
                            response = new Mensagem{Erro, seq, 1, Tamanho};

                            if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                                fprintf(log, "SEND (%d bytes): seq = %02d, tipo = Erro\n", retval, seq);
                            }

                            delete response;
                        }
                    
                    } else if (msg->tipo == Texto) {

                        response = new Mensagem{Ack, seq, 16};

                        if ((retval = Controller::sendMessage(socket, response)) >= 0) {
                            fprintf(log, "SEND (%d bytes): seq = %02d, tipo Ack\n", retval, seq);
                            seq = (seq + 1) % 16;
                        }

                        std::string message, sub_message;

                        sub_message = msg->dados;
                        message += sub_message.substr(0, msg->tamanho);
                        
                        delete msg;

                        while (true) {
                            if ((retval = Controller::recvMessage(socket, buffer)) > 0) {
                                if (buffer[0] == 0x7e) {
                                    
                                    msg = new Mensagem{retval, buffer};

                                    if (msg->sequencia == seq) {
                                        if (msg->tipo == Texto) {
                                            sub_message = msg->dados;
                                            message += sub_message.substr(0, msg->tamanho);

                                            delete msg;
                                            
                                            response = new Mensagem{Ack, seq, 16};

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

                        printf("\033[1;35m[%02d-%02d-%d  %02d:%02d:%02d]<client>: \033[0m", ltm->tm_mday, 
                        ltm->tm_mon + 1, 1900 + ltm->tm_year, 5+ltm->tm_hour, 30+ltm->tm_min, ltm->tm_sec);
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