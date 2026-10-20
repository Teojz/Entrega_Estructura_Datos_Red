#ifndef SERVIDORSOCKET_H
#define SERVIDORSOCKET_H

#include "Parqueadero.h"
#include <string>
#include <atomic>

/**
 * @brief Servidor TCP que recibe TramaVehiculo y actualiza el Parqueadero por puntero.
 * iniciarEscucha() es bloqueante (se ejecuta en un hilo aparte, lo lanza wrapper_dll.cpp)
 * y termina de forma limpia cuando se llama a detener() desde otro hilo.
 */
class ServidorSocket {
private:
    int puerto;
    std::atomic<bool> activo;      // true mientras el socket esta escuchando
    std::atomic<bool> detenido;    // true cuando se pidio detener (evita la carrera inicio/parada)
    std::atomic<int>  estado;      // 0 = iniciando, 1 = escuchando, -1 = error al iniciar, 2 = finalizado
    std::string ultimoError;       // Descripcion del error (valido cuando estado == -1)
    Parqueadero* parqueadero;      // Puntero al parqueadero

    ServidorSocket(const ServidorSocket&) = delete;
    ServidorSocket& operator=(const ServidorSocket&) = delete;

public:
    ServidorSocket(int p, Parqueadero* pq);
    ~ServidorSocket();

    void iniciarEscucha();
    void detener();

    bool estaActivo() const;
    int getEstado() const;
    std::string getUltimoError() const;
};

#endif // SERVIDORSOCKET_H
