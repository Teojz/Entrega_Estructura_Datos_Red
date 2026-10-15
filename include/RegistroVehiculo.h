#ifndef REGISTRO_VEHICULO_H
#define REGISTRO_VEHICULO_H

#include <string>

/**
 * @brief Representa el registro de un vehículo que ingresa al parqueadero.
 * Almacena su número de placa y la hora en la que ingresó.
 */
class RegistroVehiculo {
private:
    std::string placa;
    std::string horaIngreso;

public:
    // Constructores
    RegistroVehiculo();
    RegistroVehiculo(std::string pl, std::string hora);

    // Getters
    std::string getPlaca() const;
    std::string getHoraIngreso() const;
};

#endif // REGISTRO_VEHICULO_H
