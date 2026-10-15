#ifndef CELDA_H
#define CELDA_H

#include <string>
#include "RegistroVehiculo.h"

/**
 * @brief Modela una celda o plaza de parqueo individual.
 * Administra dinámicamente el puntero al vehículo que la ocupa.
 */
class Celda {
private:
    int numeroCelda;
    bool ocupada;
    RegistroVehiculo* vehiculo;

public:
    // Constructor y Destructor
    Celda();
    ~Celda();

    // Soporte para copia y movimiento seguro (Regla de los 5)
    Celda(const Celda& otra);
    Celda& operator=(const Celda& otra);
    Celda(Celda&& otra) noexcept;
    Celda& operator=(Celda&& otra) noexcept;

    // Métodos según diagrama UML
    void inicializar(int num);
    bool estaOcupada() const;
    void ocupar(std::string placa, std::string hora);
    void liberar();
    int getNumeroCelda() const;
    RegistroVehiculo* getVehiculo() const;
};

#endif // CELDA_H
