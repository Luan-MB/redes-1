#include "Mensagem.hpp"
#include <iostream>
#include <bitset>
#include <bits/stdc++.h>

Mensagem::Mensagem(unsigned char tipo,
                unsigned char sequencia,
                unsigned char tamanho,
                char *dados)
    : dados{0}
{
    this->marcadorInicio = 0x7e;
    this->tipo = tipo;
    this->sequencia = sequencia,
    this->tamanho = tamanho,
    memcpy(this->dados, dados, tamanho);
    this->crc =  0xff;
}

void Mensagem::imprimeCamposMsg() {
    std::cout << std::bitset<8>(this->marcadorInicio);
    std::cout << std::bitset<6>(this->tipo);
    std::cout << std::bitset<4>(this->sequencia);
    std::cout << std::bitset<6>(this->tamanho);
    
    for (char byte: this->dados) {
        std::cout << std::bitset<8>(byte);
    }

    std::cout << std::bitset<8>(this->crc) << std::endl;
}

char *Mensagem::montaPacote() {
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
