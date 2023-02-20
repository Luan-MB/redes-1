#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "Mensagem.hpp"

#define TIMEOUT 500

class Controller {
    public:
        static int sendMessage(int, Mensagem*);
        static void maskMessage(Mensagem *);
        static int resendMessage(int, Mensagem*);
        static int recvMessage(int, char *);
        static int recvAck(int, char *);
};

#endif