#ifndef SERVIDORSOCKET_H
#define SERVIDORSOCKET_H

#include "Parqueadero.h"
#include <string>

class ServidorSocket {
private:
    int puerto;
    bool activo;
    Parqueadero* parqueadero; // Puntero al parqueadero

public:
    ServidorSocket(int p, Parqueadero* pq);
    ~ServidorSocket();

    void iniciarEscucha();
    void detener();
};

#endif // SERVIDORSOCKET_H