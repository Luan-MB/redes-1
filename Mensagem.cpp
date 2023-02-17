#include "Mensagem.hpp"
#include <iostream>
#include <bitset>
#include <bits/stdc++.h>

Mensagem::Mensagem(const unsigned char tipo, const unsigned char tamanho)
    : marcadorInicio{0x7e}, tipo{tipo}, sequencia{0x0}, dados{0}, tamanho{tamanho}
{}

Mensagem::Mensagem(const unsigned int tamanho, const char *pacote)
    : dados{0}
{
    this->marcadorInicio = pacote[0] & 0xff;
    this->tipo = (pacote[1] >> 2) & 0x3f;
    this->sequencia = ((pacote[1] & 0x3) << 2)  | ((pacote[2] >> 6) & 0x3);
    this->tamanho = pacote[2] & 0x3f;

    memcpy(this->dados, &pacote[3], this->tamanho);  

    this->crc = pacote[tamanho-1];
}

Mensagem::Mensagem(const unsigned char tipo,
                const unsigned char sequencia,
                const unsigned char tamanho,
                const char *dados)
    : marcadorInicio{0x7e}, tipo{tipo}, sequencia{sequencia}, tamanho{tamanho}, dados{0}
{
    if (tamanho < 16) this->tamanho = 16;
    memcpy(this->dados, &dados[0], tamanho);
}

void Mensagem::imprimeCamposMsg() const {
    std::cout << "Marcador inicio: " << std::bitset<8>(this->marcadorInicio) << std::endl;
    std::cout << "Tipo: " << std::bitset<6>(this->tipo) << std::endl;
    std::cout << "Sequencia: " << std::bitset<4>(this->sequencia) << std::endl;
    std::cout << "Tamanho: " << std::bitset<6>(this->tamanho) << std::endl;
    
    std::cout << "Dados: ";
    for (char byte: this->dados) {
        std::cout << std::bitset<8>(byte) << ' ';
    }
    std::cout << std::endl;

    std::cout << "Crc: " << std::bitset<8>(this->crc) << std::endl << std::endl;
}

char *Mensagem::montaPacote() const {
    char *pacote = (char *) malloc(4 + this->tamanho);
    
    pacote[0] = this->marcadorInicio;
    pacote[1] = (this->tipo << 2) | ((this->sequencia >> 2) & 0x3);
    pacote[2] = ((this->sequencia & 0x3) << 6) | (this->tamanho);

    memcpy(&pacote[3], this->dados, this->tamanho);

    pacote[3 + this->tamanho] = this->crc;
    
    return pacote;
}