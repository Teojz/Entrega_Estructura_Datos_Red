#ifndef CLIENTESOCKET_H
#define CLIENTESOCKET_H

#include "GeneradorPlacas.h"
#include <string>

class ClienteSocket {
private:
    std::string ipServidor;
    int puerto;

public:
    ClienteSocket(const std::string& ip, int p);
    bool enviarTrama(const std::string& trama);
    void iniciarTransmision(GeneradorPlacas* gen);
};

#endif // CLIENTESOCKET_H