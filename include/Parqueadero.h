#ifndef PARQUEADERO_H
#define PARQUEADERO_H

#include "Celda.h"
#include <string>

class Parqueadero {
private:
    int capacidadTotal;
    Celda* celdas; // Arreglo dinámico en el Heap

public:
    Parqueadero(int cap);
    ~Parqueadero();

    std::string procesarEventoPlaca(const std::string& placa, int celdaSugerida, const std::string& hora);
    int buscarPlaca(const std::string& placa) const;
    std::string obtenerEstadoSerializado() const;
    int getCapacidadTotal() const;
};

#endif // PARQUEADERO_H