#ifndef MENSAGEM_HPP
#define MENSAGEM_HPP

#include <iostream>

#define MAX_MSG_SIZE 67
#define MAX_DATA_SIZE 63

enum Tipos {
    Texto = 0x01,
    Midia = 0x10,
    Ack = 0x0A,
    Nack = 0x00,
    Erro = 0x1E,
    Inicio = 0x1D,
    Fim = 0x0F,
    Dados = 0x0D,
    Mask = 0x0E,
    Quit = 0x1F
};

enum Erros {
    Tamanho = 0x65,
    Caminho = 0x66
};

class Mensagem {
    public:
        Mensagem();
        Mensagem(const unsigned int, const char *);
        Mensagem(const unsigned char, const unsigned char);
        Mensagem(const unsigned char, const unsigned char, const unsigned char);
        Mensagem(const unsigned char, const unsigned char, const char *);
        Mensagem(const unsigned char, const unsigned char, const unsigned char, Erros);
        Mensagem(const unsigned char, const unsigned char, const unsigned char, const char *);

        unsigned int getTamanhoPacote() const;

        void imprimeCamposMsg() const;
        char *montaPacote() const;

        unsigned char crc8();

        unsigned char marcadorInicio: 8;
        unsigned char tipo: 6;
        unsigned char sequencia: 4;
        unsigned char tamanho: 6;
        char dados[MAX_DATA_SIZE];
        unsigned char crc: 8;
};
#endif