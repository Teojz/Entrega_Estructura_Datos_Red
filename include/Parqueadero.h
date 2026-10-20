#ifndef PARQUEADERO_H
#define PARQUEADERO_H

#include <string>
#include <mutex>
#include <deque>
#include "Celda.h"

/**
 * @brief Administra la estructura de datos del parqueadero en memoria dinámica.
 * Contiene el arreglo dinámico Celda* celdas y la lógica de negocio para
 * ocupación, búsqueda y liberación por placa repetida.
 * Además mantiene una bitácora de eventos (registro de actividad del socket) que la GUI consulta.
 */
class Parqueadero {
private:
    int capacidadTotal;
    Celda* celdas;
    mutable std::mutex mutexAcceso;    // Protege el arreglo de celdas (hilo del socket vs. hilo de la GUI)
    mutable std::mutex mutexBitacora;  // Protege la bitácora (orden de bloqueo: mutexAcceso -> mutexBitacora)
    std::deque<std::string> bitacora;  // Últimos eventos registrados

    static const std::size_t MAX_BITACORA = 100;

    // Deshabilitar copia para garantizar la integridad del puntero celdas
    Parqueadero(const Parqueadero&) = delete;
    Parqueadero& operator=(const Parqueadero&) = delete;

    // Búsqueda sin bloqueo: SOLO se llama con mutexAcceso ya adquirido.
    int buscarPlacaSinBloqueo(const std::string& placa) const;

public:
    // Constructor y Destructor
    explicit Parqueadero(int cap);
    ~Parqueadero();

    // Métodos principales según diagrama UML
    std::string procesarEventoPlaca(std::string placa, int celdaSugerida, std::string hora);
    int buscarPlaca(std::string placa);
    std::string obtenerEstadoSerializado() const;

    // Métodos utilitarios para análisis y estadísticas
    int getCapacidadTotal() const;
    int getCeldasOcupadas() const;
    int getCeldasDisponibles() const;

    // Bitácora de actividad (se muestra en la GUI)
    void registrarEvento(const std::string& mensaje);
    std::string obtenerBitacora(int maxLineas = 50) const;
};

#endif // PARQUEADERO_H
