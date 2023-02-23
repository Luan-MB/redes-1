#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <bitset>
#include <sys/stat.h>

#include "Mensagem.hpp"
#include "Controller.hpp"
#include "raw_socket.h"

int main () {

    int socket = cria_raw_socket("lo");
    if (socket < 0) {
        printf("\033[31mFalha ao conectar-se ao socket\n\033[0m");
        return 1;
    }

    FILE *log = fopen("client.log", "w");
    if (!log) {
        std::cerr << "\033[31mFalha ao criar arquivo de log do client!\033[0m" << std::endl;
        return 1;
    }

    int retval;
    char *recv_buffer = (char *) malloc(MSG_SIZE);
    Mensagem *response;

    std::string user_name;
    std::cout << "Usuário >>> ";
    std::cin >> user_name;

    Mensagem connection{Conexao, (unsigned char) user_name.length(), user_name.c_str()};
    if ((retval = Controller::sendMessage(socket, &connection)) >= 0)
        fprintf(log, "SEND (%d bytes): seq = NULL, tipo = CONEXAO\n", connection.tamanho);

    while (true) {
        if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
            response = new Mensagem{recv_buffer};

            if ((response->tipo == Ack) && (response->sequencia == 0)) {
                fprintf(log, "RECV (%d bytes): seq = NULL, tipo = ACK\n", response->tamanho);
                delete response;
                break;
            } else if ((response->tipo == Nack) && (response->sequencia == 0)) {
                fprintf(log, "RECV (%d bytes): tipo = NACK\n", retval);
                if ((retval = Controller::sendMessage(socket, &connection)) >= 0)
                    fprintf(log, "\tRE-SEND (%d bytes): tipo = CONEXAO\n", connection.tamanho);
                delete response;
            }
        } else if (retval == -1) {
            if ((retval = Controller::sendMessage(socket, &connection)) >= 0)
                fprintf(log, "\tRE-SEND (%d bytes): tipo = CONEXAO\n", connection.tamanho);
        }
    }

    fprintf(log, "%s conectou-se ao server\n", user_name.c_str());

    printf("\033[1;36mConectado...\n\033[0m");

    char opt;

    while (true) {

        std::cout << std::endl << "\033[1;35ma) Enviar arquivo" << std::endl
                << "i) Inserir mensagem" << std::endl
                << "q) Sair\033[0m" << std::endl;
        std::cin >> opt;

        if (opt == 'a') {
            
            fprintf(log, "\n----- INICIO ENVIO DE ARQUIVO -----\n");
            std::string file_name, file_path;

            std::cout << std::endl << "Caminho do arquivo: ";
            std::cin >> file_path;

            int index = file_path.find_last_of("/");
            file_name = file_path.substr(index + 1);

            std::cout << std::endl;

            FILE *arq = fopen(file_path.c_str(), "rb");
            if (!arq) {
                fprintf(log, "Falha ao abrir o arquivo em: %s. Arquivo nao encontrado!\n", file_path.c_str());
                std::cerr << "\033[31mFalha ao abrir o arquivo: " << file_path << "\033[0m" << std::endl;
            } else {
                
                fprintf(log, "Sucesso ao abrir o arquivo em: %s\n", file_path.c_str());

                struct stat sb;
                stat(file_path.c_str(), &sb);
                
                fprintf(log, "O arquivo possui %ld bytes\n", sb.st_size);
                fprintf(log, "Serao necessarias %ld mensagens\n", (sb.st_size / MAX_DATA_SIZE) + 1);

                char send_buffer[3780];
                size_t size;
                unsigned char seq{0x0};
                bool erro{false};

                Mensagem inicio{Inicio, seq};
                if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                    fprintf(log, "SEND (%d bytes): seq = %02d, tipo = INICIO\n", inicio.tamanho, seq);

                while (true) {
                    if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
                        response = new Mensagem{recv_buffer};

                        if ((response->tipo == Ack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): seq = %02d, tipo = ACK\n", response->tamanho, seq);
                            seq = (seq + 1) % 16;
                            delete response;
                            break;
                        } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): tipo = NACK\n", retval);
                            if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                                fprintf(log, "\tRE-SEND (%d bytes): tipo = INICIO\n", inicio.tamanho);
                            delete response;
                        }
                    } else if (retval == -1) {

                        if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                            fprintf(log, "\tRE-SEND (%d bytes): tipo = INICIO\n", inicio.tamanho);
                    }
                }

                Mensagem media{Midia, seq, (unsigned char) file_name.length(), file_name.c_str()};
                if ((retval = Controller::sendMessage(socket, &media)) >= 0)
                    fprintf(log, "SEND (%d bytes): seq = %02d, tipo = MEDIA\n", media.tamanho, seq);

                while (true) {
                    if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
                        response = new Mensagem{recv_buffer};

                        if ((response->tipo == Ack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): seq = %02d, tipo = ACK\n", response->tamanho, seq);
                            delete response;
                            seq = (seq + 1) % 16;
                            break;
                        } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): tipo = NACK\n", retval);
                            if ((retval = Controller::sendMessage(socket, &media)) >= 0)
                                fprintf(log, "\tRE-SEND (%d bytes): tipo = MEDIA\n", media.tamanho);
                            delete response;
                        } else if ((response->tipo == Erro) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): tipo = ERRO\n", response->tamanho);
                            erro = true;

                            switch (response->dados[0]) {
                                case 0x66:
                                    std::cerr << "\033[31mFalha ao criar o arquivo destino."
                                    << ". Pasta downloads nao encontrada!\033[0m" << std::endl;
                                    break;
                            }
                            delete response;
                            break;
                        }
                    } else if (retval == -1) {

                        if ((retval = Controller::sendMessage(socket, &media)) >= 0)
                            fprintf(log, "\tRE-SEND (%d bytes): tipo = MEDIA\n", retval);
                    }
                }

                if (!erro) {
                    Mensagem *msg;
                    char sub_buffer[MAX_DATA_SIZE];
                    unsigned int i;
                
                    while (size = fread(send_buffer, 1, 3780, arq)) {
                        unsigned int n_packs = (size / MAX_DATA_SIZE);

                        for (i=0; i<n_packs; ++i) {
                            memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], MAX_DATA_SIZE);
                            msg = new Mensagem{Dados, seq, MAX_DATA_SIZE, sub_buffer};

                            if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                fprintf(log, "SEND (%d bytes): seq = %02d, tipo = DADOS\n", msg->tamanho, seq);

                            while (true) {
                                if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
                                    response = new Mensagem{recv_buffer};

                                    if ((response->tipo == Ack) && (response->sequencia == seq)) {
                                        fprintf(log, "RECV (%d bytes): seq = %02d, tipo = ACK\n", response->tamanho, seq);
                                        seq = (seq + 1) % 16;
                                        delete response;
                                        break;
                                    } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                                        fprintf(log, "RECV (%d bytes): seq = %02d, tipo = NACK\n", response->tamanho, seq);
                                        if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = DADOS\n", msg->tamanho, seq);
                                        delete response;
                                    }
                                } else if (retval == -1) {

                                    if ((retval = Controller::resendMessage(socket, msg)) >= 0) {
                                        if (msg->tipo == Mask)
                                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = MASK\n", msg->tamanho, seq);
                                        else
                                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = DADOS\n", msg->tamanho, seq);
                                    }
                                }
                            }
                        
                            delete msg;
                        }

                        unsigned int remainder = size % MAX_DATA_SIZE;

                        if (remainder > 0) {
                            memcpy(sub_buffer, &send_buffer[MAX_DATA_SIZE * i], remainder);
                            msg = new Mensagem{Dados, seq, (unsigned char) remainder, sub_buffer};

                            if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                fprintf(log, "SEND (%d bytes): seq = %02d, tipo = DADOS\n", msg->tamanho, seq);
                            while (true) {
                                if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
                                    response = new Mensagem{recv_buffer};

                                    if ((response->tipo == Ack) && (response->sequencia == seq)) {
                                        fprintf(log, "RECV (%d bytes): seq = %02d, tipo = ACK\n", response->tamanho, seq);
                                        seq = (seq + 1) % 16;
                                        delete response;
                                        break;
                                    } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                                        fprintf(log, "RECV (%d bytes): seq = %02d, tipo = NACK\n", response->tamanho, seq);
                                        if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = DADOS\n", msg->tamanho, seq);
                                        delete response;
                                    }
                                } else if (retval == -1) {

                                    if ((retval = Controller::resendMessage(socket, msg)) >= 0) {
                                        if (msg->tipo == Mask)
                                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = MASK\n", msg->tamanho, seq);
                                        else
                                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = DADOS\n", msg->tamanho, seq);
                                    }
                                }
                            }
                        
                            delete msg;
                        }
                    }

                    fclose(arq);

                    Mensagem fim{Fim, seq, 16};
                    if ((retval = Controller::sendMessage(socket, &fim)) >= 0)
                        fprintf(log, "SEND (%d bytes): seq = %02d, tipo = FIM\n", fim.tamanho, seq);
                    printf("\033[32mArquivo enviado com sucesso!\n\033[0m");
                }
            }
            fprintf(log, "----- FIM ENVIO DE ARQUIVO -----\n\n");

        } else if (opt == 'i') {
            
            fprintf(log, "\n----- INICIO ENVIO DE TEXTO -----\n");
            std::string message, eol;

            std::cout << std::endl << "Insira a mensagem: ";
            
            std::getline(std::cin, eol);
            std::getline(std::cin, message);

            unsigned int num_packs = (message.length() / MAX_DATA_SIZE) + 1;
            unsigned int remaining_size = message.length();
            unsigned char seq{0x0};
            unsigned int retval;
            char *recv_buffer = (char *) malloc(MSG_SIZE);

            fprintf(log, "O texto possui %ld bytes\n", message.length());
            fprintf(log, "Serao necessarias %d mensagens\n", num_packs);

            Mensagem *msg, *response;

            Mensagem inicio{Inicio, seq};
            
            if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                fprintf(log, "SEND (%d bytes): seq = %02d, tipo = INICIO\n", inicio.tamanho, seq);

            while (true) {
                if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
                    response = new Mensagem{recv_buffer};

                    if ((response->tipo == Ack) && (response->sequencia == seq)) {
                        fprintf(log, "RECV (%d bytes): seq = %02d, tipo = ACK\n", response->tamanho, seq);
                        seq = (seq + 1) % 16;
                        delete response;
                        break;
                    } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                        fprintf(log, "RECV (%d bytes): tipo = NACK\n", retval);
                        if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                            fprintf(log, "\tRE-SEND (%d bytes): tipo = INICIO\n", inicio.tamanho);
                        delete response;
                    }
                } else if (retval == -1) {

                    if ((retval = Controller::sendMessage(socket, &inicio)) >= 0)
                        fprintf(log, "\tRE-SEND (%d bytes): tipo = INICIO\n", inicio.tamanho);
                }
            }

            for (int i=0; i<num_packs; ++i) {
                if (remaining_size > MAX_DATA_SIZE) {
                    std::string message_part{message.substr(MAX_DATA_SIZE*i, MAX_DATA_SIZE)};
                    msg = new Mensagem{Texto, seq, MAX_DATA_SIZE, message_part.c_str()};
                    remaining_size -= MAX_DATA_SIZE;
                } else {
                    std::string message_part{message.substr(MAX_DATA_SIZE*i, remaining_size)};
                    msg = new Mensagem{Texto, seq, (unsigned char) remaining_size, message_part.c_str()};
                }
                
                if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                    fprintf(log, "SEND (%d bytes): seq = %02d, tipo = TEXTO\n", msg->tamanho, seq);

                while (true) {
                    if ((retval = Controller::recvAck(socket, recv_buffer)) >= 0) {
                        response = new Mensagem{recv_buffer};

                        if ((response->tipo == Ack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): seq = %02d, tipo = ACK\n", response->tamanho, seq);
                            seq = (seq + 1) % 16;
                            delete response;
                            break;
                        } else if ((response->tipo == Nack) && (response->sequencia == seq)) {
                            fprintf(log, "RECV (%d bytes): seq = %02d, tipo = NACK\n", response->tamanho, seq);
                            if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                                fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = TEXTO\n", msg->tamanho, seq);
                            delete response;
                        }
                    } else if (retval == -1) {

                        if ((retval = Controller::sendMessage(socket, msg)) >= 0)
                            fprintf(log, "\tRE-SEND (%d bytes): seq = %02d, tipo = TEXTO\n", msg->tamanho, seq);
                    }
                }
            }

            Mensagem fim{Fim, seq};
            if ((retval = Controller::sendMessage(socket, &fim)) >= 0) {
                fprintf(stderr, "SEND (%d bytes):\n", retval);
            } else
                perror("send()");

            fprintf(log, "----- FIM ENVIO DE TEXTO -----\n\n");

        } else if (opt == 'q') {

            Mensagem sair{Quit, 16};
            if (Controller::sendMessage(socket, &sair) >= 0)
                printf("\033[1;36m\nEncerrando...\n\033[0m");
            break;
        }
    }

    fclose(log);
    return 0;
}