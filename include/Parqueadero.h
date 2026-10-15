#ifndef PARQUEADERO_H
#define PARQUEADERO_H

#include <string>
#include <mutex>
#include "Celda.h"

/**
 * @brief Administra la estructura de datos del parqueadero en memoria dinámica.
 * Contiene el arreglo dinámico Celda* celdas y la lógica de negocio para
 * ocupación, búsqueda y liberación por placa repetida.
 */
class Parqueadero {
private:
    int capacidadTotal;
    Celda* celdas;
    mutable std::mutex mutexAcceso; // Protege el acceso concurrente entre sockets y la GUI

    // Deshabilitar copia para garantizar la integridad del puntero celdas
    Parqueadero(const Parqueadero&) = delete;
    Parqueadero& operator=(const Parqueadero&) = delete;

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
};

#endif // PARQUEADERO_H
