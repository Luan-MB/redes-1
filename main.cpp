#include "Mensagem.hpp"
#include <iostream>
#include <string>
#include <bitset>

int main () {

    std::string msg = "😀a";
    std::cout << msg << std::endl;
    for (unsigned char c: msg) {
        std::cout << std::bitset<8>(c);
    }

    char tipo;
    std::cin >> tipo;

    unsigned char tipoMsg;

    switch (tipo) {
        case 't':
            tipoMsg = Texto;
            break;
        case 'm':
            tipoMsg = Midia;
            break;
        case 'a':
            tipoMsg = Ack;
            break;
        case 'n':
            tipoMsg = Nack;
            break;
        case 'e':
            tipoMsg = Erro;
            break;
        case 'i':
            tipoMsg = Inicio;
            break;
        case 'f':
            tipoMsg = Fim;
            break;
        case 'd':
            tipoMsg = Dados;
            break;
    }

    Mensagem m{0x7e, tipoMsg, 0xf, 0x1e, 255};
    m.imprimeCamposMsg();

    return 0;
}