#ifndef MENSAGEM_HPP
#define MENSAGEM_HPP

#include <iostream>

enum Tipos {
    Texto = 0x01,
    Midia = 0x10,
    Ack = 0x0A,
    Nack = 0x00,
    Erro = 0x1E,
    Inicio = 0x1D,
    Fim = 0x0F,
    Dados = 0x0D
};

class Mensagem {
    public:
        Mensagem(const unsigned int, const char *);
        Mensagem(const unsigned char, const unsigned char, const unsigned char, const char *);

        unsigned int getTamanhoPacote() const {return this->tamanho + 4;};

        void imprimeDados() const;

        void imprimeCamposMsg() const;
        char *montaPacote() const;

    private:
        unsigned char marcadorInicio: 8;
        unsigned char tipo: 6;
        unsigned char sequencia: 4;
        unsigned char tamanho: 6;
        char dados[63];
        unsigned char crc: 8;
};

#endif