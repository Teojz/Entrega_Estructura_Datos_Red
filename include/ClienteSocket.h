#ifndef CLIENTESOCKET_H
#define CLIENTESOCKET_H

#include "GeneradorPlacas.h"
#include "TramaVehiculo.h"
#include <string>
#include <atomic>

/**
 * @brief Cliente TCP que genera y envía una TramaVehiculo cada 2 a 5 segundos.
 */
class ClienteSocket {
private:
    std::string ipServidor;
    int puerto;
    bool redLista;                 // true si la pila de red (Winsock) se inicializo bien
    std::atomic<bool> activo;      // false => el bucle de transmision termina
    int intervaloMinMs;            // Por defecto 2000 ms
    int intervaloMaxMs;            // Por defecto 5000 ms

    ClienteSocket(const ClienteSocket&) = delete;
    ClienteSocket& operator=(const ClienteSocket&) = delete;

public:
    ClienteSocket(const std::string& ip, int p);
    ~ClienteSocket();

    bool enviarTrama(const TramaVehiculo& trama);
    void iniciarTransmision(GeneradorPlacas* gen);

    // Extensiones (no alteran el UML original): parada limpia y rango de intervalo configurable
    void detener();
    void configurarIntervalo(int minMs, int maxMs);
};

#endif // CLIENTESOCKET_H
