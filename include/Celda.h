#ifndef CELDA_H
#define CELDA_H

#include "RegistroVehiculo.h"
#include <string>

class Celda {
private:
    int numeroCelda;
    bool ocupada;
    RegistroVehiculo* vehiculo; // Puntero al vehículo actual

public:
    Celda();
    ~Celda();
    void inicializar(int num);
    bool estaOcupada() const;
    void ocupar(const std::string& placa, const std::string& hora);
    void liberar();
    int getNumeroCelda() const;
    RegistroVehiculo* getVehiculo() const;
};

#endif // CELDA_H