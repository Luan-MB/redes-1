#include "Mensagem.hpp"
#include <iostream>
#include <bitset>
#include <bits/stdc++.h>

Mensagem::Mensagem(unsigned char marcadorInicio,
                unsigned char tipo,
                unsigned char sequencia,
                unsigned char tamanho,
                unsigned char crc) 
{
    this->marcadorInicio = marcadorInicio;
    this->tipo = tipo;
    this->sequencia = sequencia,
    this->tamanho = tamanho,
    this->crc = crc;
}

void Mensagem::imprimeCamposMsg() {
    std::cout << std::bitset<8>(this->marcadorInicio) << ' ';
    std::cout << std::bitset<6>(this->tipo) << ' ';
    std::cout << std::bitset<4>(this->sequencia) << ' ';
    std::cout << std::bitset<6>(this->tamanho) << ' ';
    std::cout << std::bitset<8>(this->crc) << std::endl;
}
