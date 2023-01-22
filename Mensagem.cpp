#include "Mensagem.hpp"
#include <iostream>
#include <bitset>
#include <bits/stdc++.h>

Mensagem::Mensagem(const unsigned int tamanho, const char *pacote)
    : dados{0} 
{
    this->marcadorInicio = pacote[0] & 0xff;
    this->tipo = (pacote[1] >> 2) & 0x3f;
    this->sequencia = ((pacote[1] & 0x3) >> 2)  | ((pacote[2] << 2) & 0x3);
    this->tamanho = pacote[2] & 0x3f;

    unsigned int byte_index = 3;

    for (int i=0; i<this->tamanho; ++i) {
        this->dados[i] = pacote[byte_index++];
    }   

    this->crc = pacote[byte_index];
}

Mensagem::Mensagem(const unsigned char tipo,
                const unsigned char sequencia,
                const unsigned char tamanho,
                const char *dados)
    : dados{0}
{
    this->marcadorInicio = 0x7e;
    this->tipo = tipo;
    this->sequencia = sequencia,
    this->tamanho = tamanho,
    memcpy(this->dados, dados, tamanho);
    this->crc =  0xff;
}

void Mensagem::imprimeCamposMsg() const {
    std::cout << "Marcador inicio: " << std::bitset<8>(this->marcadorInicio) << std::endl;
    std::cout << "Tipo: " << std::bitset<6>(this->tipo) << std::endl;
    std::cout << "Sequencia: " << std::bitset<4>(this->sequencia) << std::endl;
    std::cout << "Tamanho: " << std::bitset<6>(this->tamanho) << std::endl;
    
    std::cout << "Dados: ";
    for (char byte: this->dados) {
        std::cout << std::bitset<8>(byte);
    }
    std::cout << std::endl;

    std::cout << "Crc: " << std::bitset<8>(this->crc) << std::endl;
}

char *Mensagem::montaPacote() const {
    std::cout << this->tamanho << std::endl;
    char *pacote = (char *) malloc(4 + this->tamanho);
    
    pacote[0] = this->marcadorInicio;
    pacote[1] = (this->tipo << 2) | ((this->sequencia << 2) & 0x3);
    pacote[2] = ((this->sequencia & 0x3) << 6) | (this->tamanho);

    unsigned int byte_index = 3;

    for (int i=0; i<this->tamanho; ++i) {
        pacote[byte_index++] = this->dados[i];
    }

    pacote[byte_index] = this->crc;
    
    return pacote;
}
